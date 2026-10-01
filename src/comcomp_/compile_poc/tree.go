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
	Sym   string  // set when this leaf is a bare symbol (a command name or a variable reference)
	Int   int64
	IsInt bool
	Str   string
	IsStr bool
	Items []*Node // set for a list node; Items[0] is the operator, Items[1:] the operands
}

func (n *Node) isList() bool { return n.Items != nil }

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
	case '"':
		return parseString(s)
	default:
		return parseAtom(s)
	}
}

func parseList(s string) (*Node, string, error) {
	s = s[1:] // consume '{'
	n := &Node{}
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
	for i < len(s) && s[i] != ',' && s[i] != '}' {
		i++
	}
	tok := s[:i]
	rest := s[i:]
	if iv, err := strconv.ParseInt(tok, 10, 64); err == nil {
		return &Node{Int: iv, IsInt: true}, rest, nil
	}
	return &Node{Sym: tok}, rest, nil
}

// unparse reconstructs valid ComTerp source for a node, used when a
// subtree can't be natively compiled and has to fall back to evaluating
// through the bridge instead.
func unparse(n *Node) string {
	switch {
	case n.IsInt:
		return strconv.FormatInt(n.Int, 10)
	case n.IsStr:
		return `"` + strings.ReplaceAll(n.Str, `"`, `\"`) + `"`
	case n.Sym != "":
		return n.Sym
	case n.isList():
		op := unparse(n.Items[0])
		args := make([]string, len(n.Items)-1)
		for i, a := range n.Items[1:] {
			args[i] = unparse(a)
		}
		return op + "(" + strings.Join(args, " ") + ")"
	default:
		return ""
	}
}
