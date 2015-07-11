/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * X11-dependent request error handling
 */

#include <InterViews/reqerr.h>
#include <IV-X11/Xlib.h>
#include <IV-X11/Xutil.h>

static ReqErr* errhandler;

ReqErr::ReqErr() {
    /* no constructor code currently necessary */
}

ReqErr::~ReqErr() {
    if (errhandler == this) {
	errhandler = nil;
    }
}

void ReqErr::Error() {
    /* default is to do nothing */
}

static int DoXError(XDisplay* errdisplay, XErrorEvent* e) {
    register ReqErr* r = errhandler;
    if (r != nil) {
	r->msgid = e->serial;
	r->code = e->error_code;
	r->request = e->request_code;
	r->detail = e->minor_code;
	r->id = (void*)e->resourceid;
	XGetErrorText(errdisplay, r->code, r->message, sizeof(r->message));
	r->Error();
    }
    return 0;
}

ReqErr* ReqErr::Install() {
    if (errhandler == nil) {
	XSetErrorHandler(DoXError);
    }
    ReqErr* r = errhandler;
    errhandler = this;
    return r;
}
