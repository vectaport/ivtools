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
 * Overlay Vertices component definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovvertices.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/Commands/align.h>

#include <Unidraw/Graphic/util.h>
#include <Unidraw/Graphic/verts.h>

#include <IV-2_6/InterViews/rubcurve.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <stream.h>

/****************************************************************************/

int VerticesOvComp::_symid = -1;

ClassId VerticesOvComp::GetClassId () { return OVVERTICES_COMP; }

boolean VerticesOvComp::IsA (ClassId id) {
    return OVVERTICES_COMP == id || OverlayComp::IsA(id);
}

VerticesOvComp::VerticesOvComp (Vertices* graphic, OverlayComp* parent) 
: OverlayComp(graphic, parent) { }

VerticesOvComp::VerticesOvComp(istream& in, OverlayComp* parent) 
: OverlayComp(nil, parent) {
}

Vertices* VerticesOvComp::GetVertices () { return (Vertices*) GetGraphic(); }

void VerticesOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("pts", ParamStruct::keyword, &VerticesScript::ReadPts,
		  this, this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

boolean VerticesOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    Vertices* verta = GetVertices();
    Vertices* vertb = ((VerticesOvComp&)comp).GetVertices();
    return *verta == *vertb &&
	OverlayComp::operator==(comp);
}

/****************************************************************************/

VerticesOvComp* VerticesOvView::GetVerticesOvComp () { 
    return (VerticesOvComp*) GetSubject();
}

ClassId VerticesOvView::GetClassId () { return OVVERTICES_VIEW; }

boolean VerticesOvView::IsA (ClassId id) {
    return OVVERTICES_VIEW == id || OverlayView::IsA(id);
}

VerticesOvView::VerticesOvView (VerticesOvComp* subj) : OverlayView(subj) { }

void VerticesOvView::Update () {
    Graphic* vertices = GetGraphic();

    IncurDamage(vertices);
    if (VertexChanged()) {
        // unimplemented
    }
    *vertices = *GetVerticesOvComp()->GetGraphic();
    IncurDamage(vertices);
    EraseHandles();
}

void VerticesOvView::CreateHandles () {
    Coord* x, *y;
    int n;
    Viewer* v = GetViewer();
    
    if (v != nil) {
        GetVertices(x, y, n);
        _handles = new RubberHandles(nil, nil, x, y, n, 0, HANDLE_SIZE);
        v->InitRubberband(_handles);
        delete x;
        delete y;
    }
}

boolean VerticesOvView::VertexChanged () { return false; }

void VerticesOvView::Interpret (Command* cmd) {
    if (cmd->IsA(ALIGNTOGRID_CMD)) {
        Vertices* verts = (Vertices*) GetGraphic();
        Transformer total;
        verts->TotalTransformation(total);

        float tx0, ty0;
        const Coord* x, *y;
        int n = verts->GetOriginal(x, y);
        total.Transform(float(x[0]), float(y[0]), tx0, ty0);
        ((AlignToGridCmd*) cmd)->Align(this, tx0, ty0);

    } else {
        OverlayView::Interpret(cmd);
    }
}

void VerticesOvView::GetVertices (Coord*& x, Coord*& y, int& n) {
    Vertices* vertices = (Vertices*) GetGraphic();
    Transformer t;
    const Coord* origx, *origy;

    n = vertices->GetOriginal(origx, origy);
    ArrayDup(origx, origy, n, x, y);
    vertices->TotalTransformation(t);
    t.TransformList(x, y, n);
}

Graphic* VerticesOvView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();

    if (graphic == nil) {
        VerticesOvComp* verticesComp = GetVerticesOvComp();
        graphic = verticesComp->GetGraphic()->Copy();
        SetGraphic(graphic);
    }
    return graphic;
}

/****************************************************************************/

ClassId VerticesPS::GetClassId () { return VERTICES_PS; }

boolean VerticesPS::IsA (ClassId id) { 
    return VERTICES_PS == id || OverlayPS::IsA(id);
}

VerticesPS::VerticesPS (OverlayComp* subj) : OverlayPS(subj) { }
const char* VerticesPS::Name () { return ""; }

boolean VerticesPS::Definition (ostream& out) {
    const Coord* x;
    const Coord* y;
    int n;

    Vertices* vertices = (Vertices*) GetGraphicComp()->GetGraphic();
    n = vertices->GetOriginal(x, y);

    out << "Begin " << MARK << " " << Name() << "\n";
    MinGS(out);
    out << MARK << " " << n << "\n";
    for (int i = 0; i < n; i++) {
        out << x[i] << " " << y[i] << "\n";
    }
    out << n << " " << Name() << "\n";
    out << "End\n\n";

    return out.good();
}

/****************************************************************************/

ClassId VerticesScript::GetClassId () { return VERTICES_SCRIPT; }

boolean VerticesScript::IsA (ClassId id) { 
    return VERTICES_SCRIPT == id || OverlayScript::IsA(id);
}

VerticesScript::VerticesScript (VerticesOvComp* subj) : OverlayScript(subj) { }
const char* VerticesScript::Name () { return "vertices"; }

boolean VerticesScript::Definition (ostream& out) {
    const Coord* x;
    const Coord* y;
    int n;

    VerticesOvComp* comp = (VerticesOvComp*) GetSubject();
    n = comp->GetVertices()->GetOriginal(x, y);

    out << Name() << "(";
    Clipboard* cb = GetPtsList();
    if (cb) {
	out << " :pts " << MatchedPts(cb);
    } else {
	for (int i = 0; i < n; ) {
	    for (int j = 0; j < 10 && i < n; j++, i++) {
		out << "(" << x[i] << "," << y[i] << ")";
		if (i+1 < n ) 
		    out << ",";
	    }
	    if (i+1 < n ) {
		out << "\n";
		Indent(out);
	    }
	}
    }
    MinGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}

int VerticesScript::ReadPts (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    OverlayComp* comp = (OverlayComp*)addr1;
    Vertices* vert = *(Vertices**)addr2;
    int id;
    in >> id;
    if (id>=0) 
      vert->SetOriginal(comp->GetIndexedPts(id));
    return in.good() ? 0 : -1;
}


