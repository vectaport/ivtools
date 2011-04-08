/*
 * Copyright (c) 1998 Vectaport Inc.
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

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovhull.h>
#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovviewer.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/macro.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/manips.h>
#include <Unidraw/statevars.h>
#include <Unidraw/Components/polygon.h>
#include <Unidraw/Graphic/polygons.h>
#include <IV-2_6/InterViews/rubverts.h>
#include <InterViews/event.h>
#include <InterViews/transformer.h>
#include <OS/math.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream.h>

ConvexHullCmd::ConvexHullCmd(Editor* ed, Clipboard* cb) : Command(ed, cb) {
}

void ConvexHullCmd::Execute() {
  Iterator it;
  GetClipboard()->First(it);
  GraphicComp* comp = GetClipboard()->GetComp(it);
  if (comp && comp->IsA(OVPOLYGON_COMP)) {
    SF_Polygon* poly = ((PolygonOvComp*)comp)->GetPolygon();
    Coord* x, *y;
    int np = poly->GetOriginal((const Coord*&)x,(const Coord*&) y);
    if (np > 2) {
      float* fx = new float[np];
      float* fy = new float[np];
      for (int i = 0; i < np; i++) {
	fx[i] = x[i];
	fy[i] = y[i];
      }
      float* hx;
      float* hy;
      int nh = ConvexHull(np, fx, fy, hx, hy);
      if (nh > 0) {
	Coord* xh = new Coord[nh];
	Coord* yh = new Coord[nh];
	for (int i = 0; i < nh; i++) {
	  xh[i] = Math::round(hx[i]);
	  yh[i] = Math::round(hy[i]);
	}
	delete [] hx;
	delete [] hy;
	SF_Polygon* newpoly = new SF_Polygon(xh, yh, nh, poly);
	PolygonOvComp* newcomp = new PolygonOvComp(newpoly);
	Clipboard* pastecb = new Clipboard(newcomp);
	PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
	pastecmd->Execute();
      }
      delete [] fx;
      delete [] fy;
    }
  }
}

int ConvexHullCmd::ConvexHull(int np, float* fx, float* fy, float*& hx, float*& hy) {
  if (np > 2 && OverlayKit::bincheck("qhull") ) {
    char* tnam = tempnam("/tmp", "qhin");
    if (tnam) {
      FILE* fp = fopen(tnam, "w");
      if (fp) {
	fprintf(fp, "%d\n%d\n", 2, np);
#ifdef DEBUG
	cerr << "convex hull input\n";
#endif
	for (int i = 0; i < np; i++) {
	  fprintf(fp, "%f %f\n", fx[i], fy[i]);
#ifdef DEBUG
	  cerr << i << ": " << fx[i] << "," << fy[i] << "\n";
#endif
	}

	fclose(fp);
	char qhcmd[100];
	sprintf(qhcmd, "qhull Fx < %s", tnam);
	FILE* pp = popen(qhcmd, "r");
	int nhp;
	if (pp) {
	  char line[80];
	  fgets(line, 80, pp);
	  sscanf(line, "%d", &nhp);
	  if (nhp) {
	    
	    hx = new float[nhp];
	    hy = new float[nhp];
	    for (int i = 0; i < nhp; i++) {
	      int idx;
	      fgets(line, 80, pp);
	      sscanf(line, "%d", &idx);
	      hx[i] = fx[idx];
	      hy[i] = fy[idx];
	    }
#ifdef DEBUG
	    cerr << "convex hull output\n";
	    for (int i=0; i<np; i++) 
	      cerr << i << ": " << hx[i] << "," << hy[i] << "\n";
#endif
	  } else {
	    nhp = np;
	    hx = new float[nhp];
	    hy = new float[nhp];
	    for (int i = 0; i < nhp; i++) {
	      hx[i] = fx[i];
	      hy[i] = fy[i];
	    }
	  }
	  pclose(pp);
	}
	else
	  return 0;
	unlink(tnam);
	return nhp;
      }
    }
    return 1;
  }
  else
    return 0;
}

ConvexHullTool::ConvexHullTool(ControlInfo* ci) : Tool(ci) {
}

Manipulator* ConvexHullTool::CreateManipulator
  (Viewer* v, Event& e, Transformer* rel)
{
  Coord x[1], y[1];
  x[0] = e.x;
  y[0] = e.y;
  GrowingVertices* rub = new GrowingPolygon(nil, nil, x, y, 1, -1, HANDLE_SIZE
        );
  Manipulator* m =
    new VertexManip(v, rub, rel, this);
  return m;
}

Command* ConvexHullTool::InterpretManipulator(Manipulator* m) {
    VertexManip* vm = (VertexManip*) m;
    Viewer* viewer = vm->GetViewer();
    Editor* ed = viewer->GetEditor();
    GraphicView* views = viewer->GetGraphicView();
    GrowingVertices* gv = vm->GetGrowingVertices();
    Coord* x;
    Coord* y;
    int n;
    Clipboard* pastecb = new Clipboard();
    FullGraphic gs(stdgraphic);
    gs.SetPattern(psnonepat);
    Transformer* rel = new Transformer(((OverlayViewer*)viewer)->GetRel());
    rel->Invert();
    gs.SetTransformer(rel);
    rel->unref();

    gv->GetCurrent(x, y, n);
    SF_Polygon* cpoly = new SF_Polygon(x, y, n, &gs);

    BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
    PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
    ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");
    if (brVar != nil) cpoly->SetBrush(brVar->GetBrush());
    if (patVar != nil) cpoly->SetPattern(patVar->GetPattern());
    if (colVar != nil) {
      cpoly->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
    }


    PolygonOvComp* ccomp = new PolygonOvComp(cpoly);
    pastecb->Append(ccomp);
    PasteCmd* pastecmd = new PasteCmd(viewer->GetEditor(), pastecb);
    MacroCmd* cmds = new MacroCmd(viewer->GetEditor());
    cmds->Append(pastecmd);
    ConvexHullCmd* hullcmd = new ConvexHullCmd(viewer->GetEditor(),
					       pastecb->DeepCopy());
    cmds->Append(hullcmd);
    return cmds;
}
