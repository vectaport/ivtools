// Command translate is comcomp's Go-side half: reads a ComTerp
// postfix(:tree) value (as its own printed text, e.g. "{func,...}") on
// stdin and writes the funccompile.go-compiled Go function body to
// stdout. It has no cgo/bridge dependency of its own -- getting the tree
// text out of a real ComTerp instance is the comcomp .comt script's job
// (comterp already has postfix(:tree) natively); this binary only ever
// sees the printed tree text a caller hands it.
package main

import (
	"fmt"
	"io"
	"os"
)

func main() {
	treeText, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintf(os.Stderr, "translate: reading stdin: %s\n", err)
		os.Exit(1)
	}

	tree, err := parseTree(string(treeText))
	if err != nil {
		fmt.Fprintf(os.Stderr, "translate: parsing tree: %s\n", err)
		os.Exit(1)
	}

	body, err := compileSortFunc(tree)
	if err != nil {
		fmt.Fprintf(os.Stderr, "translate: compiling: %s\n", err)
		os.Exit(1)
	}
	fmt.Print(body)
}
