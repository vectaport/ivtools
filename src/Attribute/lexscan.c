/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

#include <Attribute/attrvalue.h>
#include <Attribute/lexscan.h>
#include <Attribute/_comutil.h>
#include <OS/math.h>

#include <string.h>

/*****************************************************************************/

LexScan::LexScan() : ComTerpModule() 
{
    init();
}

LexScan::LexScan(const char* path) : ComTerpModule(path) 
{
    init();
}


LexScan::LexScan(void* inptr, char*(*infunc)(char*,int,void*), 
		 int(*eoffunc)(void*), int(*errfunc)(void*)) 
: ComTerpModule(inptr, infunc, eoffunc, errfunc)
{
    init();
}

LexScan::~LexScan() 
{
    delete _tokbuf;
}

void LexScan::init() 
{
    _begcmt = (char*) "/*";
    _endcmt = (char*) "*/";
    _tokbuf = new char[_bufsiz];
}

attr_value LexScan::get_next_token(unsigned int& toktype)
{
    unsigned int toklen, tokstart;
    int status = lexscan (_inptr, _infunc, _eoffunc, _errfunc, NULL, NULL,
			  _begcmt, _endcmt, '#', "//", _buffer, _bufsiz, &_bufptr,
			  _token, _toksiz, &toklen, &toktype, &tokstart, &_linenum);
    attr_value retval;
    switch (toktype) {
    case TOK_IDENTIFIER:  
    case TOK_STRING:      retval.symval.symid = symbol_add(_token); break;
    case TOK_CHAR:        retval.charval = *_token; break;
    case TOK_DFINT:       retval.dfintval = *(int*)_token; break;
    case TOK_DFUNS:       retval.dfunsval = *(unsigned int*)_token; break;
    case TOK_LNINT:       retval.lnintval = *(long*)_token; break;
    case TOK_LNUNS:       retval.lnunsval = *(unsigned long*)_token; break;
    case TOK_FLOAT:       retval.floatval = *(float*)_token; break;
    case TOK_DOUBLE:      retval.doublval = *(double*)_token; break;
    case TOK_OPERATOR:    retval.symval.symid = symbol_add(_token); break;
    case TOK_EOL:         break;
    case TOK_EOF:         break;
    default:              break;
    }
    return retval;
}

const char* LexScan::get_next_token_string(unsigned int& toktype)
{
    unsigned int toklen, tokstart;
    int status = lexscan (_inptr, _infunc, _eoffunc, _errfunc, NULL, NULL,
			  _begcmt, _endcmt, '#', "//", _buffer, _bufsiz, &_bufptr,
			  _token, _toksiz, &toklen, &toktype, &tokstart, &_linenum);
    unsigned tok_buflen = _bufptr-tokstart;
    strncpy(_tokbuf, _buffer+tokstart, tok_buflen);
    _tokbuf[tok_buflen] = '\0';
    return _tokbuf;
}

AttributeValue* LexScan::get_attrval(char* buf, unsigned int bufsiz)
{
    static int nilsym = symbol_add("nil");
    reset();
    int n = Math::min((int)bufsiz,_bufsiz-2);
    memcpy(_buffer, buf, n);
    if (_buffer[n-1]!='\n') {
      _buffer[n] = '\n';
      _buffer[n+1] = '\0';
    } else 
      _buffer[n] = '\0';
    unsigned int toktype;
    attr_value tokval = get_next_token(toktype);
    AttributeValue::ValueType valtype; 
    switch (toktype) {
    case TOK_IDENTIFIER:  valtype = tokval.dfintval==nilsym ? AttributeValue::UnknownType : AttributeValue::SymbolType; break;
    case TOK_STRING:      valtype = AttributeValue::StringType; break;
    case TOK_CHAR:        valtype = AttributeValue::CharType; break;
    case TOK_DFINT:       valtype = AttributeValue::IntType; break;
    case TOK_DFUNS:       valtype = AttributeValue::UIntType; break;
    case TOK_LNINT:       valtype = AttributeValue::LongType; break;
    case TOK_LNUNS:       valtype = AttributeValue::ULongType; break;
    case TOK_FLOAT:       valtype = AttributeValue::FloatType; break;
    case TOK_DOUBLE:      valtype = AttributeValue::DoubleType; break;
    case TOK_EOF:         valtype = AttributeValue::EofType; break;
    case TOK_OPERATOR:    valtype = AttributeValue::OperatorType; break;
    default:              valtype = AttributeValue::UnknownType; break;
    }

    return new AttributeValue(valtype, tokval);
}



const int LexScan::get_next_value(AttributeValue* attrval, char delim)
{
    static int minus_symid = symbol_add("-");
    static int plus_symid = symbol_add("+");
    attr_value token;
    unsigned int toktype;
    int negate = 0;
    do {
        token = get_next_token(toktype);
	if(toktype == TOK_OPERATOR && token.symval.symid==minus_symid) {
	  negate = 1;
	  token = get_next_token(toktype);
	}
	else if(toktype == TOK_OPERATOR && token.symval.symid==plus_symid) {
	  token = get_next_token(toktype);
	}
    } while(toktype == TOK_OPERATOR && *symbol_pntr(token.symval.symid)==delim);
    if (toktype==TOK_EOF)
      return 0;
    else if (toktype==TOK_EOL)
      return 2;
    else {
      AttributeValue tokval((AttributeValue::ValueType)toktype, token);
      if (negate) tokval.negate();
      *attrval = tokval;
      return 1;
    }
}
