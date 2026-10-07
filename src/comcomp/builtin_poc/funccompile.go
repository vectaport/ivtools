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
		fc.compileStmt(s, 1)
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
// sub-statements compile the same way a top-level one does.
func (fc *funcCompiler) compileStmt(n *Node, depth int) {
	if n.isList() && len(n.Items) == 3 && n.Items[0].Sym == "seq" {
		fc.compileStmt(n.Items[1], depth)
		fc.compileStmt(n.Items[2], depth)
		return
	}
	if !n.isList() || n.Items[0].Sym == "" {
		fc.emit(depth, "_ = "+fc.compileExpr(n)) // a bare expression statement (shouldn't occur in this spike's func, kept for safety)
		return
	}

	op := n.Items[0].Sym
	args := n.Items[1:]
	switch op {
	case "assign":
		fc.compileAssign(args[0], args[1], depth)
	case "for":
		fc.compileFor(args[0], args[1], args[2], args[3], depth)
	case "while":
		fc.compileWhile(args[0], args[1], depth)
	default:
		fc.emit(depth, "_ = "+fc.compileExpr(n))
	}
}

func (fc *funcCompiler) compileAssign(lhs, rhs *Node, depth int) {
	rhsExpr := fc.compileExpr(rhs)
	if lhs.isList() && len(lhs.Items) == 3 && lhs.Items[0].Sym == "at" {
		// lst@idx=val -> arr[idx] = val
		idxExpr := fc.compileExpr(lhs.Items[2])
		fc.emit(depth, fmt.Sprintf("%s[%s] = %s", fc.goArrName(lhs.Items[1]), idxExpr, rhsExpr))
		return
	}
	if lhs.Sym == "" {
		fc.emit(depth, "// unsupported assignment target: "+unparse(lhs))
		return
	}
	if fc.declared[lhs.Sym] {
		fc.emit(depth, fmt.Sprintf("%s = %s", lhs.Sym, rhsExpr))
	} else {
		fc.declared[lhs.Sym] = true
		fc.emit(depth, fmt.Sprintf("%s := %s", lhs.Sym, rhsExpr))
	}
}

func (fc *funcCompiler) goArrName(n *Node) string {
	if n.Sym == fc.arrParam {
		return "arr"
	}
	return n.Sym // not reachable in this spike's one-array func, kept explicit rather than silently wrong
}

func (fc *funcCompiler) compileFor(initN, testN, nextN, bodyN *Node, depth int) {
	// The loop variable is declared by the init clause itself (Go's
	// for-statement init runs in the loop's own scope), so it's marked
	// declared before compiling init's RHS rather than through
	// compileAssign's normal first-use check.
	if initN.Items[0].Sym != "assign" || initN.Items[1].Sym == "" {
		fc.emit(depth, "// unsupported for-init: "+unparse(initN))
		return
	}
	loopVar := initN.Items[1].Sym
	wasDeclared := fc.declared[loopVar]
	fc.declared[loopVar] = true
	initExpr := fmt.Sprintf("%s := %s", loopVar, fc.compileExpr(initN.Items[2]))
	testExpr := fc.compileExpr(testN)
	nextStmt := fc.compileSimpleAssignExpr(nextN)

	fc.emit(depth, fmt.Sprintf("for %s; %s; %s {", initExpr, testExpr, nextStmt))
	fc.compileStmt(bodyN, depth+1)
	fc.emit(depth, "}")
	fc.declared[loopVar] = wasDeclared
}

// compileSimpleAssignExpr renders an {assign,var,expr} node as a bare Go
// assignment ("i = i + 1"), the shape a for-statement's post clause needs
// (no ':=', no statement terminator).
func (fc *funcCompiler) compileSimpleAssignExpr(n *Node) string {
	if !n.isList() || n.Items[0].Sym != "assign" || n.Items[1].Sym == "" {
		return "/* unsupported for-next: " + unparse(n) + " */"
	}
	return fmt.Sprintf("%s = %s", n.Items[1].Sym, fc.compileExpr(n.Items[2]))
}

func (fc *funcCompiler) compileWhile(testN, bodyN *Node, depth int) {
	fc.emit(depth, fmt.Sprintf("for %s {", fc.compileExpr(testN)))
	fc.compileStmt(bodyN, depth+1)
	fc.emit(depth, "}")
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
// unrecognized node is a compile error surfaced inline rather than a silent
// runtime fallback -- there is no bridge handle available inside the
// built-in command this is compiled for.
func (fc *funcCompiler) compileExpr(n *Node) string {
	switch {
	case n.IsInt:
		return fmt.Sprintf("int64(%d)", n.Int)
	case n.Sym != "":
		if n.Sym == fc.arrParam {
			return "arr"
		}
		return n.Sym
	case n.isList() && len(n.Items) == 3 && n.Items[0].Sym == "at":
		return fmt.Sprintf("%s[%s]", fc.goArrName(n.Items[1]), fc.compileExpr(n.Items[2]))
	case n.isList() && len(n.Items) == 2 && n.Items[0].Sym == "size":
		return fmt.Sprintf("int64(len(%s))", fc.goArrName(n.Items[1]))
	case n.isList() && len(n.Items) == 3 && op(n) != "":
		lhs := fc.compileExpr(n.Items[1])
		rhs := fc.compileExpr(n.Items[2])
		if goOp, ok := nativeBinOps[n.Items[0].Sym]; ok {
			return fmt.Sprintf("(%s %s %s)", lhs, goOp, rhs)
		}
		if goOp, ok := nativeCompareOps[n.Items[0].Sym]; ok {
			return fmt.Sprintf("(%s %s %s)", lhs, goOp, rhs)
		}
		if goOp, ok := nativeLogicOps[n.Items[0].Sym]; ok {
			return fmt.Sprintf("(%s %s %s)", lhs, goOp, rhs)
		}
	}
	return "/* UNSUPPORTED: " + unparse(n) + " */"
}

func op(n *Node) string {
	if n.isList() && len(n.Items) > 0 {
		return n.Items[0].Sym
	}
	return ""
}
