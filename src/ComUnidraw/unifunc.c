/*
 * Copyright (c) 2000 Vectaport Inc, IET Inc.
 * Copyright (c) 1994-1999 Vectaport Inc.
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
#include <OverlayUnidraw/ovviews.h>
#include <ComGlyph/comtextedit.h>
#include <ComGlyph/comtextview.h>
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
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <Attribute/attrlist.h>
#include <stdio.h>
#include <strstream.h>
#include <unistd.h>

#define TITLE "UnidrawFunc"

/*****************************************************************************/

UnidrawFunc::UnidrawFunc(ComTerp* comterp, Editor* ed) : ComFunc(comterp) {
    _ed = ed;
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

void UnidrawFunc::menulength_execute(const char* kind) {
  reset_stack();
  int itemcount=0;
  Catalog* catalog = unidraw->GetCatalog();
  while(catalog->GetAttribute(catalog->Name(kind, itemcount+1)))
    itemcount++;
  ComValue retval(itemcount);
  push_stack(retval);
}

/*****************************************************************************/

UpdateFunc::UpdateFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void UpdateFunc::execute() {
  reset_stack();
  unidraw->Update(true);
  
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
    OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
    Graphic* gr = comp ? comp->GetGraphic() : nil;
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
    } else {
      push_stack(ComValue::nullval());
      return;
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

int PasteModeFunc:: _paste_mode = 0;

PasteModeFunc::PasteModeFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PasteModeFunc::execute() {
  static int get_symid = symbol_add("get");
  boolean get_flag = stack_key(get_symid).is_true();
  if (get_flag) {
    reset_stack();
    int mode = paste_mode();
    ComValue retval(mode, ComValue::IntType);
    push_stack(retval);
  } else {
    if (nargs()==0) {
      reset_stack();
      int mode = !paste_mode();
      paste_mode(mode);
      ComValue retval(mode, ComValue::IntType);
      push_stack(retval);
    } else {
      ComValue retval(stack_arg(0));
      reset_stack();
      paste_mode(retval.int_val());
      push_stack(retval);
    }
  }
}

/*****************************************************************************/

ReadOnlyFunc::ReadOnlyFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ReadOnlyFunc::execute() {
    ComValue viewval(stack_arg(0));
    static int clear_symid = symbol_add("clear");
    ComValue clear(stack_key(clear_symid));
    reset_stack();

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    AttributeList* al = comp->GetAttributeList();
    al->add_attr("readonly", ComValue::trueval());

    push_stack(viewval);
}

/*****************************************************************************/

ImportFunc::ImportFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

OvImportCmd* ImportFunc::import(const char* path, boolean popen) {
  OvImportCmd* cmd = new OvImportCmd(editor());
  cmd->pathname(path, popen);
  cmd->Execute();
  if (cmd->component()) {
    ((OverlayComp*)cmd->component())->SetPathName(path);
    ((OverlayComp*)cmd->component())->SetByPathnameFlag(!popen);
  }
  return cmd;
}

void ImportFunc::execute() {
    ComValue pathnamev(stack_arg(0));
    static int popen_symid = symbol_add("popen");
    boolean popen_flag = stack_key(popen_symid).is_true();
    reset_stack();
    
    OvImportCmd* cmd;
    if (!pathnamev.is_array()) {
      if (nargs()==1) {
	if ((cmd = import(pathnamev.string_ptr(), popen_flag)) && cmd->component()) {
	  ComValue compval(((OverlayComp*)cmd->component())->classid(),
			   new ComponentView(cmd->component()));
	  delete cmd;
	  compval.object_compview(true);
	  push_stack(compval);
	} else
	  push_stack(ComValue::nullval());
      } else {
	for (int i=0; i<nargs(); i++) 
	  if (cmd = import(stack_arg(i).string_ptr(), popen_flag)) {
	    ComValue compval(((OverlayComp*)cmd->component())->classid(),
			     new ComponentView(cmd->component()));
	    delete cmd;
	    compval.object_compview(true);
	    push_stack(compval);

	  } else
	    push_stack(ComValue::nullval());
      }
    } else   {
      AttributeValueList* outlist = new AttributeValueList();
      AttributeValueList* inlist = pathnamev.array_val();
      Iterator it;
      inlist->First(it);
      while(!inlist->Done(it)) {
	cmd = import(inlist->GetAttrVal(it)->string_ptr(), popen_flag);
	ComValue* val = new ComValue(((OverlayComp*)cmd->component())->classid(),
				     new ComponentView(cmd->component()));
	delete cmd;
	val->object_compview(true);
	outlist->Append(val);
	inlist->Next(it);
      }
    }
}

/*****************************************************************************/

SetAttrFunc::SetAttrFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void SetAttrFunc::execute() {
    ComValue viewval(stack_arg(0));
    AttributeList* al = stack_keys();
    reset_stack();
    if (!viewval.is_object()) {
      push_stack(ComValue::nullval());
      return;
    }

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    AttributeList* comp_al = comp ? comp->attrlist() : nil;
    if (!comp_al && comp)
      comp->SetAttributeList(al);
    else if (comp_al) {
      comp_al->merge(al);
      delete al;
    }
    push_stack(viewval);
}

/*****************************************************************************/

FrameFunc::FrameFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void FrameFunc::execute() {
    ComValue indexv(stack_arg(0, false, ComValue::minusoneval()));
    reset_stack();
    OverlayEditor* ed = (OverlayEditor*)GetEditor();

    OverlaysView* frameview = ed->GetFrame(indexv.int_val());
    if (frameview && frameview->GetSubject()) {
      OverlayComp* comp = (OverlayComp*)frameview->GetSubject();
      ComValue retval(comp->classid(), new ComponentView(comp));
      retval.object_compview(true);
      push_stack(retval);
    } else
      push_stack(ComValue::nullval());
}

/*****************************************************************************/

UnidrawPauseFunc::UnidrawPauseFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void UnidrawPauseFunc::execute() {
  ComValue msgstrv(stack_arg(0));
  reset_stack();
  comterpserv()->npause()++;

  ComTextEditor* te = (ComTextEditor*) 
    ((OverlayEditor*)GetEditor())->TextEditor();
  if (te) {
    ComTE_View* tv = te->comtextview();
    if (tv) {
      if (msgstrv.is_string()) {
	tv->insert_string((char*)msgstrv.string_ptr(), strlen(msgstrv.string_ptr()));
	tv->insert_char('\n');
      }
      ostrstream sbuf_s;
      sbuf_s << "pause(" << comterpserv()->npause() << "): enter command or press C/R to continue\n";
      sbuf_s.put('\0');
      tv->insert_string(sbuf_s.str(), strlen(sbuf_s.str()));
      comterpserv()->push_servstate();
      unidraw->Run();
      comterpserv()->pop_servstate();
      ostrstream sbuf_e;
      sbuf_e << "end of pause(" << comterpserv()->npause()+1 << ")\n";
      sbuf_e.put('\0');
      tv->insert_string(sbuf_e.str(), strlen(sbuf_e.str()));
    }
  } else {
    cerr << "this version of pause command only works with ComTextEditor\n";
  }
}

/*****************************************************************************/

AddToolButtonFunc::AddToolButtonFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void AddToolButtonFunc::execute() {
    ComValue pathnamev(stack_arg(0));
    reset_stack();
    OverlayEditor* ed = (OverlayEditor*)GetEditor();
    OverlayComp* comp = ed->overlay_kit()->add_tool_button(pathnamev.symbol_ptr());
    ComValue retval(comp->classid(), new ComponentView(comp));
    retval.object_compview(true);
    push_stack(retval);
}

