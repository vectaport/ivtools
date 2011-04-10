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

#include <Attribute/commodule.h>
#include <Attribute/_comutil.h>

#include <ctype.h>

#define TITLE "ComTerpModule"
#if BUFSIZ>1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

/*****************************************************************************/

int ComTerpModule::dmm_init = 0;

ComTerpModule::ComTerpModule() {
    init();
}

ComTerpModule::ComTerpModule(const char* path) {
    init();
    _inptr = (void*)fopen(path, "r");
}

ComTerpModule::ComTerpModule(FILE* fptr) {
    init();
    _inptr = (void*)fptr;
}

ComTerpModule::ComTerpModule(void* inptr, infuncptr infunc,
				 eoffuncptr eoffunc, errfuncptr errfunc) {
    init();
    _inptr = inptr;
    _infunc = infunc;
    _eoffunc = eoffunc;
    _errfunc = errfunc;
}

ComTerpModule::~ComTerpModule() {

    /* Free memory associated with dmm system */
    if (dmm_init && dmm_mblock_free() != 0) 
        KANRET ("error in call to dmm_mblock_free");

    delete _buffer;
    delete _token;
}

void ComTerpModule::init() {
    /* Initialize dmm system */
    if (!dmm_init) {
	if(dmm_mblock_alloc(1000000L) != 0) 
	    KANRET("error in call to dmm_mblock_alloc");
	dmm_init = 1;
    }

    _inptr = stdin;
    _infunc = (infuncptr)&fgets;
    _eoffunc = (eoffuncptr)&ffeof;
    _errfunc = (errfuncptr)&fferror;
    _outptr = stdout;
    _outfunc = (outfuncptr)&fputs;
    _buffer = new char[BUFSIZ*BUFSIZ];
    _bufsiz = BUFSIZ*BUFSIZ;
    _token = new char[BUFSIZ*BUFSIZ];
    _toksiz = BUFSIZ*BUFSIZ;
    _linenum = 0;
}

void ComTerpModule::reset() {
    _buffer[0] = '\0';
    _token[0] = '\0';
    _linenum = 0;
}

