/*
 * Copyright (c) 1994-1998 Vectaport Inc.
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
#include <ComUnidraw/unifunc.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovselection.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/creator.h>
#include <Unidraw/globals.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/Commands/command.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Components/compview.h>
#include <Unidraw/Graphic/graphic.h>
#include <InterViews/transformer.h>
#include <InterViews/window.h>
#include <ComTerp/comterp.h>
#include <ComTerp/comvalue.h>
#include <Attribute/attrlist.h>
#include <stdio.h>
#include <fstream.h>
#include <unistd.h>

#define TITLE "UnidrawFunc"

/*****************************************************************************/

int UnidrawFunc::_compview_id = -1;

UnidrawFunc::UnidrawFunc(ComTerp* comterp, Editor* ed) : ComFunc(comterp) {
    _ed = ed;
    if (_compview_id<0)
        _compview_id = symbol_add("CompView");
}

void UnidrawFunc::execute_log(Command* cmd) {
    if (cmd != nil) {
	cmd->Execute();
	
	if (cmd->Reversible()) {
	    cmd->Log();
	} else {
	    delete cmd;
	}
    }
}

/*****************************************************************************/

HandlesFunc::HandlesFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void HandlesFunc::execute() {
    ComValue& flag = stack_arg(0);
    if (flag.int_val()) 
	((OverlaySelection*)_ed->GetSelection())->EnableHandles();
    else
	((OverlaySelection*)_ed->GetSelection())->DisableHandles();
    reset_stack();
}

/*****************************************************************************/

PasteFunc::PasteFunc(ComTerp* comterp, Editor* ed, OverlayCatalog* catalog) : UnidrawFunc(comterp, ed) {
  _catalog = catalog;
}

void PasteFunc::execute() {
    ComValue viewv(stack_arg(0));
    ComValue xscalev(stack_arg(1));
    ComValue yscalev(stack_arg(2));
    ComValue xoffv(stack_arg(3));
    ComValue yoffv(stack_arg(4));
    reset_stack();

    /* extract comp and scale/translate before pasting */
    ComponentView* view = (ComponentView*)viewv.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();
    Graphic* gr = comp->GetGraphic();
    if (gr) {
      if (xscalev.is_num() && yscalev.is_num()) {
	float af[6];
	af[0] = xscalev.float_val();
	af[1] = af[2] = 0.0;
	af[3] = yscalev.float_val();
	if (xoffv.is_num() && yoffv.is_num()) {
	  af[4] = xoffv.float_val();
	  af[5] = yoffv.float_val();
	} else
	  af[4] = af[5] = 0.0;
	gr->SetTransformer(new Transformer(af[0],af[1],af[2],af[3],af[4],af[5]));
      } else if (xscalev.is_array()) {
	ComValue& tranv = xscalev;
	AttributeValueList* avl = tranv.array_val();
	Iterator i;
	avl->First(i);
	int num = tranv.array_len();
	float af[6];
	for (int j=0; j<6; j++) {
	  af[j] = j<num ? avl->GetAttrVal(i)->float_val() : 0.0;
	  avl->Next(i);
	}
	gr->SetTransformer(new Transformer(af[0],af[1],af[2],af[3],af[4],af[5]));
      }
    }
    
    /* set creator for gvupdater to use and disable unidraw (!use_unidraw) */
    /* then restore server/viewer state (use_unidraw) and original creator */
    PasteCmd* cmd = new PasteCmd(_ed, new Clipboard(comp));
    Creator* oldcreator = Creator::instance();
    if (_catalog && _catalog->GetCreator()) 
      Creator::instance(_catalog->GetCreator());
    boolean uflag = Component::use_unidraw();	
    Component::use_unidraw(_catalog ? false : true);
    execute_log(cmd);
    Creator::instance(oldcreator);
    Component::use_unidraw(uflag);

    push_stack(viewv);
}

/*****************************************************************************/

ReadOnlyFunc::ReadOnlyFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
    _clear_symid = symbol_add("clear");
}

void ReadOnlyFunc::execute() {
    ComValue viewval(stack_arg(0));
    ComValue clear(stack_key(_clear_symid));
    reset_stack();

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    /* should be done with an attribute setting command */
    AttributeList* al = comp->GetAttributeList();
    al->add_attr("readonly", ComValue::trueval());

    push_stack(viewval);
}

/*****************************************************************************/

BarPlotFunc::BarPlotFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
  _title_symid = symbol_add("title");
  _valtitle_symid = symbol_add("valtitle");
  _xtitle_symid = symbol_add("xtitle");
  _ytitle_symid = symbol_add("ytitle");
}

void BarPlotFunc::execute() {
  if (Component::use_unidraw()) {
    boolean ok;
    char* tmpfilename = tempnam(NULL,"plot");
    ofstream out(tmpfilename);

    ComValue title(stack_key(_title_symid));
    ComValue xtitle(stack_key(_xtitle_symid));
    ComValue ytitle(stack_key(_ytitle_symid));
    ComValue vtitle(stack_key(_valtitle_symid));
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

    ComEditor* ed = new ComEditor((const char*)nil);
    unidraw->Open(ed);
    OvImportCmd* imp = new OvImportCmd(ed);
    imp->pathname(idtmp);
    imp->Execute();

    //remove(pstmp);
    //remove(tmpfilename);
  }
  reset_stack();
}
