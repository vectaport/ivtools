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

#include <ComUnidraw/plotfunc.h>
#include <ComUnidraw/comeditor.h>
#include <OverlayUnidraw/ovimport.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/Components/component.h>
#include <fstream.h>
#include <unistd.h>

#define TITLE "PlotFunc"

/*****************************************************************************/

BarPlotFunc::BarPlotFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void BarPlotFunc::execute() {
  static int title_symid = symbol_add("title");
  static int valtitle_symid = symbol_add("valtitle");
  static int xtitle_symid = symbol_add("xtitle");
  static int ytitle_symid = symbol_add("ytitle");
  static int newview_symid = symbol_add("newview");

  if (Component::use_unidraw()) {
    boolean ok;
    char* tmpfilename = tempnam(NULL,"plot");
    ofstream out(tmpfilename);

    ComValue title(stack_key(title_symid));
    ComValue xtitle(stack_key(xtitle_symid));
    ComValue ytitle(stack_key(ytitle_symid));
    ComValue vtitle(stack_key(valtitle_symid));
    ComValue newview_flag(stack_key(newview_symid));
    char* ts = "";
    char* xs = "";
    char* ys = "";
    char* vs = "";
    if (title.is_string())
      ts = (char*)title.string_ptr();
    if (xtitle.is_string())
      xs = (char*)xtitle.string_ptr();
    if (ytitle.is_string())
      ys = (char*)ytitle.string_ptr();
    if (vtitle.is_string())
      vs = (char*)vtitle.string_ptr();

    out << "$ DATA=BARCHART\n";
    out << "% toplabel = \"" << ts << "\"\n";
    out << "% xlabel = \"" << xs << "\"\n";
    out << "% ylabel = \"" << ys << "\"\n";
    out << "\t\"" << vs << "\"\n";

    for (int i = 0; i < nargsfixed(); i += 2) {
      ComValue var(stack_arg(i));
      ComValue val(stack_arg(i+1));
      if (var.is_string() && val.is_num()) {
	char* vars = (char*)var.string_ptr();
	double v = val.double_val();
	out << "\"" << vars << "\"  " << v << "\n";
      }
    }

    out << "$ END\n";
    out.flush();
    out.close();

    char cmd[256];
    char* pstmp = tempnam(NULL,"ps");
    sprintf(cmd, "plotmtv -noxplot -color -o %s %s", pstmp, tmpfilename);
    FILE* plotp = popen(cmd, "w");
    fprintf(plotp, "n\n");
    pclose(plotp);

    char* idtmp = tempnam(NULL,"idraw");
    sprintf(cmd, "pstoedit -f idraw < %s > %s", pstmp, idtmp);
fprintf(stderr, "%s\n", cmd);
    system(cmd);

    ComEditor* ed = nil;
    if (newview_flag.is_true()) {
      ed = new ComEditor((const char*)nil);
      unidraw->Open(ed);
    } else
      ed = (ComEditor*) GetEditor();

    OvImportCmd* imp = new OvImportCmd(ed);
    imp->pathname(idtmp);
    imp->Execute();

    unlink(pstmp);
    unlink(tmpfilename);
  }
  reset_stack();
}
