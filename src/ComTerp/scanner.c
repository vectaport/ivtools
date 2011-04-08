/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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
#include <ComTerp/scanner.h>
#include <ComTerp/_comutil.h>

#include <string.h>

Scanner::Scanner() : LexScan() 
{
}

Scanner::Scanner(const char* path) : LexScan(path) 
{
}


Scanner::Scanner(void* inptr, char*(*infunc)(char*,int,void*), 
		 int(*eoffunc)(void*), int(*errfunc)(void*)) 
: LexScan(inptr, infunc, eoffunc, errfunc)
{
}


Scanner::~Scanner() 
{
}

const void* Scanner::get_next_token(unsigned int& toktype)
{
    unsigned int toklen, tokstart;
    int status = scanner (_inptr, _infunc, _eoffunc, _errfunc, NULL, NULL,
			  _buffer, _bufsiz, &_bufptr, _token, _toksiz, &toklen, 
			  &toktype, &tokstart, &_linenum);
    return _token;
}

const char* Scanner::get_next_token_string(unsigned int& toktype)
{
    unsigned int toklen, tokstart;
    int status = scanner (_inptr, _infunc, _eoffunc, _errfunc, NULL, NULL,
			  _buffer, _bufsiz, &_bufptr, _token, _toksiz, &toklen, 
			  &toktype, &tokstart, &_linenum);
    unsigned tok_buflen = _bufptr-tokstart;
    strncpy(_tokbuf, _buffer+tokstart, tok_buflen);
    _tokbuf[tok_buflen] = '\0';
    return _tokbuf;
}


