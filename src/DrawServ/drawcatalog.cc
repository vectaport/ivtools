/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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
 * DrawCatalog implementation.
 */

#include <DrawServ/drawcatalog.h>
#include <DrawServ/drawcomps.h>
#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/nodecomp.h>
#include <OverlayUnidraw/ovimport.h>
#include <TopoFace/topoedge.h>
#include <Attribute/paramlist.h>
#include <ctype.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <fstream.h>

/*****************************************************************************/

DrawCatalog::DrawCatalog (
    const char* name, Creator* creator
) : FrameCatalog(name, creator) {
  _startnode = _endnode = nil;
  _edges = nil;
  _nodes = nil;
  _comps = nil;
}

boolean DrawCatalog::Retrieve (const char* filename, Component*& comp) {
    FILE* fptr = nil;
    boolean compressed = false;
    char* name = strdup(filename);
    if (Valid(name, comp)) {
        _valid = true;

    } else {
        filebuf* pfbuf = nil;
	if (strcmp(name, "-") == 0) {
	    FILEBUFP(pfbuf, stdin, input);
	    _valid = 1;
	    name = nil;
	} else {
	    fptr = fopen(name, "r");
	    fptr = OvImportCmd::CheckCompression(fptr, name, compressed);
	    FILEBUFP(pfbuf, fptr, input);
	    _valid = fptr ? 1 : 0;
	    if (compressed) {
		int namelen = strlen(name);
		if (strcmp(name+namelen-3,".gz")==0) name[namelen-3] = '\0';
		else if (strcmp(name+namelen-2,".Z")==0) name[namelen-2] = '\0';
	    }
	}
	
        if (_valid) {
	    istream in(pfbuf);
	    const char* command = "drawserv";
	    int len = strlen(command)+1;
	    char buf[len];

	    char ch;
	    while (isspace(ch = in.get())) {}; in.putback(ch);
	    ParamList::parse_token(in, buf, len);
	    if (strcmp(buf, "drawserv") == 0) { 
		comp = new DrawIdrawComp(in, name, _parent);
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
	delete pfbuf;
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


OverlayComp* DrawCatalog::ReadComp(const char* name, istream& in, OverlayComp* parent) {
  OverlayComp* child = nil;

  if (strcmp(name, "edge") == 0) {
     child = new EdgeComp(in, parent);
     EdgeComp* comp = (EdgeComp*)child;
     _startnode[_edge_cnt] = comp->GetStartNode();
     _endnode[_edge_cnt] = comp->GetEndNode();
     _edges[_edge_cnt] = comp;
     _edge_cnt++;
  }
  
  else if (strcmp(name, "node") == 0) {
     child = new NodeComp(in, parent);
     _nodes[_node_cnt] = (NodeComp*)child;
     _node_cnt++;
  }
  
  else if (strcmp(name, "graph") == 0) 
     child = new GraphComp(in, nil, parent);
     
  else
     child = OverlayCatalog::ReadComp(name, in, parent);
    
  return child;
}

void DrawCatalog::graph_init(DrawIdrawComp* comps, int num_edge, int num_node) {
  delete _startnode;
  delete _endnode;
  delete _edges;
  delete _nodes;
  _comps = comps;
  _startnode = new int[num_edge];
  _endnode = new int[num_edge];
  _edges = new EdgeComp*[num_edge];
  _nodes = new NodeComp*[num_node];
  _num_edge = num_edge;
  _num_node = num_node;
  _edge_cnt = 0;
  _node_cnt = 0;
}

void DrawCatalog::graph_finish() {
  for (int i=0; i<_num_edge; i++) {
    int start_id = _startnode[i];
    int end_id = _endnode[i];
    if (start_id < 0 || end_id < 0)
      _comps->AppendEdge(_edges[i]);
    _edges[i]->AttachNodes(start_id < 0 ? nil : _nodes[start_id], 
			   end_id < 0 ? nil : _nodes[end_id]);
    #if defined(GRAPH_OBSERVABLES)
    if (start_id >=0 && end_id >=0) 
      _edges[i]->NodeStart()->attach(_edges[i]->NodeEnd());
    #endif
  }
  delete _startnode; _startnode = nil;
  delete _endnode; _endnode = nil;
  delete _edges; _edges = nil;
  delete _nodes; _nodes = nil;
  _comps = nil;
}

