/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1994 Vectaport Inc., Cartoactive Systems
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
 * OverlayView and OverlaysView definitions
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovfixview.h>
#include <OverlayUnidraw/ovmanips.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/grid.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Commands/align.h>
#include <Unidraw/Commands/edit.h>

#include <Unidraw/Components/grcomp.h>
#include <Unidraw/Components/gvupdater.h>

#include <Unidraw/Graphic/damage.h>
#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Graphic/picture.h>

#include <Unidraw/Tools/tool.h>

#include <InterViews/transformer.h>

#include <IV-2_6/InterViews/rubrect.h>

#include <stream.h>

/*****************************************************************************/

OverlayView::OverlayView(OverlayComp* subj) : GraphicView(subj) {
    _touched = false;
    UnfixSize();
    UnfixLocation();
    _hilite_gs = nil;
}

ClassId OverlayView::GetClassId () { return OVERLAY_VIEW; }

boolean OverlayView::IsA (ClassId id) {
    return OVERLAY_VIEW == id || GraphicView::IsA(id);
}

OverlayComp* OverlayView::GetOverlayComp () {
    return (OverlayComp*) GetSubject();
}    

void OverlayView::DrawHandles () {

    if (Highlightable()) {
	Highlight();
    } else 
	GraphicView::DrawHandles();
    _touched = true;
}

void OverlayView::RedrawHandles () {
    if (Highlightable()) {
	Highlight();
    } else 
	GraphicView::RedrawHandles();
    _touched = true;
}

void OverlayView::InitHandles () {
    if (!Highlightable()) 
	GraphicView::InitHandles();
}

void OverlayView::EraseHandles () {
    if (!_touched) return;

    if (Highlightable()) {
	Unhighlight();
    } else 
	GraphicView::EraseHandles();
}

boolean OverlayView::Highlightable () {
    return HighlightGraphic() != nil;
}

boolean OverlayView::Highlighted () {
    return HighlightGraphic() != nil && _touched;
}

Graphic* OverlayView::HighlightGraphic() {
    return _hilite_gs;
}

void OverlayView::HighlightGraphic(Graphic* hilite_gs) {
  delete _hilite_gs;
  _hilite_gs = new FullGraphic(hilite_gs);
}

void OverlayView::Highlight() {
    Graphic* hgr = HighlightGraphic();
    if (!hgr) return;
    Viewer* viewer = GetViewer();

    Graphic* gview = GetGraphic();
    if (viewer) viewer->GetDamage()->Incur(gview);
    gview->concatGS(gview,hgr,gview);
    if (viewer) viewer->GetDamage()->Incur(gview);
}

void OverlayView::Unhighlight() {
    Graphic* gview = GetGraphic();
    Graphic* gsubj = GetOverlayComp()->GetGraphic();
    Viewer* viewer = GetViewer();
    
    if (viewer) viewer->GetDamage()->Incur(gview);
    Transformer* t = gview->GetTransformer();
    Ref(t);
    gview->SetTransformer(nil);
    *gview = *gsubj;
    gview->SetTransformer(t);
    Unref(t);
    if (viewer) viewer->GetDamage()->Incur(gview);
}

boolean OverlayView::Hidable() { return true; }
boolean OverlayView::Hidden() { return _gr->Hidden(); }
void OverlayView::Hide() { _gr->Hide(); IncurDamage(_gr);}
void OverlayView::Show() { _gr->Show(); IncurDamage(_gr);}

boolean OverlayView::Desensitizable() { return true; }
boolean OverlayView::Desensitized() { return _gr->Desensitized(); }
void OverlayView::Desensitize() { _gr->Desensitize(); }
void OverlayView::Sensitize() { _gr->Sensitize(); }

Selection* OverlayView::MakeSelection() 
{ 
  return ((OverlayEditor*)GetViewer()->GetEditor())->overlay_kit()->MakeSelection(); 
}

void OverlayView::AdjustForZoom(float factor, Coord cx, Coord cy) {
    if (factor==1.0) return;

    if (!_fixed_size && !_fixed_location) return;

    int power2 = 0;
    float ftest = factor;
    if (ftest > 1.0) {
      while (ftest > 1.0) {
	power2++;
	ftest /= 2.0;
      }
    } else {
      while (ftest < 1.0) {
	power2++;
	ftest *= 2.0;
      }
    }

    Graphic* gr = GetGraphic();
    float gxc, gyc;
    gr->GetCenter(gxc, gyc);
    if (_fixed_size) {
      float actfact = 
	factor<1.0 ? .5/_fixed_size_factor : 2.0*_fixed_size_factor;
      for (int p=0; p<power2; p++) 
	gr->Scale(1.0/actfact,1.0/actfact,gxc,gyc);
	
    }
    if (_fixed_location) {
	Graphic *root, *parent = gr;
	do { 
	    root = parent;
	    parent = root->Parent();
	} while (parent != nil);

	root->Scale(factor, factor, cx, cy);
	float nxc, nyc;
	gr->GetCenter(nxc, nyc);
	float mag = GetViewer()->GetMagnification()*factor;
	gr->Translate((gxc-nxc)/mag, (gyc-nyc)/mag);
	root->Scale(1.0/factor, 1.0/factor, cx, cy);
    }
}

void OverlayView::AdjustForPan(float dx, float dy) {
    if (!dx && !dy) return;
    if (_fixed_location) {
	Graphic* gr = GetGraphic();
	float mag = GetViewer()->GetMagnification();
	gr->Translate(-dx/mag, -dy/mag);
    }
}

void OverlayView::FixSize (float factor) {
  _fixed_size = true; 
  _fixed_size_factor = factor;
}

void OverlayView::UnfixSize() { _fixed_size = false; }
void OverlayView::FixLocation() { _fixed_location = true; }
void OverlayView::UnfixLocation() { _fixed_location = false; }

OverlayView* OverlayView::View (UList* r) { return (OverlayView*) (*r)(); }

OverlayView* OverlayView::GetOverlayView (Graphic* g) {
    return (OverlayView*) g->GetTag();
}

void OverlayView::Interpret (Command* cmd) {
    if (cmd->IsA(HIDE_VIEW_CMD)) {
	Hide();
	Desensitize();
    } else if (cmd->IsA(DESENSITIZE_VIEW_CMD)) {
	Desensitize();
    } else if (cmd->IsA(FIX_VIEW_CMD)) {
	FixViewCmd* fv_cmd = (FixViewCmd*) cmd;
	if (fv_cmd->Size()) FixSize();
	if (fv_cmd->Location()) FixLocation();
    } else if (cmd->IsA(UNFIX_VIEW_CMD)) {
	FixViewCmd* fv_cmd = (FixViewCmd*) cmd;
	if (fv_cmd->Size()) UnfixSize();
	if (fv_cmd->Location()) UnfixLocation();
    } else {
        GraphicView::Interpret(cmd);
    }
}


void OverlayView::Uninterpret (Command* cmd) {
    if (cmd->IsA(HIDE_VIEW_CMD)) {
	Sensitize();
	Show();
    } else if (cmd->IsA(DESENSITIZE_VIEW_CMD)) {
	Sensitize();
    } else if (cmd->IsA(FIX_VIEW_CMD)) {
	FixViewCmd* fv_cmd = (FixViewCmd*) cmd;
	if (fv_cmd->Size()) UnfixSize();
	if (fv_cmd->Location()) UnfixLocation();
    } else if (cmd->IsA(UNFIX_VIEW_CMD)) {
	FixViewCmd* fv_cmd = (FixViewCmd*) cmd;
	if (fv_cmd->Size()) FixSize();
	if (fv_cmd->Location()) FixLocation();
   } else {
        GraphicView::Interpret(cmd);
    }
}

Manipulator* OverlayView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Rubberband* rub = nil;
    Manipulator* m = nil;
    IntCoord l, b, r, t;

    if (tool->IsA(MOVE_TOOL)) {
      if (!FixedLocation()) {
        v->Constrain(e.x, e.y);
        v->GetSelection()->GetBox(l, b, r, t);
        rub = new SlidingRect(nil, nil, l, b, r, t, e.x, e.y);
        m = new OpaqueDragManip(
  	    v, rub, rel, tool, 
	    DragConstraint(HorizOrVert | Gravity),
	    GetGraphic()
	    );
      }

    } else if (tool->IsA(SCALE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetGraphic()->GetBox(l, b, r, t);
        rub = new ScalingRect(nil, nil, l, b, r, t, (l+r)/2, (b+t)/2);
        m = new OpaqueDragManip(v, rub, rel, tool, Gravity, GetGraphic());

    } else if (tool->IsA(STRETCH_TOOL)) {
        m = CreateStretchManip(v, e, rel, tool);

    } else if (tool->IsA(ROTATE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetGraphic()->GetBox(l, b, r, t);
        rub = new RotatingRect(
            nil, nil, l, b, r, t, (l+r)/2, (b+t)/2, e.x, e.y
        );
        m = new OpaqueDragManip(v, rub, rel, tool, Gravity, GetGraphic());

    } else
        m = GraphicView::CreateManipulator(v, e, rel, tool);
    
    return m;
}

boolean OverlayView::FixedSize() { return _fixed_size; }
boolean OverlayView::FixedLocation() { return _fixed_location; }

Manipulator* OverlayView::CreateStretchManip (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Coord l, b, r, t, tmp;
    DragConstraint dc = HorizOrVert;

    v->Constrain(e.x, e.y);
    GetGraphic()->GetBox(l, b, r, t);
    boolean horizCtr = e.x > (2*l + r)/3 && e.x < (l + 2*r)/3;
    boolean vertCtr  = e.y > (2*b + t)/3 && e.y < (b + 2*t)/3;

    if (e.x < (l + r)/2) {
        tmp = r;
        r = l;
        l = tmp;
    }
    if (e.y < (b + t)/2) {
        tmp = t;
        t = b;
        b = tmp;
    }
    if (horizCtr && !vertCtr) {
        dc = XFixed;
    } else if (!horizCtr && vertCtr) {
        dc = YFixed;
    }

    RubberRect* rub = new RubberRect(nil, nil, l, b, r, t);
    return new OpaqueDragManip(
	v, rub, rel, tool, DragConstraint(dc | Gravity), r, t, GetGraphic()
    );
}

/*****************************************************************************/

OverlaysView::OverlaysView (OverlaysComp* subj) : OverlayView(subj) {
    _views = new UList;
}

ClassId OverlaysView::GetClassId () { return OVERLAYS_VIEW; }

boolean OverlaysView::IsA (ClassId id) {
    return OVERLAYS_VIEW == id || OverlayView::IsA(id);
}

OverlaysView::~OverlaysView () {
    Iterator i;
    Graphic* parent = GetGraphic();

    First(i);
    while (!Done(i)) {
        UList* doomed = Elem(i);
        OverlayView* view = (OverlayView*) GetView(i);
        Graphic* g = view->GetGraphic();

        Next(i);
        _views->Remove(doomed);
        parent->Remove(g);
        delete doomed;
        delete view;
    }
    delete _views;
    delete _hilite_gs;
}

static OverlayView* GetLeaf (OverlayView* ov) {
    Iterator i;
    ov->First(i);

    if (!ov->Done(i)) {
        ov = GetLeaf((OverlayView*) ov->GetView(i));
    }

    return ov;
}    

void OverlaysView::Interpret (Command* cmd) {
    if (cmd->IsA(ALIGNTOGRID_CMD)) {
        Viewer* viewer = GetViewer();
        Grid* grid = (viewer == nil) ? nil : viewer->GetGrid();

        if (grid == nil) {
            return;
        }

        AlignToGridCmd* acmd = (AlignToGridCmd*) cmd;
        OverlayView* leaf = GetLeaf(this);
        Graphic* leafg = leaf->GetGraphic();

        float cx, cy, dx, dy;
        leafg->GetCenter(cx, cy);
        leaf->Interpret(acmd);
        leafg->GetCenter(dx, dy);
        leaf->Uninterpret(acmd);
    
        dx -= cx;
        dy -= cy;

        Coord rcx = 0, rcy = 0;
        grid->Constrain(rcx, rcy);

        acmd->Align(this, float(rcx) - dx, float(rcy) - dy);

    } else {
        OverlayView::Interpret(cmd);
    }
}

void OverlaysView::Update () { GVUpdater gvu(this); gvu.Update(); }

UList* OverlaysView::Elem (Iterator i) { return (UList*) i.GetValue(); }
void OverlaysView::First (Iterator& i) { i.SetValue(_views->First()); }
void OverlaysView::Last (Iterator& i) { i.SetValue(_views->Last()); }
void OverlaysView::Next (Iterator& i) { i.SetValue(Elem(i)->Next()); }
void OverlaysView::Prev (Iterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean OverlaysView::Done (Iterator i) { return Elem(i) == _views->End(); }
GraphicView* OverlaysView::GetView (Iterator i) { return this->View(Elem(i)); }

void OverlaysView::SetView (GraphicView* gv, Iterator& i) {
    i.SetValue(_views->Find(gv));
}

int OverlaysView::Index(Iterator pos) {
    int idx = 0;
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	if (i.GetValue() == pos.GetValue())
	    return idx;
	idx++;
    }
    return -1;
}
  
Selection* OverlaysView::SelectAll () {
    Iterator i;
    Selection* selection = MakeSelection();
    
    for (First(i); !Done(i); Next(i)) {
	OverlayView* view = (OverlayView*) GetView(i);
	if (!view->Desensitized()) 
	    selection->Append(view);
    }
    return selection;
}

Selection* OverlaysView::ViewContaining (Coord x, Coord y) {
    Selection* s = MakeSelection();
    PointObj pt(x, y);
    Graphic* g = GetGraphic()->LastGraphicContaining(pt);

    if (g != nil) {
        OverlayView* ov = GetOverlayView(g);

        if (ov != nil) {
            s->Append(ov);
        }
    }
    return s;
}

Selection* OverlaysView::ViewsContaining (Coord x, Coord y) {
    Iterator i;
    Selection* s = MakeSelection();
    PointObj pt(x, y);

    for (First(i); !Done(i); Next(i)) {
        OverlayView* view = (OverlayView*) GetView(i);

        if (view->GetGraphic()->Contains(pt)) {
            s->Append(view);
        }
    }
    return s;
}

Selection* OverlaysView::ViewIntersecting (
    Coord x0, Coord y0, Coord x1, Coord y1
) {
    Selection* s = MakeSelection();
    BoxObj b(x0, y0, x1, y1);
    Graphic* g = GetGraphic()->LastGraphicIntersecting(b);

    if (g != nil) {
        OverlayView* ov = GetOverlayView(g);

        if (ov != nil) {
            s->Append(ov);
        }
    }
    return s;
}

Selection* OverlaysView::ViewsIntersecting (
    Coord x0, Coord y0, Coord x1, Coord y1
) {
    Iterator i;
    Selection* s = MakeSelection();
    BoxObj b(x0, y0, x1, y1);

    for (First(i); !Done(i); Next(i)) {
        OverlayView* view = (OverlayView*) GetView(i);

        if (view->GetGraphic()->Intersects(b)) {
            s->Append(view);
        }
    }
    return s;
}

Selection* OverlaysView::ViewsWithin (Coord x0, Coord y0, Coord x1, Coord y1){
    Iterator i;
    Selection* s = MakeSelection();
    BoxObj b(x0, y0, x1, y1);

    for (First(i); !Done(i); Next(i)) {
        OverlayView* view = (OverlayView*) GetView(i);
        BoxObj tmpbox;
	if (view->Desensitized()) continue;
        view->GetGraphic()->GetBox(tmpbox);

        if (tmpbox.Within(b)) {
            s->Append(view);
        }
    }
    return s;
}

Graphic* OverlaysView::GetGraphic () {
    Graphic* g = OverlayView::GetGraphic();
    
    if (g == nil) {
        OverlaysComp* comp = GetOverlaysComp();
	Graphic* compgr = comp->GetGraphic();
	g = new Picture;
	if (compgr && compgr->GetTransformer())
	  g->SetTransformer(new Transformer(compgr->GetTransformer()));

        Iterator i;

        for (First(i); !Done(i); Next(i)) {
            g->Append(GetView(i)->GetGraphic());
        }
        SetGraphic(g);
    }
    return g;
}

OverlaysComp* OverlaysView::GetOverlaysComp () {
    return (OverlaysComp*) GetSubject();
}    

void OverlaysView::Add (GraphicView* view) {
    Graphic* g = view->GetGraphic();
    Graphic* parent = GetGraphic();
    UList* rv = new UList(view);

    _views->Append(rv);
    parent->Append(g);
    SetParent(view, this);
}

void OverlaysView::Append (GraphicView* view) {
    Graphic* g = view->GetGraphic();
    Graphic* parent = GetGraphic();
    UList* rv = new UList(view);

    _views->Append(rv);
    parent->Append(g);
    SetParent(view, this);
}

void OverlaysView::InsertBefore (Iterator i, GraphicView* view) {
    Graphic* g = view->GetGraphic();
    Graphic* parent = GetGraphic();
    UList* r = Elem(i);
    UList* rv = new UList(view);

    r->Append(rv);

    if (r == _views->End()) {
        parent->Append(g);

    } else {
        Iterator j;
        parent->SetGraphic(this->View(r)->GetGraphic(), j);
        parent->InsertBefore(j, g);
    }
    SetParent(view, this);
}

void OverlaysView::Remove (Iterator& i) {
    UList* doomed = Elem(i);
    OverlayView* view = (OverlayView*) GetView(i);
    Graphic* g = view->GetGraphic();
    Graphic* parent = GetGraphic();

    Next(i);
    view->EraseHandles();
    _views->Remove(doomed);
    parent->Remove(g);
    SetParent(view, nil);
    delete doomed;
}

void OverlaysView::DeleteView (Iterator& i) {
    UList* doomed = Elem(i);
    OverlayView* view = (OverlayView*) GetView(i);
    Graphic* g = view->GetGraphic();
    Graphic* parent = GetGraphic();

    Next(i);
    IncurDamage(g);
    view->EraseHandles();
    _views->Remove(doomed);
    parent->Remove(g);
    delete doomed;
    delete view;
}

void OverlaysView::AdjustForZoom(float factor, Coord cx, Coord cy) {
    if (factor==1.0) return;

    Iterator i;

    for (First(i); !Done(i); Next(i)) {
        ((OverlayView*)GetView(i))->AdjustForZoom(factor, cx, cy);
    }
    OverlayView::AdjustForZoom(factor, cx, cy);
}

void OverlaysView::AdjustForPan(float dx, float dy) {
    if (!dx && !dy) return;

    Iterator i;

    for (First(i); !Done(i); Next(i)) {
        ((OverlayView*)GetView(i))->AdjustForPan(dx, dy);
    }
    OverlayView::AdjustForPan(dx, dy);
}

Manipulator* OverlaysView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Rubberband* rub = nil;
    Manipulator* m = nil;
    IntCoord l, b, r, t;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
      v->Constrain(e.x, e.y);
      m = new DragManip(v, nil, rel, tool, 
			      DragConstraint(XFixed | YFixed));
    } else
        m = OverlayView::CreateManipulator(v, e, rel, tool);
    
    return m;
}

Command* OverlaysView::InterpretManipulator(Manipulator* m) {
    Tool* tool = m->GetTool();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) 
    {
      DragManip* dm = (DragManip*) m;
      Editor* ed = dm->GetViewer()->GetEditor();
      Transformer* rel = dm->GetTransformer();

      Event initial = dm->GraspEvent();
      Coord x = initial.x;
      Coord y = initial.y;

      if (rel != nil) {
	rel = new Transformer(rel);
	rel->Invert();
      }
      /* get the comp */
      OverlaysComp* comp = (OverlaysComp*)GetSubject()->Copy();
      Transformer* t = comp->GetGraphic()->GetTransformer();
      if (!t) {
	t = new Transformer();
	comp->GetGraphic()->SetTransformer(t);
      }
      t->Translate(x, y);
      t->postmultiply(rel);
      Unref(rel);

      cmd = new PasteCmd(ed, new Clipboard(comp));

    } else 
    {
        cmd = OverlayView::InterpretManipulator(m);
    }
    return cmd;
}

/*****************************************************************************/

OverlayIdrawView::OverlayIdrawView (OverlayIdrawComp* subj) : OverlaysView(subj) { }

ClassId OverlayIdrawView::GetClassId () { return OVERLAY_IDRAW_VIEW; }

boolean OverlayIdrawView::IsA (ClassId id) {
    return OVERLAY_IDRAW_VIEW == id || OverlaysView::IsA(id);
}

