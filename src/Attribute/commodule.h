/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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

#if !defined(_commodule_h)
#define _commodule_h

#include <stdio.h>
#include <iostream.h>

extern "C" {
#include <ComUtil/comterp.h>
}

//: pointer to fgets-like function.
typedef char* (*infuncptr)(char*,int,void*);
//: pointer to feof-like function.
typedef int (*eoffuncptr)(void*);
//: pointer to ferror-like function.
typedef int (*errfuncptr)(void*);
//: pointer to fputs-like function.
typedef int (*outfuncptr)(const char*, void*);

//: base class for C++ wrappers of Fischer-LeBlanc style compiler pipeline.
// This lives here, instead of in the ComTerp library, so that the LexScan
// derivative class can be used by the ParamList mechanism.
class ComTerpModule {
public:
    ComTerpModule();  
    // construct for stdin/stdout
    ComTerpModule(const char* path); 
    // construct to read from 'path'.
    ComTerpModule(FILE* fptr);
    // construct to read from 'fptr'.
    ComTerpModule(void* inptr, infuncptr infunc,
		    eoffuncptr eoffunc, errfuncptr errfunc);
    // construct to use arbitrary function pointers that are passed
    // an arbitrary void* pointer.
    virtual ~ComTerpModule();

    void reset();
    // re-allocate internal buffers and reset internal pointers.

    int infix_symid(const char*);
    // symbol id for binary infix operator
    int prefix_symid(const char*);
    // symbol id for unary prefix operator
    int postfix_symid(const char*);
    // symbol id for unary postfix operator

    void op_symid(const char*, int& infix, int& prefix, int& postfix);
    // symbol ids for binary infix, unary prefix and postfix for a given
    // operator string.

protected:
    void init();
    // allocate internal buffers and initialize internal pointers.

    void* _inptr;
    infuncptr _infunc;
    eoffuncptr _eoffunc;
    errfuncptr _errfunc;
    void* _outptr;
    outfuncptr _outfunc;

protected:
    char* _buffer;
    int _bufsiz;
    unsigned _bufptr;
    char* _token;
    int _toksiz;
    unsigned _linenum;

    int _buffer_alloc;
    int _token_alloc;

    static int dmm_init;
};

#endif /* !defined(_commodule_h) */
