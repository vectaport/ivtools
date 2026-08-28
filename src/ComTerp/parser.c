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

static char newline;

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
    __lexscan_last_tokend = 0;
    __lexscan_last_toktype = TOK_NONE;

    /* And the backup copies of the parse state proper, which nothing set
       before: check_parser_client() installs these when this parser takes the
       globals over, so leaving them indeterminate meant the first parse ran on
       whatever the previous client had left there -- and save_parser_client()
       then copied that client's ParenStack POINTER in here, aliasing two
       interpreters onto one stack of argument and keyword counts.  A NULL stack
       makes parser() allocate one belonging to this parser alone. */
    _expecting = 0;
    _ParenStack = NULL;
    _TopOfParenStack = -1;
    _SizeOfParenStack = 0;
    _OperStack = NULL;
    _TopOfOperStack = -1;
    _SizeOfOperStack = 0;
    _NextBufptr = 0;
    _NextToken = NULL;
    _NextToklen = 0;
    _NextToktype = 0;
    _NextTokstart = 0;
    _NextLinenum = 0;
    for (int i=0; i<OPTYPE_NUM; i++)
      _NextOp_ids[i] = 0;

    /* the operator table is legitimately shared -- every parser wants the same
       operators -- so take the current values rather than emptying them */
    _opr_tbl_ptr = opr_tbl_ptr_get();
    _opr_tbl_numop = opr_tbl_numop_get();
    _opr_tbl_maxop = opr_tbl_maxop_get();
    _opr_tbl_maxpri = opr_tbl_maxpri_get();
    _opr_tbl_lastop = opr_tbl_lastop_get();
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
    return _pfnum==0 || _pfbuf[_pfnum-1].type != TOK_EOF;
}

postfix_token* Parser::copy_postfix_tokens(int& ntokens) {
    ntokens = _pfnum;
    return copy_postfix_tokens(_pfbuf, ntokens);
}

postfix_token* Parser::copy_postfix_tokens(postfix_token* toks, int ntokens) {
    postfix_token *pfcopy = new postfix_token[ntokens];
    for (int i=0; i<ntokens; i++) {
      pfcopy[i] = toks[i];
      if (pfcopy[i].type==TOK_STRING)
	symbol_reference(pfcopy[i].v.symbolid);
    }
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
  char instr[BUFSIZ];
  in.get(instr, BUFSIZ);  // needs to be generalized with <vector.h>
  in.get(newline);
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

void Parser::check_parser_client(boolean restore) {
  if (parser_client==NULL)
    parser_client = (void*)this;
  else if (parser_client != (void*)this || restore) {
    parser_client = (void*)this;
    _continuation_prompt = __continuation_prompt;
    _continuation_prompt_disabled = __continuation_prompt_disabled;
    _skip_shell_comments = __skip_shell_comments;
    _oneshot_infunc = __oneshot_infunc;
    _detail_matched_delims = __detail_matched_delims;
    _ignore_numerics = __ignore_numerics;
    _angle_brackets = __angle_brackets ;
    _token_state_save = __token_state_save;
    _lexscan_last_tokend = __lexscan_last_tokend;
    _lexscan_last_toktype = __lexscan_last_toktype;
    {
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
    }
    /* The operator table is the one thing here that is legitimately shared --
       every parser wants the same operators, and a saved copy is only ever a
       copy of the one global.  Restoring a snapshot on a parser's FIRST parse
       would undo an optable(:insert) made since that parser was constructed
       (%% is defined that way at runtime), so this keeps the guard the parse
       state above no longer needs. */
    if (_linenum != 0) {
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
  __lexscan_last_tokend = _lexscan_last_tokend;
  __lexscan_last_toktype = _lexscan_last_toktype;
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

void Parser::parser_reset() {
    _pfnum = 0;
    NextToklen = 0;
    *_buffer = '\0';
    _continuation_prompt = 0;
    TopOfOperStack = -1;
    TopOfParenStack = -1;
    *_token = '\0';
    _token_state_save = TOK_WHITESPACE;
}
    
