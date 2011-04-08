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

/*
 * GraphCatalog implementation.
 */

#include <OverlayUnidraw/paramlist.h>

#include <GraphUnidraw/graphcatalog.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/nodecomp.h>

#include <TopoFace/topoedge.h>

#include <OverlayUnidraw/ovimport.h>

#include <UniIdraw/idarrows.h>
#include <UniIdraw/idcomp.h>

#include <Unidraw/Components/ellipse.h>
#include <Unidraw/Components/psformat.h>
#include <Unidraw/Components/text.h>

#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Graphic/picture.h>

#include <InterViews/transformer.h>

#include <ctype.h>
#include <stdio.h>
#include <stream.h>
#include <string.h>
#if __GNUG__>=3
#include <fstream.h>
#endif

/*****************************************************************************/

GraphCatalog::GraphCatalog (const char* name, Creator* creator) 
    : OverlayCatalog(name, creator) {
    _import = false;
}

boolean GraphCatalog::Save (Component* c, const char* name) {
    return OverlayCatalog::Save(c, name); 
}

boolean GraphCatalog::Retrieve (const char* pathname, Component*& comp) {
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
	    pfbuf = new fileptr_filebuf(fptr, input);
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
	    if (strcmp(sbuf, "graphdraw") == 0 || 
		strcmp(sbuf, "netdraw") == 0 || 
		strcmp(sbuf, "graph-idraw") == 0) { 
	        _import ? comp = new GraphComp(in, name, _parent) :
		    comp = new GraphIdrawComp(in, name, _parent);
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

