package main

import "fmt"

// nativeBinOps is the one hand-written primitive family for this spike --
// integer arithmetic. Each entry compiles straight to the matching Go
// operator; every other ComTerp primitive, whatever it is, falls back to
// evaluating through the bridge instead of failing to compile.
var nativeBinOps = map[string]string{
	"add": "+",
	"sub": "-",
	"mpy": "*",
	"div": "/",
}

// compiled is one compiled subexpression: the Go source text for it, and
// whether it's natively emitted (pure Go, no bridge round trip) or a
// fallback call through the bridge for this node's original ComTerp source.
type compiled struct {
	goExpr   string
	native   bool
	fallback int // number of fallback (bridge) calls this subexpression and its children make
}

// compileNode walks one tree node, emitting native Go wherever the whole
// subtree is built only from int literals and nativeBinOps, and falling
// back to a bridge call at the smallest enclosing subtree that isn't.
func compileNode(n *Node) compiled {
	if n.IsInt {
		return compiled{goExpr: fmt.Sprintf("int64(%d)", n.Int), native: true}
	}

	if n.isList() && len(n.Items) == 3 && n.Items[0].Sym != "" {
		if op, ok := nativeBinOps[n.Items[0].Sym]; ok {
			// The operator itself is native even when a child isn't --
			// compose with whatever Go each child already produced
			// (itself native, or its own smaller fallback call) rather
			// than discarding that work and falling back the whole node.
			lhs := compileNode(n.Items[1])
			rhs := compileNode(n.Items[2])
			return compiled{
				goExpr:   fmt.Sprintf("(%s %s %s)", lhs.goExpr, op, rhs.goExpr),
				native:   lhs.native && rhs.native,
				fallback: lhs.fallback + rhs.fallback,
			}
		}
	}

	// Fallback tier: not (yet) a primitive this compiler hand-writes --
	// evaluate the node's own ComTerp source through the bridge instead
	// of refusing to compile it.
	return compiled{
		goExpr:   fmt.Sprintf("evalInt(h, %q)", unparse(n)),
		native:   false,
		fallback: 1,
	}
}
