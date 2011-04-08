/*
 * Copyright (c) 1999 Vectaport Inc.
 * Copyright (c) 1994, 1995 Vectaport Inc., Cider Press
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

#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/framescripts.h>
#include <FrameUnidraw/framestates.h>
#include <FrameUnidraw/frameviews.h>

#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/transforms.h>

#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Graphic/picture.h>

#include <InterViews/world.h>

#include <Attribute/attrlist.h>

#include <string.h>

/*****************************************************************************/

static void NullGS (Graphic* g) { FullGraphic null; *g = null; }

/*****************************************************************************/

ParamList* FrameOverlaysComp::_frame_ovcomps_params = nil;

FrameOverlaysComp::FrameOverlaysComp(OverlayComp* parent) : OverlaysComp(parent) {}
FrameOverlaysComp::FrameOverlaysComp(Graphic* g, OverlayComp* parent) : OverlaysComp(g, parent) {}

FrameOverlaysComp::FrameOverlaysComp (istream& in, OverlayComp* parent) : OverlaysComp(parent) {
    _valid = GetParamList()->read_args(in, this);
}

ParamList* FrameOverlaysComp::GetParamList() {
    if (!_frame_ovcomps_params) 
	GrowParamList(_frame_ovcomps_params = new ParamList());
    return _frame_ovcomps_params;
}

void FrameOverlaysComp::GrowParamList(ParamList* pl) {
    pl->add_param("kids", ParamStruct::optional, &FrameScript::ReadChildren, this, this);
    OverlaysComp::GrowParamList(pl);
    return;
}

ClassId FrameOverlaysComp::GetClassId() { return FRAME_OVERLAYS_COMP; }

boolean FrameOverlaysComp::IsA(ClassId id) {
    return id == FRAME_OVERLAYS_COMP || OverlaysComp::IsA(id);
}

Component* FrameOverlaysComp::Copy () {
    FrameOverlaysComp* comps = new FrameOverlaysComp(new Picture(GetGraphic()));
    if (attrlist()) comps->SetAttributeList(new AttributeList(attrlist()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comps;
}

void FrameOverlaysComp::Interpret (Command* cmd) {
    if (cmd->IsA(GROUP_CMD) || cmd->IsA(FRONT_CMD) || cmd->IsA(BACK_CMD)) {
        cmd->GetClipboard()->Append(this);
    } else 
	OverlaysComp::Interpret(cmd);
}

void FrameOverlaysComp::Uninterpret (Command* cmd) {
    FrameEditor* ed = (FrameEditor*)cmd->GetEditor();
    FramesView *views = (FramesView*)ed->GetViewer()->GetGraphicView();
    
    if (cmd->IsA(GROUP_CMD)) {
	FrameComp* framecomp = (FrameComp*)ed->GetFrame()->GetGraphicComp();
	framecomp->Uninterpret(cmd);
    }
    else
	OverlaysComp::Uninterpret(cmd);
}

/*****************************************************************************/

ParamList* FrameComp::_frame_comp_params = nil;

FrameComp::FrameComp(OverlayComp* parent) : OverlaysComp(parent) {}
FrameComp::FrameComp(Graphic* g, OverlayComp* parent) : OverlaysComp(g, parent) {}

FrameComp::FrameComp (istream& in, OverlayComp* parent) 
: OverlaysComp(parent) { 
    _valid = GetParamList()->read_args(in, this);
}

ParamList* FrameComp::GetParamList() {
    if (!_frame_comp_params) 
	GrowParamList(_frame_comp_params = new ParamList());
    return _frame_comp_params;
}

void FrameComp::GrowParamList(ParamList* pl) {
    pl->add_param("kids", ParamStruct::required, &FrameScript::ReadChildren, this, this);
    OverlayComp::GrowParamList(pl);
    return;
}

ClassId FrameComp::GetClassId() { return FRAME_COMP; }

boolean FrameComp::IsA(ClassId id) {
    return id == FRAME_COMP || OverlaysComp::IsA(id);
}

Component* FrameComp::Copy () {
    FrameComp* comp = new FrameComp(new Picture(GetGraphic()));
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comp->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comp;
}

void FrameComp::Interpret (Command* cmd) {
    FrameEditor* ed = (FrameEditor*)cmd->GetEditor();
    if (cmd->IsA(DELETE_CMD)) {
        Clipboard* cb = cmd->GetClipboard();
        Selection* s = ed->GetSelection();

        if (cb == nil) {
            if (s->IsEmpty()) {
                return;
            }
            cmd->SetClipboard(cb = new Clipboard);
            cb->Init(s);
        }
        s->Clear();
        Iterator i;

        for (cb->First(i); !cb->Done(i); cb->Next(i)) {
            OverlayComp* comp = (OverlayComp*) cb->GetComp(i);
            unidraw->CloseDependents(comp);
            comp->Interpret(cmd);
            StorePosition(comp, cmd);
            Remove(comp);
        }
        Notify();
        unidraw->Update();

    } else if (cmd->IsA(CUT_CMD)) {
        Clipboard* cb = cmd->GetClipboard();
        Selection* s = ed->GetSelection();

        if (cb == nil) {
            if (s->IsEmpty()) {
                return;
            }
            GraphicView* views = ed->GetFrame();
            s->Sort(views);
            cmd->SetClipboard(cb = new Clipboard);
            cb->Init(s);

            Clipboard* globalcb = unidraw->GetCatalog()->GetClipboard();
            globalcb->DeleteComps();
            globalcb->CopyInit(s);
        }
        s->Clear();
        Iterator i;

        for (cb->First(i); !cb->Done(i); cb->Next(i)) {
            OverlayComp* comp = (OverlayComp*)cb->GetComp(i);
            unidraw->CloseDependents(comp);
            comp->Interpret(cmd);
            StorePosition(comp, cmd);
            Remove(comp);
        }
        Notify();
        unidraw->Update();

    } else if (cmd->IsA(DUP_CMD)) {
        GraphicView* views = ed->GetFrame();
        GraphicComp* prev, *dup1;
        Iterator i, pos;
        Clipboard* cb = cmd->GetClipboard();
        const float offset = 8;
        MoveCmd move(ed, offset, offset);

        if (cb == nil) {
            Selection* s = ed->GetSelection();

            if (s->IsEmpty()) {
                return; 
            }
            cmd->SetClipboard(cb = new Clipboard);
            s->Sort(views);

            for (s->First(i); !s->Done(i); s->Next(i)) {
                dup1 = (GraphicComp*) s->GetView(i)->GetGraphicComp()->Copy();
                dup1->Interpret(&move);
                cb->Append(dup1);
            }
            cb->First(i);
            dup1 = cb->GetComp(i);
            Last(pos);
            prev = GetComp(pos);
            cmd->Store(dup1, new VoidData(prev));

        } else {
            cb->First(i);
            dup1 = cb->GetComp(i);
            VoidData* vd = (VoidData*) cmd->Recall(dup1);
            prev = (GraphicComp*) vd->_void;
            SetComp(prev, pos);
        }

        for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
            InsertAfter(pos, cb->GetComp(i));
        }

        Notify();
        SelectClipboard(cb, ed);
        unidraw->Update();

    } else if (cmd->IsA(GROUP_CMD)) {
        GroupCmd* gcmd = (GroupCmd*) cmd;
        FrameOverlaysComp* group = (FrameOverlaysComp*)gcmd->GetGroup();
	Clipboard* cb = cmd->GetClipboard();
	NullGS(group->GetGraphic());
	Group(cb, group, cmd);
	Notify();
	SelectViewsOf(group, ed);
	unidraw->Update();
    } else if (cmd->IsA(UNGROUP_CMD)) {
	UngroupCmd* ucmd = (UngroupCmd*) cmd;
 	Clipboard* cb = cmd->GetClipboard();
 	Clipboard* kids = new Clipboard;
 	ucmd->SetKids(kids);
 	Iterator i;
 	
 	for (cb->First(i); !cb->Done(i); cb->Next(i)) {
 	    OverlayComp* parent = (OverlayComp*)cb->GetComp(i);
 	    unidraw->CloseDependents(parent);
 	    Ungroup(parent, kids, cmd);
 	}
 	Notify();
 	SelectClipboard(kids, ed);
 	unidraw->Update();
    } else if (cmd->IsA(FRONT_CMD) || cmd->IsA(BACK_CMD)) {
	Clipboard* cb = cmd->GetClipboard();
	Iterator i;

	if (cmd->IsA(FRONT_CMD)) {
	    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
		OverlayComp* comp = (OverlayComp*) cb->GetComp(i);
		StorePosition(comp, cmd);
		Remove(comp);
		Append(comp);
	    }
	} else {
	    for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
		OverlayComp* comp = (OverlayComp*) cb->GetComp(i);
		StorePosition(comp, cmd);
		Remove(comp);
		Prepend(comp);
	    }
	}
	Notify();
	unidraw->Update();
    } else {
	OverlaysComp::Interpret(cmd);
    }
}

void FrameComp::Uninterpret (Command* cmd) {
    Editor* ed = cmd->GetEditor();
    if (cmd->IsA(DELETE_CMD)) {
        Clipboard* cb = cmd->GetClipboard();

        if (cb != nil) {
            Iterator i;

            for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
                OverlayComp* comp = (OverlayComp*) cb->GetComp(i);
                RestorePosition(comp, cmd);
                comp->Uninterpret(cmd);
            }
            Notify();
            SelectClipboard(cb, ed);
            unidraw->Update();
        }

    } else if (cmd->IsA(CUT_CMD)) {

        Clipboard* cb = cmd->GetClipboard();

        if (cb != nil) {
            Iterator i;

            for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
                OverlayComp* comp = (OverlayComp*) cb->GetComp(i);
                RestorePosition(comp, cmd);
                comp->Uninterpret(cmd);
            }
            Notify();
            SelectClipboard(cb, ed);
            unidraw->Update();
        }

    } else if (cmd->IsA(GROUP_CMD)) {
	Clipboard* cb = cmd->GetClipboard();

	Iterator i;
	cb->First(i);
	GraphicComp* group = (GraphicComp*) cb->GetComp(i)->GetParent();
	unidraw->CloseDependents(group);

	for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
	    RestorePosition((OverlayComp*) cb->GetComp(i), cmd);
	}
	Remove(group);
	Notify();
	SelectClipboard(cb, ed);
	unidraw->Update();
    } else if (cmd->IsA(UNGROUP_CMD)) {
        UngroupCmd* ucmd = (UngroupCmd*) cmd;
	Clipboard* cb = ucmd->GetClipboard();
	Clipboard* kids = ucmd->GetKids();
	Clipboard insertedParents;
	Iterator k;

	for (kids->First(k); !kids->Done(k); kids->Next(k)) {
	    GraphicComp* kid = kids->GetComp(k);
	    UngroupData* ud = (UngroupData*) cmd->Recall(kid);
	    GraphicComp* parent = ud->_parent;
	    *kid->GetGraphic() = *ud->_gs;

	    if (!insertedParents.Includes(parent)) {
		GSData* gd = (GSData*) cmd->Recall(parent);
		*parent->GetGraphic() = *gd->_gs;

		Iterator insertPt;
		SetComp(kid, insertPt);
		InsertBefore(insertPt, parent);
		insertedParents.Append(parent);
	    }
	    Remove(kid);
	    parent->Append(kid);
	}
	Notify();
	SelectClipboard(cb, ed);
	unidraw->Update();

	delete kids;
	ucmd->SetKids(nil);
    } else if (cmd->IsA(FRONT_CMD)) {
	Clipboard* cb = cmd->GetClipboard();
	Iterator i;

	for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
	    RestorePosition((OverlayComp*) cb->GetComp(i), cmd);
	}
	Notify();
	SelectClipboard(cb, ed);
	unidraw->Update();
    } else if (cmd->IsA(BACK_CMD)) {
	Clipboard* cb = cmd->GetClipboard();
	Iterator i;

	for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	    RestorePosition((OverlayComp*) cb->GetComp(i), cmd);
	}
	Notify();
	SelectClipboard(cb, ed);
	unidraw->Update();
    } else {
	OverlaysComp::Uninterpret(cmd);
    }
}

/*****************************************************************************/

ParamList* FramesComp::_frame_comps_params = nil;

FramesComp::FramesComp(OverlayComp* parent) : FrameComp(parent) {} 
FramesComp::FramesComp(Graphic* g, OverlayComp* parent) : FrameComp(g, parent) {} 

FramesComp::FramesComp (istream& in, OverlayComp* parent) 
: FrameComp(parent) { 
    _valid = GetParamList()->read_args(in, this);
}

ParamList* FramesComp::GetParamList() {
    if (!_frame_comps_params) 
	GrowParamList(_frame_comps_params = new ParamList());
    return _frame_comps_params;
}

void FramesComp::GrowParamList(ParamList* pl) {
    pl->add_param("frames", ParamStruct::optional, &FramesScript::ReadFrames, this, this);
    OverlayComp::GrowParamList(pl);
    return;
}

Component* FramesComp::Copy () {
    FramesComp* comps = new FramesComp(new Picture(GetGraphic()));
    if (attrlist()) comps->SetAttributeList(new AttributeList(attrlist()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comps;
}

ClassId FramesComp::GetClassId() { return FRAMES_COMP; }

boolean FramesComp::IsA(ClassId id) {
    return id == FRAMES_COMP || FrameComp::IsA(id);
}

void FramesComp::Interpret (Command* cmd) {
    if (cmd->IsA(DELETE_CMD) || cmd->IsA(CUT_CMD) || 
	cmd->IsA(DUP_CMD) || cmd->IsA(GROUP_CMD) || 
	cmd->IsA(UNGROUP_CMD) || cmd->IsA(FRONT_CMD) ||
	cmd->IsA(BACK_CMD)) 
	OverlaysComp::Interpret(cmd);
    else
	FrameComp::Interpret(cmd);
}

void FramesComp::Uninterpret (Command* cmd) {
    if (cmd->IsA(DELETE_CMD) || cmd->IsA(CUT_CMD) || 
	cmd->IsA(DUP_CMD) || cmd->IsA(GROUP_CMD) || 
	cmd->IsA(UNGROUP_CMD) || cmd->IsA(FRONT_CMD) ||
	cmd->IsA(BACK_CMD)) 
	OverlaysComp::Uninterpret(cmd);
    else
	FrameComp::Uninterpret(cmd);
}

/*****************************************************************************/

ParamList* FrameIdrawComp::_frame_idraw_params = nil;

FrameIdrawComp::FrameIdrawComp(boolean add_bg, const char* pathname, OverlayComp* parent) 
: FramesComp(parent) {
    _pathname = _basedir = nil;
    _gslist = nil;
    _ptsbuf = nil;
    SetPathName(pathname);
    if (add_bg)
        Append(new FrameComp());
}

FrameIdrawComp::FrameIdrawComp (istream& in, const char* pathname, OverlayComp* parent) 
: FramesComp(parent) {
    _pathname = _basedir = nil;
    _gslist = nil;
    _ptsbuf = nil;
    SetPathName(pathname);
    _valid = GetParamList()->read_args(in, this);
    delete _gslist;
    if (_ptsbuf) {
	for (int i=0; i<_ptsnum; i++) 
	    Unref(_ptsbuf[i]);
	delete _ptsbuf;
    }
}

FrameIdrawComp::FrameIdrawComp(OverlayComp* parent) : FramesComp(parent) {
}

FrameIdrawComp::~FrameIdrawComp () {
    delete _pathname;
    delete _basedir;
}

ParamList* FrameIdrawComp::GetParamList() {
    if (!_frame_idraw_params) 
	GrowParamList(_frame_idraw_params = new ParamList());
    return _frame_idraw_params;
}

void FrameIdrawComp::GrowParamList(ParamList* pl) {
    pl->add_param("grid", ParamStruct::keyword, &ParamList::read_float,
		  this, &_xincr, &_yincr);
    FramesComp::GrowParamList(pl); 
    return;
}

Component* FrameIdrawComp::Copy () {
    FrameIdrawComp* comps = new FrameIdrawComp(false, GetPathName());
    if (attrlist()) comps->SetAttributeList(new AttributeList(attrlist()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comps;
}

ClassId FrameIdrawComp::GetClassId() { return FRAME_IDRAW_COMP; }

boolean FrameIdrawComp::IsA(ClassId id) {
    return id == FRAME_IDRAW_COMP || FramesComp::IsA(id);
}

void FrameIdrawComp::Interpret (Command* cmd) {
    FrameEditor* ed = (FrameEditor*)cmd->GetEditor();


    FrameIdrawView *views = (FrameIdrawView*)ed->GetViewer()->GetGraphicView();

    if (cmd->IsA(PASTE_CMD) ||
	cmd->IsA(DELETE_CMD) ||
	cmd->IsA(CUT_CMD) ||
	cmd->IsA(DUP_CMD) ||
	cmd->IsA(GROUP_CMD) ||
	cmd->IsA(UNGROUP_CMD) ||
	cmd->IsA(FRONT_CMD) ||
	cmd->IsA(BACK_CMD)) {
	if (OverlaysView* frameview = ed->GetFrame()) 
	  frameview->GetGraphicComp()->Interpret(cmd);
	else
	  OverlaysComp::Interpret(cmd);

    }
    else if (cmd->IsA(CREATEFRAME_CMD)) {
	boolean after = ((CreateFrameCmd*)cmd)->After();
	Iterator frame;
	views->SetView(ed->GetFrame(), frame);
	int framenum = views->Index(frame);
	if (framenum >= 0) {
	    for (int i = 0; i <= framenum; i++) {
		if (i == 0)
		    First(frame);
		else
		    Next(frame);
	    }
	    if (after)
		InsertAfter(frame, new FrameComp());
	    else
		InsertBefore(frame, new FrameComp());

	    Notify();
	    unidraw->Update();
	    FrameListState* fliststate = ed->frameliststate();
	    Iterator last;
	    views->Last(last);
	    fliststate->framenumber(views->Index(last)+1);
	}
    }
    else if (cmd->IsA(DELETEFRAME_CMD)) {
        FrameListState* fls = ed->frameliststate();
	Iterator frame;
	views->SetView(ed->GetFrame(), frame);
	int framenum = views->Index(frame);
        int lastframe = fls->framenumber()-1;
        if (framenum > 0 && framenum != lastframe) {
            FrameNumberState* fns = ed->framenumstate();

            int framestatenum = fns->framenumber();
            Command *movecmd = new MoveFrameCmd(ed);
	    movecmd->Execute();
            GraphicComp* comp = ed->GetViewer()->GetGraphicView()->GetView(frame)->GetGraphicComp();
	    cmd->Store(this, new DeleteFrameData(comp, false));
	    Remove(comp);
	    fns->framenumber(framestatenum);

	    Notify();
	    unidraw->Update();
	    Iterator last;
	    views->Last(last);
            fls->framenumber(views->Index(last)+1);
        }
	else if (framenum > 0 && framenum == lastframe) {
	    FrameNumberState* fns = ed->framenumstate();
            int framestatenum = fns->framenumber();
            Command *movecmd = new MoveFrameCmd(ed, -1);
	    movecmd->Execute();
            GraphicComp* comp = ed->GetViewer()->GetGraphicView()->GetView(frame)->GetGraphicComp();
	    cmd->Store(this, new DeleteFrameData(comp, true));
	    Remove(comp);
	    fns->framenumber(framestatenum-1);

	    Notify();
	    unidraw->Update();
	    Iterator last;
	    views->Last(last);
            fls->framenumber(views->Index(last)+1);
        }
	else if (framenum == 0) {
            unidraw->GetWorld()->RingBell(1);
        }
    }
    else 
	FramesComp::Interpret(cmd);
}

void FrameIdrawComp::Uninterpret (Command* cmd) {
    FrameEditor* ed = (FrameEditor*)cmd->GetEditor();
    FrameListState* fls = ed->frameliststate();

    FrameIdrawView *views = (FrameIdrawView*)ed->GetViewer()->GetGraphicView();
    if (cmd->IsA(PASTE_CMD) ||
	cmd->IsA(DELETE_CMD) ||
	cmd->IsA(CUT_CMD) ||
	cmd->IsA(DUP_CMD) ||
	cmd->IsA(GROUP_CMD) ||
	cmd->IsA(UNGROUP_CMD) ||
	cmd->IsA(FRONT_CMD) ||
	cmd->IsA(BACK_CMD)) 
	if (OverlaysView* frameview = ed->GetFrame()) 
	  frameview->GetGraphicComp()->Uninterpret(cmd);
	else
	  OverlaysComp::Uninterpret(cmd);

    else if (cmd->IsA(CREATEFRAME_CMD)) {
	boolean after = ((CreateFrameCmd*)cmd)->After();
	Iterator frame;
	views->SetView(ed->GetFrame(), frame);
	int framenum = views->Index(frame);
	if (framenum >= 0) {
	    if (after)
		Next(frame);
	    else
		Prev(frame);
	    GraphicComp* comp = ed->GetViewer()->GetGraphicView()->GetView(frame)->GetGraphicComp();
	    Remove(comp);

	    Notify();
	    unidraw->Update();
	    Iterator last;
	    views->Last(last);
	    fls->framenumber(views->Index(last)+1);
	}
    } 
    else if (cmd->IsA(DELETEFRAME_CMD)) {
        Iterator frame;
	views->SetView(ed->GetFrame(), frame);
	DeleteFrameData* data = (DeleteFrameData*)cmd->Recall(this);
	int framenum = views->Index(frame);
        int lastframe = fls->framenumber()-1;
        if (data != nil) {
            GraphicComp* comp = (GraphicComp*)data->_void;
            for (int i = 0; i <= framenum; i++) {
		 if (i == 0)
		     First(frame);
		 else
		     Next(frame);
	    }
	    if (framenum == 0 || data->RestoreAfter()) {
		InsertAfter(frame, comp);
	        Notify();
	        Command *movecmd = new MoveFrameCmd(ed, +1);
	        movecmd->Execute();
            }
	    else {
		InsertBefore(frame, comp);
	        Notify();
	        Command *movecmd = new MoveFrameCmd(ed, -1);
	        movecmd->Execute();
	    }
	    unidraw->Update();
	    Iterator last;
	    views->Last(last);
	    fls->framenumber(views->Index(last)+1);
        }
    }
    else
	FramesComp::Uninterpret(cmd);
}

void FrameIdrawComp::SetPathName(const char* pathname) {
    delete _pathname;
    _pathname = pathname ? strdup(pathname) : nil;
    char * old_basedir = _basedir;
    _basedir = pathname ? strdup(pathname) : nil;
    if (_basedir) {
	char* last_slash = strrchr(_basedir, '/');
	if (last_slash)
	    *(last_slash+1) = '\0';
	else 
	    _basedir[0] = '\0';
    }
    if (old_basedir && _basedir && strcmp(old_basedir, _basedir) != 0) {
      AdjustBaseDir(old_basedir, _basedir);
      delete old_basedir;
    }
}

const char* FrameIdrawComp::GetPathName() { return _pathname; }

const char* FrameIdrawComp::GetBaseDir() { return _basedir; }

void FrameIdrawComp::GrowIndexedGS(Graphic* gs) {
    if (!_gslist) _gslist = new Picture();
    _gslist->Append(gs);
}

Graphic* FrameIdrawComp::GetIndexedGS(int index) {
    if (_gslist) {
	Iterator i;
	for (_gslist->First(i); !_gslist->Done(i); _gslist->Next(i)) {
	    if (index==0) return _gslist->GetGraphic(i);
	    index--;
	}
    }
    return nil;
}

void FrameIdrawComp::GrowIndexedPts(MultiLineObj* mlo) {
    if (!_ptsbuf) {
	_ptslen = 64;
	_ptsbuf = new MultiLineObj*[_ptslen];
	_ptsnum = 0;
	for (int i=0; i<_ptslen; i++) 
	    _ptsbuf[i] = nil;
    }
    if (_ptsnum==_ptslen) {
	MultiLineObj** newbuf = new MultiLineObj*[_ptslen*2];
	int i;
	for (i=0; i<_ptslen; i++) 
	    newbuf[i] = _ptsbuf[i];
	for (;i<_ptslen*2; i++)
	    newbuf[i] = nil;
	_ptslen *= 2;
	delete _ptsbuf;
	_ptsbuf = newbuf;
    }
    Resource::ref(mlo);
    _ptsbuf[_ptsnum++] = mlo;
}

MultiLineObj* FrameIdrawComp::GetIndexedPts(int index) {
    if (index >= 0  && index < _ptsnum) 
	return _ptsbuf[index];
    else
	return nil;
}
    
void FrameIdrawComp::GrowIndexedPic(OverlaysComp* pic) {
    if (!_picbuf) {
	_piclen = 64;
	_picbuf = new OverlaysComp*[_piclen];
	_picnum = 0;
	for (int i=0; i<_piclen; i++) 
	    _picbuf[i] = nil;
    }
    if (_picnum==_piclen) {
	OverlaysComp** newbuf = new OverlaysComp*[_piclen*2];
	int i;
	for (i=0; i<_piclen; i++) 
	    newbuf[i] = _picbuf[i];
	for (;i<_piclen*2; i++)
	    newbuf[i] = nil;
	_piclen *= 2;
	delete _picbuf;
	_picbuf = newbuf;
    }
    _picbuf[_picnum++] = pic;
}

OverlaysComp* FrameIdrawComp::GetIndexedPic(int index) {
    if (index >= 0  && index < _picnum) 
	return _picbuf[index];
    else
	return nil;
}
    
void FrameIdrawComp::ResetIndexedGS() {
  delete _gslist;
  _gslist = nil;
}

