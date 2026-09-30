/*
 * Copyright (c) 2015 Eta Compute Inc.
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 1998 Vectaport Inc.
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

#include <ComTerp/parsefunc.h>
#include <ComTerp/iofunc.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <iostream.h>
#if __GNUC__>=3
#include <fstream.h>
#endif

#define TITLE "ParseFunc"


/*****************************************************************************/ 
ParseFunc::ParseFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ParseFunc::execute() {
    static int flat_symid = symbol_add("flat");
    ComValue fileobjv(stack_arg(0));
    ComValue flatv(stack_key(flat_symid));
    boolean flatflag = flatv.is_true();
    reset_stack();

    FileObj *fileobj = (FileObj*)fileobjv.geta(FileObj::class_symid());
    FILE* fptr = NULL;
    if (fileobj && fileobj->fptr()) {
	fptr = fileobj->fptr();
    } else {
	PipeObj *pipeobj = (PipeObj*)fileobjv.geta(PipeObj::class_symid());
	if (pipeobj && pipeobj->rdfptr())
	    fptr = pipeobj->rdfptr();
    }

    /* a closed or never-opened file/pipe (e.g. open() on a nonexistent
       path) carries no FILE* -- report nil rather than handing a null
       FILE* to the C parser chain, which assumes a live stream */
    if (!fptr) {
	push_stack(ComValue::nullval());
	return;
    }

    AttributeValueList* avl = comterpserv()->parse_next_expr(fptr, flatflag);

    if (flatflag) {
	ComValue retval(avl ? ComValue(avl) : ComValue::nullval());
	push_stack(retval);
    } else
	push_stack(ComValue::trueval());
}
