package main

import (
	"fmt"
	"os"
)

func writeFile(path, content string) error {
	return os.WriteFile(path, []byte(content), 0644)
}

// roundtripCases are real expressions from the existing test suite
// (src/comterp_/tests/postfixtree.comt), not invented samples -- chosen
// because that test file already documents each one's exact expected
// tree shape.
var roundtripCases = []string{
	`x=1+2`,
	`(x=1;y=2)`,
	`y=list(1 :n 2)`,
	`global(x)=1`,
	`list(1 :a 2 :b)`,
	`func(a b;a+b*2)`,
}

// roundtripSelfTest proves unparse() is a real decompiler, not just good
// enough for the compiler's own fallback calls: get each case's real
// tree, unparse it back to source, re-parse that regenerated source, and
// check the resulting tree is identical to the original. The regenerated
// syntax is allowed to differ from the input (assign sugar vs.
// assign(...), say) as long as it parses back to the same tree -- that's
// the bar for "faithfully reconstructs the program," not byte-for-byte
// text equality.
func roundtripSelfTest(eval func(string) (string, error)) bool {
	ok := true
	for _, expr := range roundtripCases {
		origTree, err := eval("postfix(" + expr + " :tree)")
		if err != nil {
			fmt.Printf("FAIL %-28q getting original tree: %s\n", expr, err)
			ok = false
			continue
		}
		node, err := parseTree(origTree)
		if err != nil {
			fmt.Printf("FAIL %-28q parsing tree %q: %s\n", expr, origTree, err)
			ok = false
			continue
		}
		regenerated := unparse(node)
		roundTree, err := eval("postfix(" + regenerated + " :tree)")
		if err != nil {
			fmt.Printf("FAIL %-28q regenerated %q: %s\n", expr, regenerated, err)
			ok = false
			continue
		}
		if roundTree != origTree {
			fmt.Printf("FAIL %-28q regenerated %q -> tree %q, want %q\n", expr, regenerated, roundTree, origTree)
			ok = false
			continue
		}
		fmt.Printf("pass %-28q -> regenerated %-40q -> tree %s\n", expr, regenerated, roundTree)
	}
	return ok
}

// fileRoundtripSelfTest is the same proof at file scope: parse a real
// multi-statement file (parse(fileobj :tree), not a single expression),
// regenerate its source from that tree, write it out, and re-parse the
// regenerated file -- checking the whole file's tree, not just one
// expression's.
func fileRoundtripSelfTest(eval func(string) (string, error), path string) bool {
	origTree, err := eval(fmt.Sprintf("parse(open(%q) :tree)", path))
	if err != nil {
		fmt.Printf("FAIL parsing %s: %s\n", path, err)
		return false
	}
	node, err := parseTree(origTree)
	if err != nil {
		fmt.Printf("FAIL parsing tree for %s: %s\n", path, err)
		return false
	}
	regenerated := unparse(node)
	fmt.Printf("--- regenerated %s ---\n%s\n---\n", path, regenerated)

	regenPath := path + ".regenerated"
	// ComTerp's file scanner needs a trailing newline to close off the
	// last statement -- without one it silently drops it instead of
	// erroring, so unparse() output bound for a file always needs one.
	if err := writeFile(regenPath, regenerated+"\n"); err != nil {
		fmt.Printf("FAIL writing %s: %s\n", regenPath, err)
		return false
	}
	roundTree, err := eval(fmt.Sprintf("parse(open(%q) :tree)", regenPath))
	if err != nil {
		fmt.Printf("FAIL parsing regenerated %s: %s\n", regenPath, err)
		return false
	}
	if roundTree != origTree {
		fmt.Printf("FAIL file round trip:\n  orig:  %s\n  round: %s\n", origTree, roundTree)
		return false
	}
	fmt.Printf("pass %s -> tree %s\n", path, roundTree)
	return true
}
