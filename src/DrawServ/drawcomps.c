/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 * DrawIdrawComp implementation.
 */

#include <DrawServ/drawcatalog.h>
#include <DrawServ/drawcomps.h>
#include <DrawServ/drawclasses.h>
#include <FrameUnidraw/framefile.h>
#include <GraphUnidraw/graphclasses.h>
#include <OverlayUnidraw/paramlist.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/Graphic/picture.h>
#include <Attribute/attrlist.h>
#include <iostream.h>
#include <string.h>
#include <fstream>

using std::cerr;

/*****************************************************************************/

ParamList* DrawIdrawComp::_com_idraw_params = nil;

DrawIdrawComp::DrawIdrawComp (const char* pathname, OverlayComp* parent)
: FrameIdrawComp(false, pathname, parent) {
    _graphedges = new UList();
}

DrawIdrawComp::DrawIdrawComp (istream& in, const char* pathname, OverlayComp* parent) : 
FrameIdrawComp(parent) {
    _pathname = _basedir = nil;
    _gslist = nil;
    _ptsbuf = nil;
    SetPathName(pathname);
    _graphedges = new UList();
    _valid = GetParamList()->read_args(in, this);
    delete _gslist;
    if (_ptsbuf) {
	for (int i=0; i<_ptsnum; i++) 
	    Unref(_ptsbuf[i]);
	delete _ptsbuf;
    }
}

DrawIdrawComp::~DrawIdrawComp () {
    delete _graphedges;
}

ClassId DrawIdrawComp::GetClassId () { return DRAW_IDRAW_COMP; }

Component* DrawIdrawComp::Copy () {
    DrawIdrawComp* comps = new DrawIdrawComp(GetPathName());
    if (attrlist()) comps->SetAttributeList(new AttributeList(attrlist()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    for (UList* u = _graphedges->First(); u != _graphedges->End(); u = u->Next()) {
      EdgeComp* edgecomp = (EdgeComp*) (*u)();
      comps->AppendEdge(edgecomp);
    }
    return comps;
}

boolean DrawIdrawComp::IsA (ClassId id) {
    return DRAW_IDRAW_COMP == id || FrameIdrawComp::IsA(id);
}


ParamList* DrawIdrawComp::GetParamList() {
    if (!_com_idraw_params) 
	GrowParamList(_com_idraw_params = new ParamList());
    return _com_idraw_params;
}

void DrawIdrawComp::GrowParamList(ParamList* pl) {
    pl->add_param("edges_nodes", ParamStruct::required, &ParamList::read_int,
		  this, &_num_edge, &_num_node);
    pl->add_param("frames", ParamStruct::required, &DrawIdrawScript::ReadFrames, this, this);
    OverlayComp::GrowParamList(pl);
    return;
}

void DrawIdrawComp::AppendEdge(EdgeComp* comp) {
    _graphedges->Append(new UList(comp));
}

/*****************************************************************************/

DrawIdrawScript::DrawIdrawScript (DrawIdrawComp* subj) : FrameIdrawScript(subj) {
}

DrawIdrawScript::~DrawIdrawScript() {

}

ClassId DrawIdrawScript::GetClassId () { return DRAW_IDRAW_SCRIPT; }

boolean DrawIdrawScript::IsA (ClassId id) { 
    return DRAW_IDRAW_SCRIPT == id || FrameIdrawScript::IsA(id);
}

boolean DrawIdrawScript::Emit (ostream& out) {
    out << script_name() << "(";

    GraphicComp* comps = GetGraphicComp();
    Iterator i;

    int num_edge = 0;
    int num_node = 0;
    for (comps->First(i); !comps->Done(i); comps->Next(i)) {
      GraphicComp* comp = comps->GetComp(i);
      if (comp->IsA(FRAME_COMP)) {
	Iterator j;
	for(comp->First(j); !comp->Done(j); comp->Next(j)) {
	  GraphicComp* subcomp = comp->GetComp(j);
	  if (subcomp->IsA(NODE_COMP))
	    num_node++;
	  else if (subcomp->IsA(EDGE_COMP))
	    num_edge++;
	}
      }
    }
    
    out << num_edge << "," << num_node;

    /* make list and output unique point lists */
    boolean prevout = false;
    if (_pts_compacted) {
	_ptslist = new Clipboard();
	prevout = EmitPts(out, _ptslist, prevout);
    }

    /* make list and output unique graphic states */
    if (_gs_compacted) {
	_gslist = new Clipboard();
	prevout = EmitGS(out, _gslist, prevout);
    }

    /* make list and output unique picture graphics */
    if (_pic_compacted) {
	_piclist1 = new Clipboard();
	_piclist2 = new Clipboard();
	prevout = EmitPic(out, _piclist1, _piclist2, prevout);
    }

    /* output graphic components */
    boolean status = true;
    First(i);
    if (!Done(i) ) {
	if (prevout) out << ",";
	out << "\n";
    }
    for (; status && !Done(i); ) {
	ExternView* ev = GetView(i);
	Indent(out);
        status = ev->Definition(out);
	Next(i);
	if (!Done(i)) out << ",\n";
    }

    out << "\n";
    FullGS(out);
    Annotation(out);
    Attributes(out);
    out << ")\n";
    return status;
}

int DrawIdrawScript::ReadFrames (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  FrameComp* frame;
  FrameFileComp* framefile;
  OverlayComp* child;
  DrawIdrawComp* comps = (DrawIdrawComp*)addr1;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];
  char* buf = buf1;

  DrawCatalog* catalog = (DrawCatalog*)unidraw->GetCatalog();
  int num_edge = (comps)->GetNumEdge();
  int num_node = (comps)->GetNumNode();
  catalog->graph_init(comps, num_edge, num_node);
  
  FrameComp* bgframe = nil;

  while (in.good()) {
    frame = nil;
    framefile = nil;
    child = nil;

    if (read_name(in, buf, BUFSIZ)) break;

    int status;
    if (status = read_gsptspic(buf, in, comps)) {
      if (status==-1) break;
    }

    else if (strcmp(buf, "frame") == 0) {
      frame = new FrameComp(in, comps);
      if (!bgframe) bgframe = frame;

    } else if (strcmp(buf, "framefile") == 0) 	    framefile = new FrameFileComp(in, comps);

    else {
      if (!bgframe) {
	bgframe = new FrameComp(comps);
	comps->Append(bgframe);
      }
      child = read_obj(buf, in, bgframe);
      if (!child) return -1;
    }

    if (frame != nil) {
      if (in.good() && frame->valid()) {
	comps->Append(frame);
      } else {
	/* report failure even if one child fails */
	delete frame;
	return -1;
      }
    }
    if (framefile != nil) {
      Iterator j;
      framefile->First(j);
      FrameIdrawComp* frameidraw = (FrameIdrawComp*)framefile->GetComp(j);
      if (in.good() && frameidraw->valid()) {
	Iterator i;
	frameidraw->First(i);
	frameidraw->Next(i);
	while (!frameidraw->Done(i)) {
	  comps->Append((GraphicComp*)frameidraw->GetComp(i));
	  frameidraw->Next(i);
	}
      } else {
	/* report failure even if one child fails */
	delete framefile;
	return -1;
      }
    }
    if (child) {
      if (in.good() && child->valid()) {
	bgframe->Append(child);
      } else {
	/* report failure even if one child fails */
	if (!*buf && (buf==buf1 ? *buf2 : *buf1)) 
	  cerr << "Error after reading " << (buf==buf1 ? buf2 : buf1) << "\n";
	delete child;
	return -1;
      }
    }
    buf = buf==buf1 ? buf2 : buf1;
  }
  catalog->graph_finish();
  return 0;
}
