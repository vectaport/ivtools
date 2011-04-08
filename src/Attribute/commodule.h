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

#if !defined(_commodule_h)
#define _commodule_h

#include <stdio.h>
#include <iostream.h>

typedef char* (*infuncptr)(char*,int,void*);
typedef int (*eoffuncptr)(void*);
typedef int (*errfuncptr)(void*);
typedef int (*outfuncptr)(const char*, void*);

class ComTerpModule {
public:
    ComTerpModule();
    ComTerpModule(const char* path);
    ComTerpModule(FILE* fptr);
    ComTerpModule(void* inptr, infuncptr infunc,
		    eoffuncptr eoffunc, errfuncptr errfunc);
    virtual ~ComTerpModule();

    void reset();

protected:
    void init();

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
