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
 * ComCatalog implementation.
 */

#include <DrawServ/comcatalog.h>
#include <DrawServ/comcomps.h>
#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/nodecomp.h>
#include <OverlayUnidraw/ovimport.h>
#include <Attribute/paramlist.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

ComCatalog::ComCatalog (
    const char* name, Creator* creator
) : OverlayCatalog(name, creator) {
}

boolean ComCatalog::Retrieve (const char* filename, Component*& comp) {
    FILE* fptr = nil;
    boolean compressed = false;
    char* name = strdup(filename);
    if (Valid(name, comp)) {
        _valid = true;

    } else {
        filebuf fbuf;
	if (strcmp(name, "-") == 0) {
	    _valid = fbuf.attach(fileno(stdin)) != 0;
	    name = nil;
	} else {
	    fptr = fopen(name, "r");
	    fptr = OvImportCmd::CheckCompression(fptr, name, compressed);
	    _valid = fptr ? fbuf.attach(fileno(fptr)) != 0 : false;
	    if (compressed) {
		int namelen = strlen(name);
		if (strcmp(name+namelen-3,".gz")==0) name[namelen-3] = '\0';
		else if (strcmp(name+namelen-2,".Z")==0) name[namelen-2] = '\0';
	    }
	}
	
        if (_valid) {
	    istream in(&fbuf);
	    const char* command = "drawserv";
	    int len = strlen(command)+1;
	    char buf[len];

	    char ch;
	    while (isspace(ch = in.get())); in.putback(ch);
	    ParamList::parse_token(in, buf, len);
	    if (strcmp(buf, command) == 0) { 
		comp = new ComIdrawComp(in, name, _parent);
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


OverlayComp* ComCatalog::ReadComp(const char* name, istream& in, OverlayComp* parent) {
  OverlayComp* child = nil;

  if (strcmp(name, "edge") == 0)
     child = new EdgeComp(in, parent);
  
  else if (strcmp(name, "node") == 0) 
     child = new NodeComp(in, parent);
  
  else if (strcmp(name, "graph") == 0) 
     child = new GraphComp(in, nil, parent);
     
  else
     child = OverlayCatalog::ReadComp(name, in, parent);
    
  return child;
}
