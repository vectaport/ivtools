package main

import (
	"fmt"
	"strings"
)

// flattenSeqChain walks a left-associated seq spine -- {seq,{seq,a,b},c} --
// and returns its leaves in left-to-right order: [a,b,c]. This is the Go
// side of the same left-spine shape funcobj_flatten_seq() flattens in
// strmfunc.c for info(f).source.
func flattenSeqChain(n *Node) []*Node {
	if n.isList() && len(n.Items) == 3 && n.Items[0].Sym == "seq" {
		return append(flattenSeqChain(n.Items[1]), n.Items[2])
	}
	return []*Node{n}
}

// funcCompiler emits a Go function body for a single-int-array-parameter
// ComTerp func, one ComTerp local becoming one Go local declared with ':='
// on first assignment and plain '=' after -- the per-variable type
// inference Scott described, narrowed here to the one type this spike
// needs (int64) since every value in a sort is an array element, an index,
// or a loop bound.
type funcCompiler struct {
	declared map[string]bool
	arrParam string // the ComTerp param name bound to the Go []int64 slice
	out      strings.Builder
}

// compileSortFunc compiles a func node of the shape
// {func,{seq,...,{seq,...,paramSym},lastStmt}} -- a single bare parameter
// followed by a flat statement chain -- into a Go function body operating
// on a []int64 named "arr". Returns an error string instead of panicking
// so a node this spike doesn't yet handle is a clear, reported failure
// rather than a crash.
func compileSortFunc(funcNode *Node) (string, error) {
	if !funcNode.isList() || len(funcNode.Items) != 2 || funcNode.Items[0].Sym != "func" {
		return "", fmt.Errorf("not a func node: %s", unparse(funcNode))
	}
	stmts := flattenSeqChain(funcNode.Items[1])
	if len(stmts) < 2 || stmts[0].Sym == "" {
		return "", fmt.Errorf("expected a single bare parameter first, got: %s", unparse(stmts[0]))
	}
	fc := &funcCompiler{declared: map[string]bool{}, arrParam: stmts[0].Sym}
	fc.declared[fc.arrParam] = true

	for _, s := range stmts[1 : len(stmts)-1] {
		if err := fc.compileStmt(s, 1); err != nil {
			return "", err
		}
	}
	// The func's own last value is its return value in ComTerp; this
	// spike only ever returns the sorted array itself, so require that
	// shape rather than silently compiling something else.
	last := stmts[len(stmts)-1]
	if last.Sym != fc.arrParam {
		return "", fmt.Errorf("expected func to end by returning %q, got: %s", fc.arrParam, unparse(last))
	}
	return fc.out.String(), nil
}

func (fc *funcCompiler) emit(depth int, line string) {
	fc.out.WriteString(strings.Repeat("\t", depth))
	fc.out.WriteString(line)
	fc.out.WriteString("\n")
}

// compileStmt emits one or more Go statements for a ComTerp statement node,
// recursing through seq chains so a for/while body's own ';'-joined
// sub-statements compile the same way a top-level one does. An
// unrecognized node is a compile error, not a comment standing in for the
// work it would have done.
func (fc *funcCompiler) compileStmt(n *Node, depth int) error {
	if n.isList() && len(n.Items) == 3 && n.Items[0].Sym == "seq" {
		if err := fc.compileStmt(n.Items[1], depth); err != nil {
			return err
		}
		return fc.compileStmt(n.Items[2], depth)
	}
	if !n.isList() || n.Items[0].Sym == "" {
		expr, err := fc.compileExpr(n)
		if err != nil {
			return err
		}
		fc.emit(depth, "_ = "+expr) // a bare expression statement (shouldn't occur in this spike's func, kept for safety)
		return nil
	}

	op := n.Items[0].Sym
	args := n.Items[1:]
	switch op {
	case "assign":
		return fc.compileAssign(args[0], args[1], depth)
	case "for":
		return fc.compileFor(args[0], args[1], args[2], args[3], depth)
	case "while":
		return fc.compileWhile(args[0], args[1], depth)
	default:
		expr, err := fc.compileExpr(n)
		if err != nil {
			return err
		}
		fc.emit(depth, "_ = "+expr)
		return nil
	}
}

// compileAssign handles both lst@idx=val ("arr[idx] = val") and a plain
// variable assign, ":=" on first assignment and "=" after -- the
// assignment target goes through goArrName exactly like any other
// reference to it, so lhs==arrParam becomes "arr" in Go too, not the
// ComTerp parameter name.
func (fc *funcCompiler) compileAssign(lhs, rhs *Node, depth int) error {
	rhsExpr, err := fc.compileExpr(rhs)
	if err != nil {
		return err
	}
	if lhs.isList() && len(lhs.Items) == 3 && lhs.Items[0].Sym == "at" {
		// lst@idx=val -> arr[idx] = val
		idxExpr, err := fc.compileExpr(lhs.Items[2])
		if err != nil {
			return err
		}
		fc.emit(depth, fmt.Sprintf("%s[%s] = %s", fc.goArrName(lhs.Items[1]), idxExpr, rhsExpr))
		return nil
	}
	if lhs.Sym == "" {
		return fmt.Errorf("unsupported assignment target: %s", unparse(lhs))
	}
	goName := fc.goArrName(lhs)
	if fc.declared[lhs.Sym] {
		fc.emit(depth, fmt.Sprintf("%s = %s", goName, rhsExpr))
	} else {
		fc.declared[lhs.Sym] = true
		fc.emit(depth, fmt.Sprintf("%s := %s", goName, rhsExpr))
	}
	return nil
}

func (fc *funcCompiler) goArrName(n *Node) string {
	if n.Sym == fc.arrParam {
		return "arr"
	}
	return n.Sym // not reachable in this spike's one-array func, kept explicit rather than silently wrong
}

// compileFor hoists the loop variable as a Go local declared BEFORE the
// for statement (var x int64 = init; for ; test; next {...}) rather than
// in the for-statement's own init clause -- Go's for-init variable is
// scoped to the for statement itself and stops existing once it ends,
// while a ComTerp loop variable is an ordinary func-scoped local that
// keeps its last value for whatever runs after the loop. A loop variable
// already declared outside this loop (an outer local, or a previous
// sibling loop reusing the same name) is simply assigned, not redeclared,
// so it keeps being the one shared variable throughout.
func (fc *funcCompiler) compileFor(initN, testN, nextN, bodyN *Node, depth int) error {
	if !initN.isList() || len(initN.Items) == 0 || initN.Items[0].Sym != "assign" || initN.Items[1].Sym == "" {
		return fmt.Errorf("unsupported for-init: %s", unparse(initN))
	}
	loopVar := initN.Items[1].Sym
	goName := fc.goArrName(initN.Items[1])
	initExpr, err := fc.compileExpr(initN.Items[2])
	if err != nil {
		return err
	}
	testExpr, err := fc.compileExpr(testN)
	if err != nil {
		return err
	}
	if !nextN.isList() || len(nextN.Items) == 0 || nextN.Items[0].Sym != "assign" || nextN.Items[1].Sym == "" {
		return fmt.Errorf("unsupported for-next: %s", unparse(nextN))
	}
	nextGoName := fc.goArrName(nextN.Items[1])
	nextRhs, err := fc.compileExpr(nextN.Items[2])
	if err != nil {
		return err
	}
	nextStmt := fmt.Sprintf("%s = %s", nextGoName, nextRhs)

	wasDeclared := fc.declared[loopVar]
	var predecl string
	if wasDeclared {
		predecl = fmt.Sprintf("%s = %s", goName, initExpr)
	} else {
		fc.declared[loopVar] = true
		predecl = fmt.Sprintf("var %s int64 = %s", goName, initExpr)
	}
	fc.emit(depth, predecl)
	fc.emit(depth, fmt.Sprintf("for ; %s; %s {", testExpr, nextStmt))
	if err := fc.compileStmt(bodyN, depth+1); err != nil {
		return err
	}
	fc.emit(depth, "}")
	return nil
}

func (fc *funcCompiler) compileWhile(testN, bodyN *Node, depth int) error {
	testExpr, err := fc.compileExpr(testN)
	if err != nil {
		return err
	}
	fc.emit(depth, fmt.Sprintf("for %s {", testExpr))
	if err := fc.compileStmt(bodyN, depth+1); err != nil {
		return err
	}
	fc.emit(depth, "}")
	return nil
}

// nativeBinOps is the arithmetic primitive family (same mapping as the
// compile_poc spike's table, repeated here since this directory is its own
// Go module). nativeCompareOps/nativeLogicOps extend it with the
// comparison and logical primitives a sort needs; kept as separate tables
// since they read bool, not int64, in Go.
var nativeBinOps = map[string]string{"add": "+", "sub": "-", "mpy": "*", "div": "/"}
var nativeCompareOps = map[string]string{
	"lt": "<", "gt": ">", "lt_or_eq": "<=", "gt_or_eq": ">=", "eq": "==", "not_eq": "!=",
}
var nativeLogicOps = map[string]string{"and": "&&", "or": "||"}

// compileExpr renders a value-producing ComTerp node as a Go expression.
// Unlike compile.go's compileNode (which falls back to a bridge call for
// anything it doesn't recognize), this spike's func is known in full, so an
// unrecognized node is a compile error returned to the caller rather than a
// silent runtime fallback -- there is no bridge handle available inside the
// built-in command this is compiled for.
func (fc *funcCompiler) compileExpr(n *Node) (string, error) {
	switch {
	case n.IsInt:
		return fmt.Sprintf("int64(%d)", n.Int), nil
	case n.Sym != "":
		if n.Sym == fc.arrParam {
			return "arr", nil
		}
		return n.Sym, nil
	case n.isList() && len(n.Items) == 3 && n.Items[0].Sym == "at":
		idxExpr, err := fc.compileExpr(n.Items[2])
		if err != nil {
			return "", err
		}
		return fmt.Sprintf("%s[%s]", fc.goArrName(n.Items[1]), idxExpr), nil
	case n.isList() && len(n.Items) == 2 && n.Items[0].Sym == "size":
		return fmt.Sprintf("int64(len(%s))", fc.goArrName(n.Items[1])), nil
	case n.isList() && len(n.Items) == 3 && op(n) != "":
		goOp, isBin := nativeBinOps[n.Items[0].Sym]
		if !isBin {
			goOp, isBin = nativeCompareOps[n.Items[0].Sym]
		}
		if !isBin {
			goOp, isBin = nativeLogicOps[n.Items[0].Sym]
		}
		if isBin {
			lhs, err := fc.compileExpr(n.Items[1])
			if err != nil {
				return "", err
			}
			rhs, err := fc.compileExpr(n.Items[2])
			if err != nil {
				return "", err
			}
			return fmt.Sprintf("(%s %s %s)", lhs, goOp, rhs), nil
		}
	}
	return "", fmt.Errorf("unsupported node: %s", unparse(n))
}

func op(n *Node) string {
	if n.isList() && len(n.Items) > 0 {
		return n.Items[0].Sym
	}
	return ""
}
