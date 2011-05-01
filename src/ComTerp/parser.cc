/*
 * Copyright (c) 2005 Scott E. Johnston
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

extern int _continuation_prompt;
extern int _continuation_prompt_disabled;
extern int _skip_shell_comments;
extern infuncptr _oneshot_infunc;
extern int _detail_matched_delims;
extern int _ignore_numerics;
extern int _angle_brackets;
extern unsigned _token_state_save;

extern void* parser_client;             /* pointer to current client */
extern unsigned expecting;              /* Type of operator expected next */

extern paren_stack *ParenStack;         /* Stack to count args and keywords */
extern int TopOfParenStack;             /* Top of ParenStack */
extern int SizeOfParenStack;            /* Allocated size of ParenStack */

extern oper_stack *OperStack;          /* Operator stack */
extern int TopOfOperStack;             /* Top of OperStack */
extern int SizeOfOperStack;            /* Allocated size of OperStack */

extern unsigned NextBufptr;            /* Variables for look-ahead token */
extern char *NextToken;
extern unsigned NextToklen;    
extern unsigned NextToktype;
extern unsigned NextTokstart;
extern unsigned NextLinenum;
extern int NextOp_ids[OPTYPE_NUM];

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

    /* initialize the backup copies of all the globals */
    __continuation_prompt = 0;
    __continuation_prompt_disabled = 0;
    __skip_shell_comments = 0;
    __detail_matched_delims = 0;
    __angle_brackets = 0;
    __token_state_save = TOK_WHITESPACE;
    __ignore_numerics = 0;
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

void Parser::check_parser_client() {
  if (parser_client==NULL)
    parser_client = (void*)this;
  else if (parser_client != (void*)this) {
    parser_client = (void*)this;
    _continuation_prompt = __continuation_prompt;
    _continuation_prompt_disabled = __continuation_prompt_disabled;
    _skip_shell_comments = __skip_shell_comments;
    _oneshot_infunc = __oneshot_infunc;
    _detail_matched_delims = __detail_matched_delims;
    _ignore_numerics = __ignore_numerics;
    _angle_brackets = __angle_brackets ;
    _token_state_save = __token_state_save;
    if (_linenum != 0) {
      expecting = _expecting;
      ParenStack = _ParenStack;
      TopOfParenStack = _TopOfParenStack;
      SizeOfParenStack = _SizeOfParenStack;
      OperStack = _OperStack;
      TopOfOperStack = _TopOfOperStack;
      SizeOfOperStack = _SizeOfOperStack;
      NextBufptr = _NextBufptr;
      NextToken = _NextToken;
      NextToklen = _NextToklen;    
      NextToktype = _NextToktype;
      NextTokstart = _NextTokstart;
      NextLinenum = _NextLinenum;
      for (int i=0; i<OPTYPE_NUM; i++)
	NextOp_ids[i] = _NextOp_ids[i];
      opr_tbl_ptr_set(_opr_tbl_ptr);
      opr_tbl_numop_set(_opr_tbl_numop);
      opr_tbl_maxop_set(_opr_tbl_maxop);
      opr_tbl_maxpri_set(_opr_tbl_maxpri);
      opr_tbl_lastop_set(_opr_tbl_lastop);
    }
  }
}

void Parser::save_parser_client() {
  __continuation_prompt = _continuation_prompt;
  __continuation_prompt_disabled = _continuation_prompt_disabled;
  __skip_shell_comments = _skip_shell_comments;
  __oneshot_infunc = _oneshot_infunc;
  __detail_matched_delims = _detail_matched_delims;
  __ignore_numerics = _ignore_numerics;
  __angle_brackets  = _angle_brackets ;
  __token_state_save = _token_state_save;
  _expecting = expecting;
  _ParenStack = ParenStack;
  _TopOfParenStack = TopOfParenStack;
  _SizeOfParenStack = SizeOfParenStack;
  _OperStack = OperStack;
  _TopOfOperStack = TopOfOperStack;
  _SizeOfOperStack = SizeOfOperStack;
  _NextBufptr = NextBufptr;
  _NextToken = NextToken;
  _NextToklen = NextToklen;    
  _NextToktype = NextToktype;
  _NextTokstart = NextTokstart;
  _NextLinenum = NextLinenum;
  for (int i=0; i<OPTYPE_NUM; i++)
    _NextOp_ids[i] = NextOp_ids[i];
  _opr_tbl_ptr = opr_tbl_ptr_get();
  _opr_tbl_numop = opr_tbl_numop_get();
  _opr_tbl_maxop = opr_tbl_maxop_get();
  _opr_tbl_maxpri = opr_tbl_maxpri_get();
  _opr_tbl_lastop = opr_tbl_lastop_get();
}
