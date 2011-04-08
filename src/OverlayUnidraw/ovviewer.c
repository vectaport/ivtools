/*
 * Copyright (c) 1994-1997 Vectaport Inc.
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
 * OverlayViewer implementation.
 */

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovdamage.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovunidraw.h>

#include <UniIdraw/idclasses.h>
#include <UniIdraw/ided.h>

#include <Unidraw/catalog.h>
#include <Unidraw/iterator.h>
#include <Unidraw/manips.h>
#include <Unidraw/statevars.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/Commands/command.h>
#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Tools/tool.h>

#include <InterViews/canvas.h>
#include <InterViews/cursor.h>
#include <InterViews/display.h>
#include <InterViews/perspective.h>
#include <InterViews/transformer.h>
#include <InterViews/window.h>
#include <IV-X11/xcanvas.h>
#include <IV-X11/xwindow.h>
#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/sensor.h>
#include <OS/math.h>
#include <iostream.h>

/*****************************************************************************/

Painter* OverlayViewer::xorPainter = nil;

/*****************************************************************************/

OverlayViewer::OverlayViewer (
    Editor* ed, GraphicView* gv, UPage* page, Grid* grid, 
    Coord w, Coord h, Orientation orientation,
    Alignment align, Zooming zoom
) : Viewer(ed, gv, page, grid, w, h, orientation) {
    delete _damage;
    _damage = new OverlayDamage;
    _damage->SetGraphic(_graphic);
    if (xorPainter == nil) {
        xorPainter = new Painter;
        Ref(xorPainter);
    }
    _needs_resize = true;
    SetColorMap();
    _pan_chain = _zoom_chain = _scribble_pointer = false;
}

OverlayViewer::~OverlayViewer () {}


void OverlayViewer::Update () {
    ((OverlayUnidraw*)unidraw)->CurrentViewer(this);
    if (_needs_resize)
      return;

    OverlaySelection* s = (OverlaySelection*)GetSelection();
    OverlayView* view = GetOverlayView();
    Component* viewComp = view->GetOverlayComp();
    OverlayComp* edComp = (OverlayComp*)_editor->GetComponent();

    if (viewComp != edComp) {
        GraphicView* newView = (GraphicView*)edComp->Create(ViewCategory());

        if (newView->IsA(GRAPHIC_VIEW)) {
            edComp->Attach(newView);
            newView->Update();
            SetGraphicView(newView);

        } else {
            delete newView;
        }

    } else {
	s->HideHandles(this);
        _viewerView->Update();
        GraphicBlock::UpdatePerspective();
	s->ShowHighlights();
	_damage->Repair();
        s->ShowHandles(this);
    }
    GetEditor()->GetWindow()->repair();
    GetEditor()->GetWindow()->display()->flush();
    GetEditor()->GetWindow()->cursor(arrow);
}

void OverlayViewer::Draw() 
{
    ((OverlayUnidraw*)unidraw)->CurrentViewer(this);
    OverlaySelection* s = (OverlaySelection*)GetSelection();

    _editor->GetWindow()->cursor(hourglass);
    StartBuffering();
    s->ShowHighlights(this);
    GraphicBlock::Draw();
    FinishBuffering(true);

    s->Init(this);
    s->ShowHandles(this);
    _editor->GetWindow()->cursor(arrow);

    _damage->Reset();
}

void OverlayViewer::Redraw(Coord x0, Coord y0, Coord x1, Coord y1) 
{
    ((OverlayUnidraw*)unidraw)->CurrentViewer(this);
    OverlaySelection* s = (OverlaySelection*)GetSelection();

    _editor->GetWindow()->cursor(hourglass);
    StartBuffering();
    s->ShowHighlights(this);
    GraphicBlock::Redraw(x0, y0, x1, y1);
    FinishBuffering(true);

    xorPainter->Clip(canvas, x0, y0, x1, y1);
    s->ShowHandles(this);
    xorPainter->NoClip();
    _editor->GetWindow()->cursor(arrow);
}

void OverlayViewer::PrepareDoubleBuf() {
    /* prepare for double buffering within viewer canvas */
    canvas->rep()->unbind();
    canvas->rep()->bind(true);
    canvas->rep()->clip_.x = 0;
    canvas->rep()->clip_.y = 0;
    canvas->rep()->clip_.width = (unsigned short)canvas->rep()->pwidth_;
    canvas->rep()->clip_.height = (unsigned short)canvas->rep()->pheight_;
    canvas->rep()->xdrawable_ = canvas->rep()->copybuffer_;
}

void OverlayViewer::Resize() 
{
    Viewer::Resize();
    PrepareDoubleBuf();

    /* initial drawing commands, if any */
    if (_needs_resize) {
      ((OverlayEditor*)_editor)->InformComponents();
      ((OverlayEditor*)_editor)->InitCommands();
      _needs_resize = false;
    }
}

void OverlayViewer::StartBuffering() 
{
    canvas->rep()->xdrawable_ = canvas->rep()->drawbuffer_;
}

void OverlayViewer::FinishBuffering(boolean refresh_needed) 
{
    if (refresh_needed)
	canvas->rep()->swapbuffers();
    canvas->rep()->xdrawable_ = canvas->rep()->copybuffer_;
}

void OverlayViewer::UseTool (Tool* t, Event& e) {
    Transformer* relative = ComputeGravityRel();
    Manipulator* m = t->CreateManipulator(this, e, relative);

    if (m != nil) {
        Manipulate(m, e);
        Command* cmd = t->InterpretManipulator(m);

        if (cmd != nil) {
#if 0
            cmd->Execute();
            if (cmd->Reversible()) {
                cmd->Log();
	    } else {
		delete cmd;
            }
#else
	    ((OverlayEditor*)GetEditor())->ExecuteCmd(cmd);
#endif

	    ((OverlaySelection*)GetSelection())->RepairClear(this, t->IsA(SELECT_TOOL));
        } else 
	    ((OverlaySelection*)GetSelection())->RepairClear(this, true);
        delete m;
    } else 
	((OverlaySelection*)GetSelection())->RepairClear(this, true);

    Unref(relative);
}

void OverlayViewer::Zoom (Perspective& np) {
    float factor = ScaleFactor(np);
    factor = LimitMagnification(GetMagnification() * factor)/GetMagnification();
    register Perspective* p = perspective;
    IntCoord halfw, halfh;
    halfw = p->curwidth/2;
    halfh = p->curheight/2;
    GetOverlayView()->AdjustForZoom(factor, halfw, halfh);
    Viewer::Zoom(np);
}

void OverlayViewer::Scroll (Perspective& np) {
    register Perspective* p = perspective;
    Coord dx, dy;
    dx = p->curx - np.curx;
    dy = p->cury - np.cury;
    if (dx==0 && dy==0) return;
    GetOverlayView()->AdjustForPan((float)dx, (float)dy);
    Viewer::Scroll(np);
}


OverlayView* OverlayViewer::GetOverlayView () { return (OverlayView*) _gview; }

void OverlayViewer::Chain(boolean pan, boolean zoom)  { 
    _pan_chain = pan ? true : _pan_chain;
    _zoom_chain = zoom ? true : _zoom_chain; 
}

void OverlayViewer::Unchain(boolean pan, boolean zoom)  { 
    _pan_chain = pan ? false : _pan_chain;
    _zoom_chain = zoom ? false : _zoom_chain; 
}

boolean OverlayViewer::Chained() { return _pan_chain || _zoom_chain; }
boolean OverlayViewer::ChainedPan() { return _pan_chain; }
boolean OverlayViewer::ChainedZoom() { return _zoom_chain; }

void OverlayViewer::Adjust (Perspective& np) {

    Editor* ed = GetEditor();
    Perspective basep = *GetPerspective();
    Viewer::Adjust(np);
    if (Chained()) {
	Iterator i;
	int dx = np.curx - basep.curx;
	int dy = np.cury - basep.cury;
	float xfactor = (float) np.curwidth / basep.curwidth;
	float yfactor = (float) np.curheight / basep.curheight;
	for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
	    OverlayViewer* v = (OverlayViewer*) unidraw->GetEditor(i)->GetViewer();
	    if (v->Chained() && v != this) {
		Perspective p = *v->GetPerspective();
		p.cury += dy * p.height / basep.height;
		p.curx += dx * p.width / basep.width;
		p.curwidth = (int) (xfactor * p.curwidth);
		p.curheight = (int) (yfactor * p.curheight);
		Perspective np = p;
		v->Normalize(np);
		if (np.curwidth != canvas->Width() || np.curheight!=canvas->Height()) {
		    if (ChainedZoom()) 
			v->Viewer::Adjust(p);
		} else {
		    if (ChainedPan())
			v->Viewer::Adjust(p);
		}
	    }
	}
    }
}


void OverlayViewer::SetColorMap() {
    Catalog* catalog = unidraw->GetCatalog();
    const char* col6 = catalog->GetAttribute("color6");
    const char* nocol6 = catalog->GetAttribute("nocolor6");
    const char* col5 = catalog->GetAttribute("color5");
    const char* gr7 = catalog->GetAttribute("gray7");
    const char* gr6 = catalog->GetAttribute("gray6");
    const char* gr5 = catalog->GetAttribute("gray5");
    boolean color6 =  col6 ? strcmp(col6 ? col6 : "", "true") == 0 : false;
    boolean nocolor6 = nocol6 ? strcmp(col6 ? col6 : "", "true") == 0 : false;
    boolean color5 = strcmp(col5 ? col5 : "", "true") == 0;
    boolean gray7 = strcmp(gr7 ? gr7 : "", "true") == 0;
    boolean gray6 = strcmp(gr6 ? gr6 : "", "true") == 0;
    boolean gray5 = strcmp(gr5 ? gr5 : "", "true") == 0;

    color6 = color6 && !nocolor6;

    if (color6 || color5) {

	if (color6) 
	    color5 = OverlayRaster::color_init(6) != 0;

	if (color5) 
	    OverlayRaster::color_init(5);
    }

    if (gray7 || gray6 || gray5) {

	if (gray7) 
	    gray6 = OverlayRaster::gray_init(7) != 0;

	if (gray6) 
	    gray5 = OverlayRaster::gray_init(6) != 0;

	if (gray5) 
	    OverlayRaster::gray_init(5);
    }

    return;
}

void OverlayViewer::Manipulate (Manipulator* m, Event& e) {
    Listen(allEvents);
    m->Grasp(e);

    /*
     * boolean b is just here to workaround a cfront 3.0 bug.
     */
    boolean b = false;
    GetCanvas()->window()->grab_pointer();
    do {
        Read(e);

	/* correct for motion events that arrive before the */
        /* GetCanvas()->window()->grab_pointer() takes affect */
        /* this also has the pleasant side effect of fixing a */
        /* bug in computing x,y location when the mouse rolls */
        /* off the canvas */
	if (e.type() == Event::motion && e.window() && 
	    e.window() != GetCanvas()->window()) {
	  WindowRep& ew = *e.window()->rep();
	  WindowRep& cw = *GetCanvas()->window()->rep();
	  e.x -=  cw.xpos_-ew.xpos_;
	  e.y +=  cw.ypos_-ew.ypos_;
	}

	b = m->Manipulating(e);
    } while (b);
    GetCanvas()->window()->ungrab_pointer();

    m->Effect(e);
    Listen(input);
}

void OverlayViewer::ScreenToDrawing(float xscreen, float yscreen, 
				    float& xdraw, float& ydraw) {
  Transformer* rel  = GetRel();
  rel->Invert();
  rel->Transform(xscreen, yscreen, xdraw, ydraw);
  rel->unref();
  return;
}

void OverlayViewer::ScreenToDrawing(Coord xscreen, Coord yscreen, 
				    float& xdraw, float& ydraw) {
  ScreenToDrawing(float(xscreen), float(yscreen), xdraw, ydraw);
}

void OverlayViewer::DrawingToScreen(float xdraw, float ydraw,
				    float& xscreen, float& yscreen) {
  Transformer* rel  = GetRel();
  float f_xscreen, f_yscreen;
  rel->Transform(xdraw, ydraw, xscreen, yscreen);
  rel->unref();
  return;
  
}

void OverlayViewer::DrawingToScreen(float xdraw, float ydraw,
				    Coord& xscreen, Coord& yscreen) {
  float fxscreen, fyscreen;
  DrawingToScreen(xdraw, ydraw, fxscreen, fyscreen);
//  xscreen = int(fxscreen);
//  yscreen = int(fyscreen);
  xscreen = Math::round(fxscreen);
  yscreen = Math::round(fyscreen);
  return;
  
}

void OverlayViewer::ScreenToGraphic
(float xscreen, float yscreen, Graphic* gr, float& xgr, float& ygr) {
  if (!gr) {
    xgr = xscreen;
    ygr = yscreen;
    return;
  }

  /* compute origin of graphic in drawing */
  /* typically the lower-left corner of screen when graphic originally */
  /* pasted, except for rasters, stencils, and text, where it is the */
  /* lower left corner of the graphic itself */
  float xorig_gr = 0.0, yorig_gr = 0.0;
  if (gr->GetTransformer())
    gr->GetTransformer()->Transform(0.,0., xorig_gr, yorig_gr);

  /* convert screen coordinates to drawing coordinates */
  float xdraw, ydraw;
  ScreenToDrawing(xscreen, yscreen, xdraw, ydraw);

  /* compute graphic relative coordinates */
  float xone_gr = 1.0, yone_gr = 1.0;
  if (gr->GetTransformer())
    gr->GetTransformer()->Transform(1.0, 1.0, xone_gr, yone_gr);
  float xscale = xone_gr-xorig_gr;
  float yscale = yone_gr-yorig_gr;
  xgr = (xdraw - xorig_gr)/xscale;
  ygr = (ydraw - yorig_gr)/yscale;
  return;
}

void OverlayViewer::ScreenToGraphic
(Coord xscreen, Coord yscreen, Graphic* gr, float& xgr, float& ygr) {
  ScreenToGraphic(float(xscreen), float(yscreen), gr, xgr, ygr);
}

void OverlayViewer::GraphicToScreen
(Graphic* gr, float xgr, float ygr, float& xscreen, float& yscreen) 
{
  if (!gr) {
    xscreen = Math::round(xgr);
    yscreen = Math::round(ygr);
    return;
  }

  /* convert graphic coordinates to drawing coordinates */
  float xdrawgr, ydrawgr;
  if (gr->GetTransformer())
    gr->GetTransformer()->Transform(xgr, ygr, xdrawgr, ydrawgr);

  /* convert drawing coordinates to screen coordinates */
  DrawingToScreen(xdrawgr, ydrawgr, xscreen, yscreen);
  return;
}

void OverlayViewer::GraphicToScreen
(Graphic* gr, float xgr, float ygr, int& xscreen, int& yscreen) 
{
  float fxscreen, fyscreen;
  GraphicToScreen(gr, xgr, ygr, fxscreen, fyscreen);
  xscreen = int(fxscreen);
  yscreen = int(fyscreen);
}

void OverlayViewer::CenterToScreen(int sx, int sy) {
  Perspective* p = GetPerspective();
  p->curx = sx - p->curwidth/2;
  p->cury = sy - p->curheight/2;
  Adjust(*p);
}

void OverlayViewer::SetMagnification (float newmag) {
  newmag = LimitMagnification(newmag);
  float oldmag = GetMagnification();
  float factor = newmag/oldmag;
  
  GraphicView* topview = GetGraphicView();
  if (topview) {
    register Perspective* p = perspective;
    IntCoord halfw, halfh;
    halfw = p->curwidth/2;
    halfh = p->curheight/2;
    GetOverlayView()->AdjustForZoom(factor, halfw, halfh);
  }
  Viewer::SetMagnification(newmag);
}    

