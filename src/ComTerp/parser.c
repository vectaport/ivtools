/*
 * Copyright (c) 1994, 1995, 1998 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

#include <ComTerp/_comterp.h>
#include <ComTerp/parser.h>
#include <ComTerp/_comutil.h>

#include <string.h>

#define TITLE "Parser"

#if __GNUC__>=3
static char newline;
#endif

/*****************************************************************************/

Parser::Parser() : ComTerpModule() 
{
    init();
}

Parser::Parser(const char* path) : ComTerpModule(path) 
{
    init();
}


Parser::Parser(void* inptr, char*(*infunc)(char*,int,void*), 
	       int(*eoffunc)(void*), int(*errfunc)(void*)) 
: ComTerpModule(inptr, infunc, eoffunc, errfunc)
{
    init();
}

Parser::Parser(istream& in) : 
ComTerpModule(&in, istream_fgets, istream_feof, istream_ferror) 
{
    init();
}

void Parser::init() {

    /* Allocate block for initial postfix tokens to start with */
    _pfsiz = 256;
    if(dmm_calloc((void**)&_pfbuf, _pfsiz, sizeof(postfix_token)) != 0) 
	KANRET("error in call to dmm_calloc");

    /* Create and load operator table */
    if(opr_tbl_default() != 0) 
	KANRET("error in creating and loading default operator table");
}


Parser::~Parser() 
{
    /* Free postfix token buffer */
    if(dmm_free((void**)&_pfbuf) != 0) 
	KANRET ("error in call to dmm_free");

}

int Parser::print_next_expr()
{
    int status = parser (_inptr, _infunc, _eoffunc, _errfunc, NULL, NULL,
			 _buffer, _bufsiz, &_bufptr, _token, _toksiz, &_linenum,
		         &_pfbuf, &_pfsiz, &_pfnum);
    if (status) 
	err_print( stdout, "parser" );
    else
	for (int i = 0; i < _pfnum; i++) print_pfbuf(_pfbuf,i);
    return _pfbuf[_pfnum-1].type != TOK_EOF;
}

postfix_token* Parser::copy_postfix_tokens(int& ntokens) {
    ntokens = _pfnum;
    postfix_token *pfcopy = new postfix_token[ntokens];
    for (int i=0; i<ntokens; i++) 
        pfcopy[i] = _pfbuf[i];
    return pfcopy;
}

boolean Parser::skip_matched_parens() {
  istream& in = *(istream*)_inptr;
  char lparen = in.get();
  if (lparen == '(' || lparen ==  '[' || lparen == '[') {
    int status = 0;

    while (status==0) {

      /* run parser until an unexpected rparen */
      status = parser (_inptr, _infunc, _eoffunc, _errfunc, NULL, NULL,
			   _buffer, _bufsiz, &_bufptr, _token, _toksiz, 
			   &_linenum, &_pfbuf, &_pfsiz, &_pfnum);
      if (status) {
	int errid = comerr_get();
	err_clear();
	if (errid == ERR_UNEXPECTED_RPAREN && lparen == '(')
	  return true;
	else  if (errid == ERR_UNEXPECTED_RBRACKET && lparen == '[')
	  return true;
	else  if (errid == ERR_UNEXPECTED_RBRACE && lparen == '{')
	  return true;
	else
	  return false;
      } 
    }
    return true;
  } else {
    in.unget();
    return false;
  }
}


char* Parser::istream_fgets(char* s, int n, void* instreamp) {
  istream& in  = *(istream*)instreamp;
#if __GNUC__<3
  char *instr;
  in.gets(&instr);
#else
  char instr[BUFSIZ];
  in.get(instr, BUFSIZ);  // needs to be generalized with <vector.h>
  in.get(newline);
#endif
  if (in.good()) {
    int i = 0;
    for (; i<n-2; i++) {
      if (instr[i] == '\0') break;
      s[i] = instr[i];
    }
    s[i++] = '\n';
    s[i] = '\0';
    return s;
  } else
    return nil;
}

int Parser::istream_feof(void* instreamp) {
  istream& in  = *(istream*)instreamp;
  return in.eof();
}

int Parser::istream_ferror(void* instreamp) {
  istream& in  = *(istream*)instreamp;
  return !in.good();
}

