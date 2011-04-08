/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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
 * OverlayIdrawComp implementation.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/paramlist.h>
#include <OverlayUnidraw/scriptview.h>

#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/unidraw.h>

#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/struct.h>
#include <Unidraw/Commands/transforms.h>

#include <Unidraw/Graphic/picture.h>

#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

extern "C" 
{
  char* realpath();
}

/*****************************************************************************/

static OverlayComp* Pred (OverlayComp* child) {
    Iterator i;
    OverlayComp* parent = (OverlayComp*) child->GetParent();

    if (parent) {
      parent->SetComp(child, i);
      parent->Prev(i);
      return (OverlayComp*) parent->GetComp(i);
    } else
      return nil;
}

static void NullGS (Graphic* g) { FullGraphic null; *g = null; }

/*****************************************************************************/

ParamList* OverlayComp::_overlay_comp_params = nil;

OverlayComp::OverlayComp (Graphic* g, OverlayComp* parent) : GraphicComp(g)
{
    _valid = true;
    _parent = parent;
    _anno = nil;
    _attrlist = nil;
}

OverlayComp::OverlayComp (istream& in) { 
    _gr = new FullGraphic();
    _parent = nil;
    _anno = nil;
    _attrlist = nil;
    _valid = GetParamList()->read_args(in, this);
}

OverlayComp::~OverlayComp () {
    if (_anno)
	delete [] _anno;
    Unref(_attrlist);
}

AttributeList* OverlayComp::GetAttributeList() {
    if (!_attrlist) {
        _attrlist = new AttributeList;
	Resource::ref(_attrlist);
    }
    return _attrlist;
}

void OverlayComp::SetAttributeList(AttributeList* al) {
    Unref(_attrlist);
    _attrlist = al;
    Resource::ref(_attrlist);
}

const char* OverlayComp::GetAnnotation() {
    return _anno;
}

void OverlayComp::SetAnnotation(const char* an) {
    if (_anno) {
	delete [] _anno;
	_anno = nil;
    }
    if (an) {
	_anno = strdup(an);
    }
}

ClassId OverlayComp::GetClassId () { return OVERLAY_COMP; }

boolean OverlayComp::IsA (ClassId id) {
    return OVERLAY_COMP == id || GraphicComp::IsA(id);
}

ParamList* OverlayComp::GetParamList() {
    if (!_overlay_comp_params) 
	GrowParamList(_overlay_comp_params = new ParamList());
    return _overlay_comp_params;
}

void OverlayComp::GrowParamList(ParamList* pl) {
    pl->add_param("other", ParamStruct::other,        &OverlayScript::ReadOther,
	this, this);
    pl->add_param("gs", ParamStruct::keyword,         &OverlayScript::ReadGS,
	this, this, &_gr);
    pl->add_param("fillbg", ParamStruct::keyword,     &OverlayScript::ReadFillBg,
	this, &_gr);
    pl->add_param("nonebr", ParamStruct::keyword,     &OverlayScript::ReadNoneBr,
	this, &_gr);
    pl->add_param("brush", ParamStruct::keyword,      &OverlayScript::ReadBrush,
	this, &_gr);
    pl->add_param("fgcolor", ParamStruct::keyword,    &OverlayScript::ReadFgColor,
	this, &_gr);
    pl->add_param("bgcolor", ParamStruct::keyword,    &OverlayScript::ReadBgColor,
	this, &_gr);
    pl->add_param("font", ParamStruct::keyword,       &OverlayScript::ReadFont,
	this, &_gr);
    pl->add_param("nonepat", ParamStruct::keyword,    &OverlayScript::ReadNonePat,
	this, &_gr);
    pl->add_param("pattern", ParamStruct::keyword,    &OverlayScript::ReadPattern,
	this, &_gr);
    pl->add_param("graypat", ParamStruct::keyword,    &OverlayScript::ReadGrayPat,
	this, &_gr);
    pl->add_param("transform", ParamStruct::keyword,  &OverlayScript::ReadTransform,
	this, &_gr);
    pl->add_param("annotation", ParamStruct::keyword, &OverlayScript::ReadAnnotation,
	this, &_anno);
    return;
}

OverlayView* OverlayComp::FindView (Viewer* viewer) {
    for (UList* u = _views->First(); u != _views->End(); u = u->Next()) {
	ComponentView* compview = View(u);
	if (compview->IsA(OVERLAY_VIEW) && 
	    ((OverlayView*)compview)->GetViewer() == viewer) 
	    return (OverlayView*)compview;
    }
    return nil;
}

const char* OverlayComp::GetBaseDir() { return _parent ? _parent->GetBaseDir() : nil; }

void OverlayComp::Interpret(Command* cmd) {
    Editor* ed = cmd->GetEditor();

    if (cmd->IsA(UNHIDE_VIEWS_CMD) || cmd->IsA(SENSITIZE_VIEWS_CMD)) {
	for (UList* u = _views->First(); u != _views->End(); u = u->Next()) {
	    ComponentView* compview = View(u);
	    if (compview->IsA(OVERLAY_VIEW)) {
		((OverlayView*)compview)->Sensitize();
		if (cmd->IsA(UNHIDE_VIEWS_CMD))
		  ((OverlayView*)compview)->Show();
	    }
	}
	Notify();
	
    } else {
      GraphicComp::Interpret(cmd);
    }
}

void OverlayComp::Uninterpret(Command* cmd) {
    Editor* ed = cmd->GetEditor();

    if (
        (cmd->IsA(UNHIDE_VIEWS_CMD) || cmd->IsA(SENSITIZE_VIEWS_CMD))
     ) {
	Iterator i;
	
	    
    } else {
	GraphicComp::Uninterpret(cmd);
    }
}

Graphic* OverlayComp::GetIndexedGS(int index) {
    return _parent ? _parent->GetIndexedGS(index) : nil;
}

MultiLineObj* OverlayComp::GetIndexedPts(int index) {
    return _parent ? _parent->GetIndexedPts(index) : nil;
}

OverlaysComp* OverlayComp::GetIndexedPic(int index) {
    return _parent ? _parent->GetIndexedPic(index) : nil;
}

void OverlayComp::SetPathName(const char* pathname) {}
const char* OverlayComp::GetPathName() { return nil;}

Component* OverlayComp::GetParent () { 
    return _parent ? _parent : GraphicComp::GetParent();
}

void OverlayComp::SetParent (Component* child, Component* parent) {
    ((OverlayComp*)child)->_parent = (OverlayComp*)parent;
}

boolean OverlayComp::GraphicEquals(Graphic* g1, Graphic* g2) {
    return
	g1->GetBrush() == g2->GetBrush() &&
	g1->GetFgColor() == g2->GetFgColor() &&
	g1->GetBgColor() == g2->GetBgColor() &&
	g1->GetFont() == g2->GetFont() &&
	g1->GetPattern() == g2->GetPattern() &&
	g1->BgFilled() == g2->BgFilled() && 
	g1->GetTransformer() == g2->GetTransformer();
}

boolean OverlayComp::operator == (OverlayComp& comp) {
    Graphic* gr = GetGraphic();
    Graphic* test = comp.GetGraphic();
    return 
	GetClassId() == comp.GetClassId() &&
	GraphicEquals(gr, test);
}

boolean OverlayComp::operator != (OverlayComp& comp) {
    return !(*this==comp);
}

boolean OverlayComp::GetByPathnameFlag() {
    return true;
}

void OverlayComp::SetByPathnameFlag(boolean flag) {
}

void OverlayComp::Configure(Editor*) { }

OverlayComp* OverlayComp::TopComp() {
  OverlayComp* comp = this;
  OverlayComp* parent = (OverlayComp*)comp->GetParent();
  while (parent)
    comp = (OverlayComp*) comp->GetParent();
  return comp;
}


void OverlayComp::AdjustBaseDir(const char* olddir, const char* newdir) {
  const char* old_path = GetPathName();
  if (old_path) {
    if (*old_path != '/') {
      if ((!olddir || !*olddir) && strcmp(newdir, "./") != 0) {
	char new_path[MAXPATHLEN];
	if (realpath(old_path, new_path))
	  SetPathName(new_path);
      } else if (olddir) {
	char whole_path[MAXPATHLEN];
	strcpy(whole_path, olddir);
	strcat(whole_path, "/");
	strcat(whole_path, old_path);
	char new_path[MAXPATHLEN];
	if (realpath(whole_path, new_path))
	  SetPathName(new_path);
      }
    }
  }
}

void OverlayComp::update(Observable* obs) {
  Notify();
}

void OverlayComp::Notify() {
  Observable::notify();
  GraphicComp::Notify();
}

AttributeValue* OverlayComp::FindValue
(const char* name, boolean last, boolean breadth, boolean down, boolean up) {
  int symid = symbol_find((char*)name);
  if (symid >= 0)
    return FindValue(symid, last, breadth, down, up);
  else
    return nil;
}

AttributeValue* OverlayComp::FindValue
(int symid, boolean last, boolean breadth, boolean down, boolean up) {
  if (breadth) {
    cerr << "breadth search not yet unsupported\n";
    return nil;
  } else if (up) {
    cerr << "upward search not yet unsupported\n";
    return nil;
  } else if (last) {
    cerr << "search for last value not yet unsupported\n";
    return nil;
  } else if (AttributeList* al = attrlist()) {
    return  al->find(symid);
  } else
    return nil;
}

/*****************************************************************************/

ParamList* OverlaysComp::_overlay_comps_params = nil;

OverlaysComp::OverlaysComp (OverlayComp* parent) : OverlayComp(new Picture, parent) { 
    _comps = new UList;
}

OverlaysComp::OverlaysComp (Graphic* g, OverlayComp* parent) : OverlayComp(g, parent) { 
    _comps = new UList;
}

OverlaysComp::OverlaysComp (istream& in, OverlayComp* parent) : OverlayComp(new Picture, parent) { 
    _comps = new UList;
    _valid = GetParamList()->read_args(in, this);
}

ParamList* OverlaysComp::GetParamList() {
    if (!_overlay_comps_params) 
	GrowParamList(_overlay_comps_params = new ParamList());
    return _overlay_comps_params;
}

void OverlaysComp::GrowParamList(ParamList* pl) {
    pl->add_param("kids", ParamStruct::optional, &OverlaysScript::ReadChildren, this, this);
    pl->add_param("pic", ParamStruct::keyword, &OverlaysScript::ReadPic,
		  this, this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

Component* OverlaysComp::Copy () {
    OverlaysComp* comps = new OverlaysComp(new Picture(GetGraphic()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comps;
}

ClassId OverlaysComp::GetClassId () { return OVERLAYS_COMP; }

boolean OverlaysComp::IsA (ClassId id) {
    return OVERLAYS_COMP == id || OverlayComp::IsA(id);
}

OverlaysComp::~OverlaysComp () {
    Iterator i;
    OverlayComp* comp;

    First(i);
    while (!Done(i)) {
        comp = (OverlayComp*) GetComp(i);
        Remove(i);
        delete comp;
    }
    delete _comps;
}

void OverlaysComp::Interpret (Command* cmd) {
    Editor* ed = cmd->GetEditor();

    if (
        (cmd->IsA(DELETE_CMD) || cmd->IsA(CUT_CMD)) && 
        ed->GetComponent() != this
    ) {
        Iterator i;
        for (First(i); !Done(i); Next(i)) {
            GetComp(i)->Interpret(cmd);
        }

    } else if (cmd->IsA(DELETE_CMD)) {
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
            OverlayComp* comp = (OverlayComp*)cb->GetComp(i);
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
            GraphicView* views = ed->GetViewer()->GetGraphicView();
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

    } else if (cmd->IsA(PASTE_CMD)) {
        Clipboard* cb = cmd->GetClipboard();
        Iterator i;

        if (cb == nil) {
            Clipboard* globalcb = unidraw->GetCatalog()->GetClipboard();

            if (globalcb->IsEmpty()) {
                return;
            }
            cmd->SetClipboard(cb = globalcb->DeepCopy());
        }

        for (cb->First(i); !cb->Done(i); cb->Next(i)) {
            Append((OverlayComp*)cb->GetComp(i));
        }
        Notify();
        SelectClipboard(cb, ed);
        unidraw->Update();

    } else if (cmd->IsA(DUP_CMD)) {
        GraphicView* views = ed->GetViewer()->GetGraphicView();
        OverlayComp* prev, *dup1;
        Iterator i, pos;
        Clipboard* cb = cmd->GetClipboard();
        const float offset = 8;
        MoveCmd move(ed, offset, offset);

        if (cb == nil) {
            OverlaySelection* s = (OverlaySelection*)ed->GetSelection();

            if (s->IsEmpty()) {
                return; 
            }
            cmd->SetClipboard(cb = new Clipboard);
            s->Sort(views);

            for (s->First(i); !s->Done(i); s->Next(i)) {
                dup1 = (OverlayComp*) s->GetView(i)->GetOverlayComp()->Copy();
                dup1->Interpret(&move);
                cb->Append(dup1);
            }
            cb->First(i);
            dup1 = (OverlayComp*) cb->GetComp(i);
            Last(pos);
            prev = (OverlayComp*) GetComp(pos);
            cmd->Store(dup1, new VoidData(prev));

        } else {
            cb->First(i);
            dup1 = (OverlayComp*) cb->GetComp(i);
            VoidData* vd = (VoidData*) cmd->Recall(dup1);
            prev = (OverlayComp*) vd->_void;
            SetComp(prev, pos);
        }

        for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
            InsertAfter(pos, cb->GetComp(i));
        }

        Notify();
        SelectClipboard(cb, ed);
        unidraw->Update();

    } else if (cmd->IsA(OVGROUP_CMD)) {
        OvGroupCmd* gcmd = (OvGroupCmd*) cmd;
        OverlayComp* group = gcmd->GetGroup();
        Component* edComp = gcmd->GetEditor()->GetComponent();

        if (group == this) {
            edComp->Interpret(gcmd);

        } else if (edComp == (Component*) this) {
            Clipboard* cb = cmd->GetClipboard();
            NullGS(group->GetGraphic());
            Group(cb, group, cmd);
            Notify();
            SelectViewsOf(group, ed);
            unidraw->Update();

        } else {
            OverlayComp::Interpret(gcmd);
        }

    } else if (cmd->IsA(UNGROUP_CMD)) {
        UngroupCmd* ucmd = (UngroupCmd*) cmd;
        Component* edComp = ucmd->GetEditor()->GetComponent();

        if (edComp == (Component*) this) {
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

        } else {
            cmd->GetClipboard()->Append(this);
        }

    } else if (cmd->IsA(FRONT_CMD) || cmd->IsA(BACK_CMD)) {
        Component* edComp = cmd->GetEditor()->GetComponent();

        if (edComp == (Component*) this) {
            Clipboard* cb = cmd->GetClipboard();
            Iterator i;

            if (cmd->IsA(FRONT_CMD)) {
                for (cb->First(i); !cb->Done(i); cb->Next(i)) {
                    OverlayComp* comp = (OverlayComp*)cb->GetComp(i);
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
            OverlayComp::Interpret(cmd);
        }

    } else {
        OverlayComp::Interpret(cmd);
    }
}

void OverlaysComp::Uninterpret (Command* cmd) {
    Editor* ed = cmd->GetEditor();

    if (
        (cmd->IsA(DELETE_CMD) || cmd->IsA(CUT_CMD)) && 
        ed->GetComponent() != this
    ) {
        Iterator i;
        for (Last(i); !Done(i); Prev(i)) {
            GetComp(i)->Uninterpret(cmd);
        }

    } else if (cmd->IsA(DELETE_CMD)) {
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

    } else if (cmd->IsA(PASTE_CMD)) {
        Clipboard* cb = cmd->GetClipboard();

        if (cb != nil) {
            Selection* s = ed->GetSelection();
            Iterator i, pos;

            s->Clear();

            for (cb->First(i); !cb->Done(i); cb->Next(i)) {
                GraphicComp* comp = cb->GetComp(i);
                unidraw->CloseDependents(comp);
                Remove(comp);
            }
            Notify();
            unidraw->Update();
        }

    } else if (cmd->IsA(DUP_CMD)) {
        Clipboard* cb = cmd->GetClipboard();

        if (cb != nil) {
            Selection* s = ed->GetSelection();
            Iterator i;

            s->Clear();

            for (cb->First(i); !cb->Done(i); cb->Next(i)) {
                GraphicComp* comp = cb->GetComp(i);
                unidraw->CloseDependents(comp);
                Remove(comp);
            }
            Notify();
            unidraw->Update();
        }

    } else if (cmd->IsA(OVGROUP_CMD)) {
        OvGroupCmd* gcmd = (OvGroupCmd*) cmd;
        OverlayComp* group = gcmd->GetGroup();
        Component* edComp = gcmd->GetEditor()->GetComponent();

        if (group == this) {
            edComp->Uninterpret(gcmd);

        } else if (edComp == (Component*) this) {
            Clipboard* cb = cmd->GetClipboard();
            Iterator i;
            cb->First(i);
            OverlayComp* group = (OverlayComp*) cb->GetComp(i)->GetParent();

            GroupCmd* gcmd = (GroupCmd*) cmd;
            unidraw->CloseDependents(group);

            for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
                RestorePosition((OverlayComp*)cb->GetComp(i), cmd);
            }
            Remove(group);
            Notify();
            SelectClipboard(cb, ed);
            unidraw->Update();

        } else {
            OverlayComp::Uninterpret(gcmd);
        }

    } else if (cmd->IsA(UNGROUP_CMD)) {
        UngroupCmd* ucmd = (UngroupCmd*) cmd;
        Component* edComp = ucmd->GetEditor()->GetComponent();

        if (edComp == (Component*) this) {
            Clipboard* cb = ucmd->GetClipboard();
            Clipboard* kids = ucmd->GetKids();
            Clipboard insertedParents;
            Iterator k;

            for (kids->First(k); !kids->Done(k); kids->Next(k)) {
                OverlayComp* kid = (OverlayComp*) kids->GetComp(k);
                UngroupData* ud = (UngroupData*) cmd->Recall(kid);
                OverlayComp* parent = (OverlayComp*)ud->_parent;
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
        }

    } else if (cmd->IsA(FRONT_CMD)) {
        Component* edComp = cmd->GetEditor()->GetComponent();

        if (edComp == (Component*) this) {
            Clipboard* cb = cmd->GetClipboard();
            Iterator i;

            for (cb->Last(i); !cb->Done(i); cb->Prev(i)) {
                RestorePosition((OverlayComp*)cb->GetComp(i), cmd);
            }
            Notify();
            SelectClipboard(cb, ed);
            unidraw->Update();

        } else {
            OverlayComp::Uninterpret(cmd);
        }

    } else if (cmd->IsA(BACK_CMD)) {
        Component* edComp = cmd->GetEditor()->GetComponent();

        if (edComp == (Component*) this) {
            Clipboard* cb = cmd->GetClipboard();
            Iterator i;

            for (cb->First(i); !cb->Done(i); cb->Next(i)) {
                RestorePosition((OverlayComp*)cb->GetComp(i), cmd);
            }
            Notify();
            SelectClipboard(cb, ed);
            unidraw->Update();

        } else {
            OverlayComp::Uninterpret(cmd);
        }

    } else {
        OverlayComp::Uninterpret(cmd);
    }
}

UList* OverlaysComp::Elem (Iterator i) { return (UList*) i.GetValue(); }
void OverlaysComp::First (Iterator& i) { i.SetValue(_comps->First()); }
void OverlaysComp::Last (Iterator& i) { i.SetValue(_comps->Last()); }
void OverlaysComp::Next (Iterator& i) { i.SetValue(Elem(i)->Next()); }
void OverlaysComp::Prev (Iterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean OverlaysComp::Done (Iterator i) { return Elem(i) == _comps->End(); }
OverlayComp* OverlaysComp::Comp (UList* r) { return (OverlayComp*) (*r)(); }
GraphicComp* OverlaysComp::GetComp (Iterator i) { return Comp(Elem(i)); }

void OverlaysComp::SetComp (GraphicComp* gc, Iterator& i) {
    i.SetValue(_comps->Find(gc));
}
void OverlaysComp::Append (GraphicComp* comp) {
    Graphic* g = comp->GetGraphic();

    _comps->Append(new UList(comp));
    if (g != nil) GetGraphic()->Append(g);

    SetParent(comp, this);
}

void OverlaysComp::Prepend (GraphicComp* comp) {
    Graphic* g = comp->GetGraphic();

    _comps->Prepend(new UList(comp));
    if (g != nil) GetGraphic()->Prepend(g);

    SetParent(comp, this);
}

void OverlaysComp::InsertBefore (Iterator i, GraphicComp* comp) {
    Graphic* g = comp->GetGraphic();
    Graphic* parent;

    Elem(i)->Append(new UList(comp));

    if (g != nil) {
        Iterator j;
        parent = GetGraphic();    
        parent->SetGraphic(GetComp(i)->GetGraphic(), j);
        parent->InsertBefore(j, g);
    }
    SetParent(comp, this);
}

void OverlaysComp::InsertAfter (Iterator i, GraphicComp* comp) {
    Graphic* g = comp->GetGraphic();
    Graphic* parent;
    
    Elem(i)->Prepend(new UList(comp));
    
    if (g != nil) {
        Iterator j;
        parent = GetGraphic();
        parent->SetGraphic(GetComp(i)->GetGraphic(), j);
        parent->InsertAfter(j, g);
    }
    SetParent(comp, this);
}

void OverlaysComp::Remove (Iterator& i) {
    UList* doomed = Elem(i);
    GraphicComp* comp = Comp(doomed);
    Graphic* g = comp->GetGraphic();

    Next(i);
    _comps->Remove(doomed);
    if (g != nil) GetGraphic()->Remove(g);

    SetParent(comp, nil);
    delete doomed;
}

void OverlaysComp::Remove (GraphicComp* comp) {
    Graphic* g = comp->GetGraphic();

    _comps->Delete(comp);
    if (g != nil) GetGraphic()->Remove(g);

    SetParent(comp, nil);
}

void OverlaysComp::Bequeath () { GetGraphic()->Bequeath(); }

void OverlaysComp::SetMobility (Mobility m) {
    Iterator i;

    for (First(i); !Done(i); Next(i)) {
        GetComp(i)->SetMobility(m);
    }
}

void OverlaysComp::SelectViewsOf (OverlayComp* comp, Editor* ed) {
    Selection* s = ed->GetSelection();
    s->Clear();
    Viewer* viewer;

    for (int i = 0; (viewer = ed->GetViewer(i)) != nil; ++i) {
        GraphicView* views = viewer->GetGraphicView();
        GraphicView* view = views->GetGraphicView(comp);

        if (view != nil) s->Append(view);
    }
}

void OverlaysComp::SelectClipboard (Clipboard* cb, Editor* ed) {
    Selection* s = ed->GetSelection();
    s->Clear();
    Viewer* viewer;
    Iterator i;

    for (int j = 0; (viewer = ed->GetViewer(j)) != nil; ++j) {
        for (cb->First(i); !cb->Done(i); cb->Next(i)) {
            GraphicView* views = viewer->GetGraphicView();
            GraphicView* view = views->GetGraphicView(cb->GetComp(i));

            if (view != nil) s->Append(view);
        }
    }
}

void OverlaysComp::StorePosition (OverlayComp* comp, Command* cmd) {
    cmd->Store(comp, new VoidData(Pred(comp)));
}

void OverlaysComp::RestorePosition (OverlayComp* comp, Command* cmd) {
    VoidData* vd = (VoidData*) cmd->Recall(comp);
    OverlayComp* pred = (OverlayComp*) vd->_void;
    OverlayComp* parent = (OverlayComp*) comp->GetParent();

    if (parent != nil) parent->Remove(comp);

    if (pred == nil) {
        Prepend(comp);

    } else {
        Iterator insertPt;
        SetComp(pred, insertPt);
        InsertAfter(insertPt, comp);
    }
}

void OverlaysComp::Group (Clipboard* cb, OverlayComp* group, Command* cmd) {
    Iterator insertPt, i;

    cb->Last(i);
    OverlayComp* last = (OverlayComp*) cb->GetComp(i);
    SetComp(last, insertPt);
    InsertAfter(insertPt, group);

    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
        OverlayComp* comp = (OverlayComp*) cb->GetComp(i);
        StorePosition(comp, cmd);
        Remove(comp);
        group->Append(comp);
    }
}

void OverlaysComp::Ungroup (OverlayComp* parent, Clipboard* cb, Command* cmd) {
    Iterator i, insertPt;
    parent->First(i);

    if (!parent->Done(i)) {
        SetComp(parent, insertPt);

        for (parent->First(i); !parent->Done(i); parent->Next(i)) {
            OverlayComp* kid = (OverlayComp*) parent->GetComp(i);
            cmd->Store(kid, new UngroupData(parent, kid->GetGraphic()));
        }

        cmd->Store(parent, new GSData(parent->GetGraphic()));
        parent->Bequeath();
        parent->First(i);

        do {
            OverlayComp* kid = (OverlayComp*)parent->GetComp(i);
            parent->Remove(i);
            InsertBefore(insertPt, kid);
            cb->Append(kid);
        } while (!parent->Done(i));

        Remove(parent);
    }
}

void OverlaysComp::GrowIndexedGS(Graphic* gs) {
  if (_parent)
    ((OverlaysComp*)_parent)->GrowIndexedGS(gs);
  else
    delete gs;
}
    
void OverlaysComp::GrowIndexedPts(MultiLineObj* mlo) {
  if (_parent)
    ((OverlaysComp*)_parent)->GrowIndexedPts(mlo);
  else
    delete mlo;
}
    
void OverlaysComp::GrowIndexedPic(OverlaysComp* pic) {
  if (_parent)
    ((OverlaysComp*)_parent)->GrowIndexedPic(pic);
  else 
    delete pic;
}

void OverlaysComp::ResetIndexedGS() {
  if (_parent) 
    ((OverlaysComp*)_parent)->ResetIndexedGS();
}
    
void OverlaysComp::ResetIndexedPts() {
  if (_parent)
    ((OverlaysComp*)_parent)->ResetIndexedPts();
}
    
void OverlaysComp::ResetIndexedPic() {
  if (_parent)
    ((OverlaysComp*)_parent)->ResetIndexedPic();
}

boolean OverlaysComp::SamePicture(OverlaysComp* comp) {
    Iterator i;
    Iterator j;

    for (First(i), comp->First(j); !Done(i) && !comp->Done(j); Next(i), comp->Next(j)) {
        if (GetComp(i) != comp->GetComp(j)) return false;
    }
    return Done(i) && comp->Done(j);
}
    
boolean OverlaysComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    return SamePicture(&(OverlaysComp&)comp) &&
	OverlayComp::operator==(comp);
}

void OverlaysComp::AdjustBaseDir(const char* olddir, const char* newdir) {
  if (olddir && newdir && strcmp(olddir, newdir) == 0) return;
  Iterator i;
  for (First(i); !Done(i); Next(i))
    ((OverlayComp*)GetComp(i))->AdjustBaseDir(olddir, newdir);
}

AttributeValue* OverlaysComp::FindValue
(int symid, boolean last, boolean breadth, boolean down, boolean up) {
  if (breadth) {
    cerr << "breadth search not yet unsupported\n";
    return nil;
  } else if (up) {
    cerr << "upward search not yet unsupported\n";
    return nil;
  } else if (last) {
    cerr << "search for last value not yet unsupported\n";
    return nil;
  } else {
    if (AttributeList* al = attrlist()) {
      AttributeValue* av = al->find(symid);
      if (av) return av;
    } 
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
      AttributeValue* av = ((OverlayComp*)GetComp(i))->FindValue(symid);
      if (av) return av;
    }
  }
  return nil;
}

/*****************************************************************************/

ParamList* OverlayIdrawComp::_overlay_idraw_params = nil;

OverlayIdrawComp::OverlayIdrawComp (const char* pathname, OverlayComp* parent)
: OverlaysComp(parent) {
    _pathname = _basedir = nil;
    SetPathName(pathname);
}

OverlayIdrawComp::OverlayIdrawComp (istream& in, const char* pathname, OverlayComp* parent) : 
OverlaysComp(parent) {
    _pathname = _basedir = nil;
    SetPathName(pathname);
    _valid = GetParamList()->read_args(in, this);
    ResetIndexedGS();
    ResetIndexedPts();
    ResetIndexedPic();
}

OverlayIdrawComp::~OverlayIdrawComp () {
    delete _pathname;
    delete _basedir;
}

ParamList* OverlayIdrawComp::GetParamList() {
    if (!_overlay_idraw_params) 
	GrowParamList(_overlay_idraw_params = new ParamList());
    return _overlay_idraw_params;
}

void OverlayIdrawComp::GrowParamList(ParamList* pl) {
    pl->add_param("grid", ParamStruct::keyword, &ParamList::read_float,
		  this, &_xincr, &_yincr);
    OverlaysComp::GrowParamList(pl); 
    return;
}
    
ClassId OverlayIdrawComp::GetClassId () { return OVERLAY_IDRAW_COMP; }

Component* OverlayIdrawComp::Copy () {
    OverlayIdrawComp* comps = new OverlayIdrawComp(GetPathName());
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comps;
}

boolean OverlayIdrawComp::IsA (ClassId id) {
    return OVERLAY_IDRAW_COMP == id || OverlaysComp::IsA(id);
}

void OverlayIdrawComp::SetPathName(const char* pathname) {
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
    if (_basedir) {
      AdjustBaseDir(old_basedir, _basedir);
      delete old_basedir;
    }
}

const char* OverlayIdrawComp::GetPathName() { return _pathname; }

const char* OverlayIdrawComp::GetBaseDir() { return _basedir; }

