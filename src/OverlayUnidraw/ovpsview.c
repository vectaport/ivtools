/*
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
#include <OverlayUnidraw/ovexport.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/ovpsview.h>

#include <UniIdraw/idarrows.h>

#include <Unidraw/catalog.h>
#include <Unidraw/creator.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/unidraw.h>

#include <Unidraw/Graphic/graphic.h>

#include <InterViews/transformer.h>

#include <OS/math.h>

#include <stream.h>

/*****************************************************************************/

static const float GRID_XINCR = 8;                 // default grid spacing
static const float GRID_YINCR = 8;

static Transformer* SaveTransformer (Graphic* g) {
    Transformer* orig = g->GetTransformer();
    Ref(orig);
    g->SetTransformer(new Transformer(orig));
    return orig;
}

static void RestoreTransformer (Graphic* g, Transformer* orig) {
    g->SetTransformer(orig);
    Unref(orig);
}

// ScaleToPostscriptCoords scales the picture to Postscript
// coordinates if screen and Postscript inches are different.

static void ScaleToPostScriptCoords (Graphic* g) {
    const double ps_inch = 72.;

    if (ivinch != ps_inch) {
	double factor = ps_inch / ivinch;
	g->Scale(factor, factor);
    }
}

static boolean Uncollected (const char* name, UList* fonts) {
    for (UList* u = fonts->First(); u != fonts->End(); u = u->Next()) {
        PSFont* font = (PSFont*) (*u)();

        if (strcmp(font->GetPrintFont(), name) == 0) {
            return false;
        }
    }
    return true;
}

static void CollectFontsFromGraphic (Graphic* gr, UList* fonts) {
    PSFont* font = gr->GetFont();

    if (font != nil && Uncollected(font->GetPrintFont(), fonts)) {
        fonts->Append(new UList(font));
    }

    Iterator i;

    for (gr->First(i); !gr->Done(i); gr->Next(i)) {
        CollectFontsFromGraphic(gr->GetGraphic(i), fonts);
    }
}

/*****************************************************************************/

boolean OverlayPS::_idraw_format = true;

ClassId OverlayPS::GetClassId () { return OVERLAY_PS; }

boolean OverlayPS::IsA (ClassId id) {
    return OVERLAY_PS == id || PostScriptView::IsA(id);
}

OverlayPS::OverlayPS (OverlayComp* subj) : PostScriptView(subj) {
    _command = nil;
}

void OverlayPS::Creator (ostream& out) {
    out << "%%Creator: " << (idraw_format() ? "idraw" : "unidraw") << "\n";
}

UList* OverlayPS::GetPSFonts () {
    if (_fonts == nil) {
        _fonts = new UList;

        CollectFontsFromGraphic(GetGraphicComp()->GetGraphic(), _fonts);
    }
    return _fonts;
}

void OverlayPS::SetCommand(Command* command) {
    _command = command;
}

Command* OverlayPS::GetCommand() {
    return _command;
}

OverlayPS* OverlayPS::CreateOvPSView (GraphicComp* comp) {
    OverlayPS* ovpsv = (OverlayPS*) comp->Create(POSTSCRIPT_VIEW);

    if (ovpsv != nil) {
        comp->Attach(ovpsv);
	ovpsv->SetCommand(GetCommand());
        ovpsv->Update();
    }
    return ovpsv;
}

OverlayPS* OverlayPS::CreateOvPSViewFromGraphic (Graphic* graphic, boolean comptree) {
    ClassId compid = graphic->CompId();
    ClassId ovpsid = 0;

    if (compid == GRAPHIC_COMPS) {
	if (!comptree) 
	    ovpsid = PICTURE_PS;
	else
	    ovpsid = OVERLAYS_PS;
    }

    else if (compid == ARROWLINE_COMP)      ovpsid = ARROWLINE_PS;
    else if (compid == ARROWMULTILINE_COMP) ovpsid = ARROWMULTILINE_PS;
    else if (compid == ARROWSPLINE_COMP)    ovpsid = ARROWSPLINE_PS;
    else if (compid == CLOSEDSPLINE_COMP)   ovpsid = CLOSEDSPLINE_PS;
    else if (compid == ELLIPSE_COMP)        ovpsid = ELLIPSE_PS;
    else if (compid == LINE_COMP)           ovpsid = LINE_PS;
    else if (compid == MULTILINE_COMP)      ovpsid = MULTILINE_PS;
    else if (compid == POLYGON_COMP)        ovpsid = POLYGON_PS;
    else if (compid == RASTER_COMP)         ovpsid = RASTER_PS;
    else if (compid == RECT_COMP)           ovpsid = RECT_PS;
    else if (compid == SPLINE_COMP)         ovpsid = SPLINE_PS;
    else if (compid == STENCIL_COMP)        ovpsid = STENCIL_PS;
    else if (compid == TEXT_COMP)           ovpsid = TEXT_PS;

    OverlayPS* ovpsv = 
	(OverlayPS*) unidraw->GetCatalog()->GetCreator()->Create(ovpsid);

    if (ovpsv)
	ovpsv->SetCommand(GetCommand());

    return ovpsv;
}

OverlayComp* OverlayPS::GetOverlayComp () {
    return (OverlayComp*) GetSubject();
}

boolean OverlayPS::idraw_format() {
    boolean format = OverlayPS::_idraw_format;
    Command* cmd = GetCommand();
    if (cmd) {
      if (GetCommand()->IsA(OV_EXPORT_CMD))
	format = ((OvExportCmd*)GetCommand())->idraw_format();
      else if (GetCommand() && GetCommand()->IsA(OVPRINT_CMD)) 
	format = ((OvPrintCmd*)GetCommand())->idraw_format();
    }
    return format;
}

void OverlayPS::idraw_format(boolean flag) { _idraw_format = flag; }

/*****************************************************************************/

ClassId OverlaysPS::GetClassId () { return OVERLAYS_PS; }

boolean OverlaysPS::IsA (ClassId id) {
    return OVERLAYS_PS == id || OverlayPS::IsA(id);
}

OverlaysPS::OverlaysPS (OverlayComp* subj) : OverlayPS(subj) {
    _views = new UList;
}

OverlaysPS::~OverlaysPS () {
    DeleteViews();
    delete _views;
}

boolean OverlaysPS::Emit (ostream& out) {
    SetPSFonts();

    Graphic* g = GetGraphicComp()->GetGraphic();
    Transformer* t = SaveTransformer(g);
    ScaleToPostScriptCoords(g);

    Comments(out);
    Prologue(out);
    Version(out);
    GridSpacing(out);

    out << "\n\n%%Page: 1 1\n\n";
    out << "Begin\n";
    FullGS(out);
    out << "/originalCTM matrix currentmatrix def\n\n";

    boolean status = PreorderView::Definition(out);

    out << "End " << MARK << " eop\n\n";
    out << "showpage\n\n";

    Trailer(out);
    RestoreTransformer(g, t);

    return status;
}

boolean OverlaysPS::Definition (ostream& out) {
    out << "Begin " << MARK << " Pict\n";
    FullGS(out);
    out << "\n";

    boolean status = PreorderView::Definition(out);

    out << "End " << MARK << " eop\n\n";

    return status;
}    

void OverlaysPS::Update () {
    DeleteViews();

    GraphicComp* comps = GetGraphicComp();
    Iterator i;

    for (comps->First(i); !comps->Done(i); comps->Next(i)) {
        GraphicComp* comp = comps->GetComp(i);
        OverlayPS* ovpsv = CreateOvPSView(comp);
	if (!ovpsv) {
	    ovpsv = CreateOvPSViewFromGraphic(comp->GetGraphic());
	    if (ovpsv != nil) {
		comp->Attach(ovpsv);
		ovpsv->Update();
	    }
	}

        if (ovpsv != nil) 
            _views->Append(new UList(ovpsv));
    }
}

OverlaysComp* OverlaysPS::GetOverlaysComp () {
    return (OverlaysComp*) GetSubject();
}

UList* OverlaysPS::Elem (Iterator i) { return (UList*) i.GetValue(); }
void OverlaysPS::First (Iterator& i) { i.SetValue(_views->First()); }
void OverlaysPS::Last (Iterator& i) { i.SetValue(_views->Last()); }
void OverlaysPS::Next (Iterator& i) { i.SetValue(Elem(i)->Next()); }
void OverlaysPS::Prev (Iterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean OverlaysPS::Done (Iterator i) { return Elem(i) == _views->End(); }
ExternView* OverlaysPS::GetView (Iterator i) { return View(Elem(i)); }

void OverlaysPS::SetView (ExternView* ev, Iterator& i) {
    i.SetValue(_views->Find(ev));
}

void OverlaysPS::DeleteView (Iterator& i) {
    UList* doomed = Elem(i);
    ExternView* view = GetView(i);

    Next(i);
    _views->Remove(doomed);
    SetParent(view, nil);
    delete doomed;
    delete view;
}

void OverlaysPS::DeleteViews () {
    Iterator i;

    First(i);
    while (!Done(i)) {
        DeleteView(i);
    }
}    

/*****************************************************************************/

OverlayIdrawPS::OverlayIdrawPS (OverlayComp* subj) : OverlaysPS(subj) { }
ClassId OverlayIdrawPS::GetClassId () { return OVERLAY_IDRAW_PS; }

boolean OverlayIdrawPS::IsA (ClassId id) { 
    return OVERLAY_IDRAW_PS == id || OverlaysPS::IsA(id);
}

void OverlayIdrawPS::Prologue (ostream& out) {
    out << "%%BeginIdrawPrologue\n";
    ArrowHeader(out);
    out << "%%EndIdrawPrologue\n\n";

    OverlaysPS::Prologue(out);
}

void OverlayIdrawPS::MiscProcs (ostream& out) {
    PostScriptView::MiscProcs(out);
}

void OverlayIdrawPS::GridSpacing (ostream& out) {
    out << "Grid " << GRID_XINCR << " " << GRID_YINCR << " ";
}

void OverlayIdrawPS::ConstProcs (ostream& out) {
    int arrowWidth = Math::round(ARROWWIDTH*ivpoints);
    int arrowHeight = Math::round(ARROWHEIGHT*ivpoints);

    out << "/arrowHeight " << arrowHeight << " def\n";
    out << "/arrowWidth " << arrowWidth << " def\n\n";

    OverlaysPS::ConstProcs(out);
}

void OverlayIdrawPS::LineProc (ostream& out) {
    out << "/Line {\n";
    out << "0 begin\n";
    out << "2 storexyn\n";
    out << "newpath\n";
    out << "x 0 get y 0 get moveto\n";
    out << "x 1 get y 1 get lineto\n";
    out << "brushNone not { istroke } if\n";
    out << "0 0 1 1 leftarrow\n";
    out << "0 0 1 1 rightarrow\n";
    out << "end\n";
    out << "} dup 0 4 dict put def\n\n";
}

void OverlayIdrawPS::MultiLineProc (ostream& out) {
    out << "/MLine {\n";
    out << "0 begin\n";
    out << "storexyn\n";
    out << "newpath\n";
    out << "n 1 gt {\n";
    out << "x 0 get y 0 get moveto\n";
    out << "1 1 n 1 sub {\n";
    out << "/i exch def\n";
    out << "x i get y i get lineto\n";
    out << "} for\n";
    out << "patternNone not brushLeftArrow not brushRightArrow not and and ";
    out << "{ ifill } if\n";
    out << "brushNone not { istroke } if\n";
    out << "0 0 1 1 leftarrow\n";
    out << "n 2 sub dup n 1 sub dup rightarrow\n";
    out << "} if\n";
    out << "end\n";
    out << "} dup 0 4 dict put def\n\n";
}

void OverlayIdrawPS::BSplineProc (ostream& out) {
    out << "/BSpl {\n";
    out << "0 begin\n";
    out << "storexyn\n";
    out << "newpath\n";
    out << "n 1 gt {\n";
    out << "0 0 0 0 0 0 1 1 true subspline\n";
    out << "n 2 gt {\n";
    out << "0 0 0 0 1 1 2 2 false subspline\n";
    out << "1 1 n 3 sub {\n";
    out << "/i exch def\n";
    out << "i 1 sub dup i dup i 1 add dup i 2 add dup false subspline\n";
    out << "} for\n";
    out << "n 3 sub dup n 2 sub dup n 1 sub dup 2 copy false subspline\n";
    out << "} if\n";
    out << "n 2 sub dup n 1 sub dup 2 copy 2 copy false subspline\n";
    out << "patternNone not brushLeftArrow not brushRightArrow not and and ";
    out << "{ ifill } if\n";
    out << "brushNone not { istroke } if\n";
    out << "0 0 1 1 leftarrow\n";
    out << "n 2 sub dup n 1 sub dup rightarrow\n";
    out << "} if\n";
    out << "end\n";
    out << "} dup 0 4 dict put def\n\n";
}

void OverlayIdrawPS::SetBrushProc (ostream& out) {
    out << "/SetB {\n";
    out << "dup type /nulltype eq {\n";
    out << "pop\n";
    out << "false /brushRightArrow idef\n";
    out << "false /brushLeftArrow idef\n";
    out << "true /brushNone idef\n";
    out << "} {\n";
    out << "/brushDashOffset idef\n";
    out << "/brushDashArray idef\n";
    out << "0 ne /brushRightArrow idef\n";
    out << "0 ne /brushLeftArrow idef\n";
    out << "/brushWidth idef\n";
    out << "false /brushNone idef\n";
    out << "} ifelse\n";
    out << "} def\n\n";
}

void OverlayIdrawPS::ArrowHeader (ostream& out) {
    out << "/arrowhead {\n";
    out << "0 begin\n";
    out << "transform originalCTM itransform\n";
    out << "/taily exch def\n";
    out << "/tailx exch def\n";
    out << "transform originalCTM itransform\n";
    out << "/tipy exch def\n";
    out << "/tipx exch def\n";
    out << "/dy tipy taily sub def\n";
    out << "/dx tipx tailx sub def\n";
    out << "/angle dx 0 ne dy 0 ne or { dy dx atan } { 90 } ifelse def\n";
    out << "gsave\n";
    out << "originalCTM setmatrix\n";
    out << "tipx tipy translate\n";
    out << "angle rotate\n";
    out << "newpath\n";
    out << "arrowHeight neg arrowWidth 2 div moveto\n";
    out << "0 0 lineto\n";
    out << "arrowHeight neg arrowWidth 2 div neg lineto\n";
    out << "patternNone not {\n";
    out << "originalCTM setmatrix\n";
    out << "/padtip arrowHeight 2 exp 0.25 arrowWidth 2 exp mul add sqrt ";
    out << "brushWidth mul\n";
    out << "arrowWidth div def\n";
    out << "/padtail brushWidth 2 div def\n";
    out << "tipx tipy translate\n";
    out << "angle rotate\n";
    out << "padtip 0 translate\n";
    out << "arrowHeight padtip add padtail add arrowHeight div dup scale\n";
    out << "arrowheadpath\n";
    out << "ifill\n";
    out << "} if\n";
    out << "brushNone not {\n";
    out << "originalCTM setmatrix\n";
    out << "tipx tipy translate\n";
    out << "angle rotate\n";
    out << "arrowheadpath\n";
    out << "istroke\n";
    out << "} if\n";
    out << "grestore\n";
    out << "end\n";
    out << "} dup 0 9 dict put def\n\n";
    out << "/arrowheadpath {\n";
    out << "newpath\n";
    out << "arrowHeight neg arrowWidth 2 div moveto\n";
    out << "0 0 lineto\n";
    out << "arrowHeight neg arrowWidth 2 div neg lineto\n";
    out << "} def\n\n";
    out << "/leftarrow {\n";
    out << "0 begin\n";
    out << "y exch get /taily exch def\n";
    out << "x exch get /tailx exch def\n";
    out << "y exch get /tipy exch def\n";
    out << "x exch get /tipx exch def\n";
    out << "brushLeftArrow { tipx tipy tailx taily arrowhead } if\n";
    out << "end\n";
    out << "} dup 0 4 dict put def\n\n";
    out << "/rightarrow {\n";
    out << "0 begin\n";
    out << "y exch get /tipy exch def\n";
    out << "x exch get /tipx exch def\n";
    out << "y exch get /taily exch def\n";
    out << "x exch get /tailx exch def\n";
    out << "brushRightArrow { tipx tipy tailx taily arrowhead } if\n";
    out << "end\n";
    out << "} dup 0 4 dict put def\n\n";
}
