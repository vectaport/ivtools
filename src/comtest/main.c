/*
 * Copyright (c) 1994-1996 Vectaport, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ComTerp/parser.h>
#include <ComTerp/scanner.h>
#include <ComTerp/postfixspan.h>

#include <iostream.h>
#include <string.h>
#include <fstream>

extern int _empty_statement;
extern int _detail_matched_delims;
extern int _colon_ident;
extern int _whitespace_binding;

using std::cerr;
using std::cout;
using std::endl;

const char* type_names[] = {
	"None",
	"Identifier",
	"Operator",
	"String",
	"Char",
	"Dfint",
	"Dfuns",
	"Lnint",
	"Lnuns",
	"Float",
	"Double",
	"Eof",
	"Eol",
	"Whitespace",
	"Oct",
	"Hex",
	"Comment",
	"Lparen",
	"Rparen",
	"Lbracket",
	"Rbracket",
	"Lbrace",
	"Rbrace",
	"Keyword",
        "Command" };


int main(int argc, char *argv[]) {

    if (argc==1 || (strcmp(argv[1], "parser")!=0 &&
		   strcmp(argv[1], "scanner")!=0 &&
		   strcmp(argv[1], "lexscan")!=0 &&
		   strcmp(argv[1], "cparse")!=0 &&
		   strcmp(argv[1], "spanwalk")!=0)) {
	cerr << "comtest parser|scanner|lexscan|cparse|spanwalk\n";
	return -1;
    }

    if (strcmp(argv[1], "parser")==0) {

	Parser parser;

	while (parser.print_next_expr());

    } else if (strcmp(argv[1], "scanner")==0) {

	Scanner scanner;
	unsigned int toktype;

	do {
	    const char *token = scanner.get_next_token_string(toktype);
	    cout << token << "  (type:  " << type_names[toktype] << ")" << endl;
	} while (toktype != TOK_EOF);

    } else if (strcmp(argv[1], "lexscan")==0) {

	LexScan lexscan;
	unsigned int toktype;
	
	do {
	    const char *token = lexscan.get_next_token_string(toktype);
	    cout << token << "  (type:  " << type_names[toktype] << ")" << endl;
	} while (toktype != TOK_EOF);

    } else if (strcmp(argv[1], "cparse")==0) {

        _empty_statement = 1;
	_detail_matched_delims = 1; 
	_whitespace_binding = 1;
	_colon_ident = 0;
	// need to add ? and : as operators in a new CParser class.
	Parser parser;

	while (parser.print_next_expr());

    } else if (strcmp(argv[1], "spanwalk")==0) {

	/* Parses one expression at a time (printing the same raw postfix
	   listing "parser" mode does, for side-by-side comparison), then
	   runs PostfixSpanWalk over a detached copy of the same buffer --
	   the way FuncObjFunc::execute() and stream-literal construction
	   obtain their own copies -- and prints each command token's
	   consumed operand spans, in source order, plus whatever spans
	   are left over at the end (one per top-level ';' statement). */
	Parser parser;

	while (parser.print_next_expr()) {
	    int ntoks = 0;
	    postfix_token* toks = parser.copy_postfix_tokens(ntoks);
	    if (!toks) continue;

	    PostfixSpanWalk walk;
	    for (int i = 0; i < ntoks; i++) {
		walk.step(toks, i);
		if (toks[i].type == TOK_COMMAND) {
		    cout << "  spanwalk " << i << ": ("
			 << symbol_pntr(toks[i].v.symbolid) << ") consumed:";
		    for (int k = 0; k < walk.consumed_count(); k++) {
			PostfixSpanWalk::Span s = walk.consumed(k);
			cout << " [" << s.start << "," << s.count << ")";
		    }
		    cout << endl;
		}
	    }
	    cout << "  spanwalk remaining:";
	    for (int k = 0; k < walk.remaining_count(); k++) {
		PostfixSpanWalk::Span s = walk.remaining(k);
		cout << " [" << s.start << "," << s.count << ")";
	    }
	    cout << endl;

	    delete [] toks;
	}

    }
    return 0;
}

