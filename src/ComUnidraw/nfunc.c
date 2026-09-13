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

#include <ComUnidraw/comeditor.h>
#include <ComUnidraw/nfunc.h>
#include <OverlayUnidraw/ovraster.h>
#include <Unidraw/viewer.h>
#include <InterViews/canvas.h>

#define TITLE "NFunc"

/*****************************************************************************/

NColsFunc::NColsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void NColsFunc::execute() {
  boolean raster_arg = false;
  if (nargs()) {
    ComValue argv(stack_arg(0));
    raster_arg = argv.geta(RasterOvComp::class_symid()) != nil;
  }
  reset_stack();
  if (raster_arg) {
    fprintf(stderr, "WARNING:  ncols() takes no argument -- it reports the "
	    "onscreen window's pixel width, not a raster's column count; "
	    "use pcols(compview) for that -- line %d -- returning nil\n",
	    funcstate()->linenum());
    push_stack(ComValue::nullval());
    return;
  }
  Canvas* canvas = GetEditor()->GetViewer()->GetCanvas();
  if (canvas) {
    ComValue retval(canvas->pwidth());
    push_stack(retval);
  }
}

/*****************************************************************************/

NRowsFunc::NRowsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void NRowsFunc::execute() {
  boolean raster_arg = false;
  if (nargs()) {
    ComValue argv(stack_arg(0));
    raster_arg = argv.geta(RasterOvComp::class_symid()) != nil;
  }
  reset_stack();
  if (raster_arg) {
    fprintf(stderr, "WARNING:  nrows() takes no argument -- it reports the "
	    "onscreen window's pixel height, not a raster's row count; "
	    "use prows(compview) for that -- line %d -- returning nil\n",
	    funcstate()->linenum());
    push_stack(ComValue::nullval());
    return;
  }
  Canvas* canvas = GetEditor()->GetViewer()->GetCanvas();
  if (canvas) {
    ComValue retval(canvas->pheight());
    push_stack(retval);
  }
}

/*****************************************************************************/

NFontsFunc::NFontsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void NFontsFunc::execute() {
  menulength_execute("font");
}

/*****************************************************************************/

NBrushesFunc::NBrushesFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void NBrushesFunc::execute() {
  menulength_execute("brush");
}

/*****************************************************************************/

NPatternsFunc::NPatternsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void NPatternsFunc::execute() {
  menulength_execute("pattern");
}

/*****************************************************************************/

NColorsFunc::NColorsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void NColorsFunc::execute() {
  menulength_execute("fgcolor");
}

