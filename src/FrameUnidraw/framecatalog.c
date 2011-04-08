/*
 * Copyright (c) 1994-1997 Vectaport Inc.
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

/*
 * FrameCatalog implementation.
 */

#include <FrameUnidraw/framecatalog.h>
#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecomps.h>

#include <OverlayUnidraw/ovfile.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/paramlist.h>

#include <stdio.h>
#include <stream.h>
#include <string.h>
#include <ctype.h>
#include <fstream.h>


/*****************************************************************************/

FrameCatalog::FrameCatalog (
    const char* name, Creator* creator
) : OverlayCatalog(name, creator) {
}

boolean FrameCatalog::Retrieve (const char* pathname, Component*& comp) {
    FILE* fptr = nil;
    boolean compressed = false;
    char* name = strdup(pathname);
    if (Valid(name, comp)) {
        _valid = true;

    } else {
#if __GNUG__<3
        filebuf fbuf;
#else
        filebuf* pfbuf = nil;
#endif
	if (strcmp(name, "-") == 0) {
#if __GNUG__<3
	    _valid = fbuf.attach(fileno(stdin)) != 0;
#else
	    pfbuf = new fileptr_filebuf(stdin, input);
	    _valid = 1;
#endif
	    name = nil;
	} else {
	    fptr = fopen(name, "r");
	    fptr = OvImportCmd::CheckCompression(fptr, name, compressed);
#if __GNUG__<3
	    _valid = fptr ? fbuf.attach(fileno(fptr)) != 0 : false;
#else
	    pfbuf = fptr ? new fileptr_filebuf(fptr, input) : nil;
	    _valid = fptr ? 1 : 0;
#endif
	    if (compressed) {
		int namelen = strlen(name);
		if (strcmp(name+namelen-3,".gz")==0) name[namelen-3] = '\0';
		else if (strcmp(name+namelen-2,".Z")==0) name[namelen-2] = '\0';
	    }
	}

        if (_valid) {
#if __GNUG__<3
	    istream in(&fbuf);
#else
	    istream in(pfbuf);
#endif

	    char ch;
	    while (isspace(ch = in.get())); in.putback(ch);
	    ParamList::parse_token(in, sbuf, SBUFSIZE);
	    if (strcmp(sbuf, "flipbook") == 0 || 
		strcmp(sbuf, "frame-idraw") == 0) { 
		comp = new FrameIdrawComp(in, name, _parent);
		_valid = in.good() && ((OverlayComp*)comp)->valid();
	    } else if (strcmp(sbuf, "drawtool") == 0 || 
		strcmp(sbuf, "ov-idraw") == 0) { 
		comp = new OverlayIdrawComp(in, name, _parent);
		_valid = in.good() && ((OverlayComp*)comp)->valid();
	    } else 
		_valid = false;

            if (_valid && name) {
                Forget(comp, name);
                Register(comp, name);
            } else if (!_valid) {
		delete comp;
		comp = nil;
	    }
        }

#if __GNUG__>=3
	delete pfbuf;
#endif
    }

    if (fptr) {
	if (compressed) 
	    fclose(fptr);
	else
	    pclose(fptr);
    }
    delete name;
    return _valid;
}

OverlayComp* FrameCatalog::ReadComp(const char* name, istream& in, OverlayComp* parent) {
  OverlayComp* child = nil;

  if (strcmp(name, "picture") == 0 ||
      strcmp(name, "grp") == 0)                 child = new FrameOverlaysComp(in, parent);
  else 
    child = OverlayCatalog::ReadComp(name, in, parent);
  return child;
}
