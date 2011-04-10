/*
 * Copyright (c) 1994, 1995 Vectaport
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * FrameScript* and FrameIdrawScript implementation.
 */


#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/framefile.h>
#include <FrameUnidraw/framescripts.h>

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovfile.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/ovstencil.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/paramlist.h>
#include <OverlayUnidraw/textfile.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>

#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <ctype.h>
#include <stdio.h>
#include <stream.h>
#include <string.h>

/*****************************************************************************/

FrameOverlaysScript::FrameOverlaysScript (FrameOverlaysComp* subj) : OverlaysScript(subj) {}

ClassId FrameOverlaysScript::GetClassId () { return FRAME_OVERLAYS_SCRIPT; }

boolean FrameOverlaysScript::IsA (ClassId id) {
    return FRAME_OVERLAYS_SCRIPT == id || OverlaysScript::IsA(id);
}
 
/*****************************************************************************/

FrameScript::FrameScript (FrameComp* subj) : OverlaysScript(subj) {
  _suppress_frame = false;
}

ClassId FrameScript::GetClassId () { return FRAME_SCRIPT; }

boolean FrameScript::IsA (ClassId id) {
    return FRAME_SCRIPT == id || OverlaysScript::IsA(id);
}

boolean FrameScript::Definition (ostream& out) {
    Iterator i;
    boolean status = true;

    if (!_suppress_frame) out << "frame(\n";
    
    static int readonly_symval = symbol_add("readonly");
    boolean outflag = false;
    for (First(i); status && !Done(i); ) {
	OverlayScript* sv = (OverlayScript*)GetView(i);
	boolean readonly = false;
	AttributeList* al;
	if (al = sv->GetOverlayComp()->attrlist()) {
	  AttributeValue* av = al->find(readonly_symval);
	  if (av) readonly = av->is_true();
	}
	if (!readonly) {
	  if (outflag) out << ",\n";
	  Indent(out);
	  status = sv->Definition(out);
	  outflag = true;
	} 
	Next(i);
    }
    
    out << "\n";
    Indent(out);
    Attributes(out);
    if (!_suppress_frame) out << ")";

    return status;
}    

int FrameScript::ReadChildren (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  OverlayComp* child = nil;
  OverlaysComp* comps = (OverlaysComp*)addr1;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];
  char* buf = buf1;
  
  while (in.good()) {
    if (read_name(in, buf, BUFSIZ)) break;

    int status;
    if (status = read_gsptspic(buf, in, comps)) {
      if (status==-1) break;
    }
    
    else {
      child = read_obj(buf, in, comps);
      if (!child) return -1;
    }
 
    if (child) {
      if (in.good() && child->valid()) {
	comps->Append(child);
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
  return 0;
}

boolean FrameScript::EmitPic(ostream& out, Clipboard* cb1, Clipboard* cb2, boolean prevout) {
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	prevout = GetScript(i)->EmitPic(out, cb1, cb2, prevout);
    }
    return prevout;
}

/*****************************************************************************/

ClassId FramesScript::GetClassId () { return FRAMES_SCRIPT; }

boolean FramesScript::IsA (ClassId id) {
    return FRAMES_SCRIPT == id || FrameScript::IsA(id);
}

FramesScript::FramesScript (FramesComp* subj) : FrameScript(subj) {}

boolean FramesScript::Definition (ostream& out) {
    Iterator i;
    boolean status = true;

    Clipboard* cb = GetPicList();
    if (cb) {
	out << "frames( :pic " << MatchedPic(cb);
    } else {

	out << "frames(\n";

	static int readonly_symval = symbol_add("readonly");
	boolean outflag = false;
	for (First(i); status && !Done(i); ) {
	    OverlayScript* sv = (OverlayScript*)GetView(i);
	    boolean readonly = false;
	    AttributeList* al;
	    if (al = sv->GetOverlayComp()->attrlist()) {
	      AttributeValue* av = al->find(readonly_symval);
	      if (av) readonly = av->is_true();
	    }
	    if (!readonly) {
	      if (outflag) out << ",\n";
	      Indent(out);
	      status = sv->Definition(out);
	      outflag = true;
	    }
	    Next(i);
	}
    }
	
    if (!cb) {
	out << "\n";
	Indent(out);
	Attributes(out);
    } else {
	Transformation(out);
    }
    out << ")";

    return status;
}    

int FramesScript::ReadFrames (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  FrameComp* frame;
  FrameFileComp* framefile;
  OverlayComp* child;
  FramesComp* comps = (FramesComp*)addr1;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];
  char* buf = buf1;

  FrameComp* bgframe = nil;
  Iterator it;
  comps->First(it);
  if (!comps->Done(it)) {
    OverlayComp* comp = (OverlayComp*) comps->GetComp(it);
    if (comp->IsA(FRAME_COMP))
      bgframe = (FrameComp*) comp;
  }

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
  return 0;
}

/*****************************************************************************/

FrameIdrawScript::FrameIdrawScript (FrameIdrawComp* subj) : FramesScript(subj) {
    _gslist = nil;
    _ptslist = nil;
    _piclist1 =  _piclist2 = nil;
    _gs_compacted = _pts_compacted = _pic_compacted = false;
    _by_pathname = true;
}

FrameIdrawScript::~FrameIdrawScript() {
    delete _gslist;
    delete _ptslist;
    delete _piclist1;
    delete _piclist2;
}

ClassId FrameIdrawScript::GetClassId () { return FRAME_IDRAW_SCRIPT; }

boolean FrameIdrawScript::IsA (ClassId id) { 
    return FRAME_IDRAW_SCRIPT == id || FramesScript::IsA(id);
}

boolean FrameIdrawScript::EmitPic(ostream& out, Clipboard* cb1, Clipboard* cb2, boolean prevout) {
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	prevout = GetScript(i)->EmitPic(out, cb1, cb2, prevout);
    }
    return prevout;
}

boolean FrameIdrawScript::Emit (ostream& out) {
    out << "flipbook(";

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
    Iterator i;
    First(i);
    if (!Done(i) ) {
	if (prevout) out << ",";
	out << "\n";
    }
    static int readonly_symval = symbol_add("readonly");
    boolean outflag = false;
    for (; status && !Done(i); ) {
	OverlayScript* ev = (OverlayScript*)GetView(i);
	boolean readonly = false;
	AttributeList* al;
	if (al = ev->GetOverlayComp()->attrlist()) {
	  AttributeValue* av = al->find(readonly_symval);
	  if (av) readonly = av->is_true();
	}
	if (!readonly) {
	  if (outflag) out << ",\n";
	  Indent(out);
	  status = ev->Definition(out);
	  outflag = true;
	}
	Next(i);
    }

    out << "\n";
    FullGS(out);
    Annotation(out);
    Attributes(out);
    out << ")\n";
    return status;
}

Clipboard* FrameIdrawScript::GetGSList() {
    return _gslist;
}

Clipboard* FrameIdrawScript::GetPtsList() {
    return _ptslist;
}

Clipboard* FrameIdrawScript::GetPicList() {
    return _piclist1;
}

void FrameIdrawScript::SetCompactions(boolean gs, boolean pts, boolean pic) {
    _gs_compacted = gs;
    _pts_compacted = pts;
    _pic_compacted = pic;
}

void FrameIdrawScript::SetByPathnameFlag(boolean flag) {
    _by_pathname = flag;
}

boolean FrameIdrawScript::GetByPathnameFlag() {
    return _by_pathname;
}

