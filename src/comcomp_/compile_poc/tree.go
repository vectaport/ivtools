package main

import (
	"fmt"
	"strconv"
	"strings"
)

// Node mirrors one node of ComTerp's own postfix(expr :tree) output --
// parsed back out of the text that tree value prints as (the same
// {op,arg1,arg2} nested-list syntax any ComTerp list serializes to).
type Node struct {
	Sym    string // set when this leaf is a bare symbol (a command name or a variable reference)
	Int    int64
	IsInt  bool
	Str    string
	IsStr  bool
	IsKw   bool    // set for a (:name value) keyword tuple, e.g. a call's :n 2
	KwName string  // ":n", including the leading colon
	IsList bool    // set for a list node, even an empty one ({}) -- Items alone can't tell "empty list" from "not a list", since both leave it nil
	Items  []*Node // list node: Items[0] is the operator, Items[1:] the operands. keyword node: Items[0] is the value (an empty list for a bare flag, e.g. (:b {}))
}

func (n *Node) isList() bool { return n.IsList }

// parseTree parses the printed form of a ComTerp tree value, e.g.
// "{add,1,{mul,2,3}}" or a bare "42".
func parseTree(s string) (*Node, error) {
	n, rest, err := parseNode(strings.TrimSpace(s))
	if err != nil {
		return nil, err
	}
	if strings.TrimSpace(rest) != "" {
		return nil, fmt.Errorf("trailing text after tree: %q", rest)
	}
	return n, nil
}

func parseNode(s string) (*Node, string, error) {
	if s == "" {
		return nil, "", fmt.Errorf("unexpected end of tree text")
	}
	switch s[0] {
	case '{':
		return parseList(s)
	case '(':
		return parseKeyword(s)
	case '"':
		return parseString(s)
	default:
		return parseAtom(s)
	}
}

// parseKeyword parses a tree-form keyword tuple, e.g. "(:n 2)" or a bare
// flag's "(:b {})" -- name and value are space-separated, never comma-separated.
func parseKeyword(s string) (*Node, string, error) {
	s = s[1:] // consume '('
	i := 0
	for i < len(s) && s[i] != ' ' && s[i] != ')' {
		i++
	}
	name := s[:i]
	s = strings.TrimSpace(s[i:])
	val, rest, err := parseNode(s)
	if err != nil {
		return nil, "", err
	}
	rest = strings.TrimSpace(rest)
	if !strings.HasPrefix(rest, ")") {
		return nil, "", fmt.Errorf("expected ')' closing keyword %s, got %q", name, rest)
	}
	return &Node{IsKw: true, KwName: name, Items: []*Node{val}}, rest[1:], nil
}

func parseList(s string) (*Node, string, error) {
	s = s[1:] // consume '{'
	n := &Node{IsList: true}
	for {
		s = strings.TrimSpace(s)
		if strings.HasPrefix(s, "}") {
			return n, s[1:], nil
		}
		item, rest, err := parseNode(s)
		if err != nil {
			return nil, "", err
		}
		n.Items = append(n.Items, item)
		s = strings.TrimSpace(rest)
		if strings.HasPrefix(s, ",") {
			s = s[1:]
			continue
		}
		if strings.HasPrefix(s, "}") {
			return n, s[1:], nil
		}
		return nil, "", fmt.Errorf("expected ',' or '}' in list, got %q", s)
	}
}

func parseString(s string) (*Node, string, error) {
	// ComTerp quotes a string with "..." and backslash-escapes embedded quotes;
	// good enough for this POC's fallback-arg unparsing, not a full unescaper.
	var sb strings.Builder
	i := 1
	for i < len(s) && s[i] != '"' {
		if s[i] == '\\' && i+1 < len(s) {
			i++
		}
		sb.WriteByte(s[i])
		i++
	}
	if i >= len(s) {
		return nil, "", fmt.Errorf("unterminated string in tree text: %q", s)
	}
	return &Node{Str: sb.String(), IsStr: true}, s[i+1:], nil
}

func parseAtom(s string) (*Node, string, error) {
	i := 0
	for i < len(s) && s[i] != ',' && s[i] != '}' && s[i] != ')' && s[i] != ' ' {
		i++
	}
	tok := s[:i]
	rest := s[i:]
	if iv, err := strconv.ParseInt(tok, 10, 64); err == nil {
		return &Node{Int: iv, IsInt: true}, rest, nil
	}
	return &Node{Sym: tok}, rest, nil
}

// unparse reconstructs valid ComTerp source for a node -- a real (if not
// yet fully general) decompiler from a postfix(:tree)/parse(:tree) value
// back to source, used both for the compiler's fallback calls and, more
// generally, as the kind of func-reflection Scott described: rendering a
// FuncObj's compiled body back as something a person can read.
func unparse(n *Node) string {
	switch {
	case n.IsInt:
		return strconv.FormatInt(n.Int, 10)
	case n.IsStr:
		return `"` + strings.ReplaceAll(n.Str, `"`, `\"`) + `"`
	case n.IsKw:
		if n.Items[0].isList() && len(n.Items[0].Items) == 0 {
			return n.KwName // a bare flag keyword carries no value
		}
		return n.KwName + " " + unparse(n.Items[0])
	case n.Sym != "":
		return n.Sym
	case n.isList():
		return unparseList(n)
	default:
		return ""
	}
}

func unparseList(n *Node) string {
	if len(n.Items) == 0 {
		return "{}" // the empty-list sentinel a bare keyword flag's value prints as
	}

	// A call's operator is always a bare symbol; anything else here (a
	// nested list as Items[0]) means this is several independent
	// top-level results sharing one tree value, not a single call --
	// parse(fileobj :tree)'s own shape, 'eof' sentinel included.
	if n.Items[0].Sym == "" {
		var lines []string
		for _, item := range n.Items {
			if item.Sym == "eof" {
				continue
			}
			lines = append(lines, unparse(item))
		}
		return strings.Join(lines, "\n")
	}

	op := n.Items[0].Sym
	args := n.Items[1:]

	// A few primitives read far better as their original infix/sugar
	// form than as plain call syntax; everything else still round-trips
	// correctly as op(args...), just less idiomatically.
	switch {
	case op == "assign" && len(args) == 2:
		return unparse(args[0]) + "=" + unparse(args[1])
	case op == "seq":
		parts := make([]string, len(args))
		for i, a := range args {
			parts[i] = unparse(a)
		}
		return "(" + strings.Join(parts, ";") + ")"
	}

	argStrs := make([]string, len(args))
	for i, a := range args {
		argStrs[i] = unparse(a)
	}
	return op + "(" + strings.Join(argStrs, " ") + ")"
}
