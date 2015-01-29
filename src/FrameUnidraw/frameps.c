/*
 * Copyright (c) 1994, 1995 Vectaport Inc., Cartoactive Systems
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
 * FrameIdrawPS definition
 */

#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/frameps.h>

#include <Unidraw/Graphic/graphic.h>

#include <Unidraw/iterator.h>

#include <InterViews/transformer.h>

#include <iostream.h>

/*****************************************************************************/

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

/*****************************************************************************/

FrameIdrawPS::FrameIdrawPS (FrameIdrawComp* subj) : OverlayIdrawPS(subj) { }
ClassId FrameIdrawPS::GetClassId () { return FRAME_IDRAW_PS; }

boolean FrameIdrawPS::IsA (ClassId id) { 
    return FRAME_IDRAW_PS == id || OverlayIdrawPS::IsA(id);
}

boolean FrameIdrawPS::Emit (ostream& out) {
    SetPSFonts();

    Graphic* g = GetGraphicComp()->GetGraphic();
    Transformer* t = SaveTransformer(g);
    ScaleToPostScriptCoords(g);

    Comments(out);
    Prologue(out);
    Version(out);
    GridSpacing(out);

    /* count up number of pages (not counting the zeroth) */
    Iterator i;
    int npage = -1;
    for (First(i); !Done(i); Next(i)) npage++;

    /* loop through the first through nth page */
    First(i); Next(i);
    Iterator j;
    First(j);
    boolean status = true;
    int pagenum = 1;
    while (status && !Done(i)) {

	out << "\n\n%%Page: " << pagenum << " " << npage << "\n\n";
	out << "Begin\n";
	FullGS(out);
	out << "/originalCTM matrix currentmatrix def\n\n";

	status = ((PostScriptView*)GetView(j))->PreorderView::Definition(out);
	if (!status) break;
	out << "\n";
	status = ((PostScriptView*)GetView(i))->PreorderView::Definition(out);
	if (!status) break;

	out << "End " << MARK << " eop\n\n";
	out << "showpage\n\n";

	Next(i);
	pagenum++;
    }
    /* done looping through the first through nth page */

    Trailer(out);
    RestoreTransformer(g, t);

    return status;
}

