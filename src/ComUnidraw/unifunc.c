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
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/scriptview.h>
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
#if __GNUG__>=3
#include <fstream.h>
#endif

#ifdef HAVE_ACE
#include <ComTerp/comhandler.h>
#include <ace/SOCK_Connector.h>
#endif

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

SaveFileFunc::SaveFileFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

Command* SaveFileFunc::save(const char* path) {
  if (!path) {
    OvSaveCompCmd* cmd = new OvSaveCompCmd(editor());
    cmd->Execute();
    return cmd->component() ? cmd : nil; 
  } else {
    OvSaveCompAsCmd* cmd = new OvSaveCompAsCmd(editor());
    cmd->pathname(path);
    cmd->Execute();
    return cmd->component() ? cmd : nil; 
  }
}

void SaveFileFunc::execute() {
    const char* path = nil;
    if (nargs()>0) {
      ComValue pathnamev(stack_arg(0));
      path = pathnamev.string_ptr();
    }
    reset_stack();
    
    push_stack( save(path) ? ComValue::oneval()	: ComValue::zeroval());
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
    static char* lastpath = nil;

    ComValue pathnamev(stack_arg(0));
    static int popen_symid = symbol_add("popen");
    boolean popen_flag = stack_key(popen_symid).is_true();
    static int next_symid = symbol_add("next");
    boolean next_flag = stack_key(next_symid).is_true();
    reset_stack();

    if (next_flag) {
      if (lastpath) {
        char* ptr = lastpath + strlen(lastpath) - 1;
	while ((*ptr < '0' || *ptr > '9') && ptr > lastpath ) ptr--;
	if (*ptr >= '0' && *ptr <= '9') {
	  do {
	    if (*ptr >= '0' && *ptr <= '8') *ptr = ++*ptr;
	    else *ptr = '0';
	  } while (*ptr == '0' && --ptr > lastpath);
	}
      } else {
	lastpath = strnew(pathnamev.string_ptr());
      }
    } else {
      delete lastpath;
      lastpath = nil;
    }

    if (!next_flag && pathnamev.is_string()) 
      lastpath = strnew(pathnamev.string_ptr());

    
    OvImportCmd* cmd;
    if (!pathnamev.is_array()) {
      if (nargs()==1 || next_flag) {
	if ((cmd = import(next_flag ? lastpath : pathnamev.string_ptr(), 
			  popen_flag)) && cmd->component()) {
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

ExportFunc::ExportFunc(ComTerp* comterp, Editor* editor, 
		       const char* appname) :     
  UnidrawFunc(comterp, editor)
{
  _appname = appname;
  _docstring = nil;
}

const char* ExportFunc::docstring() { 
  const char* df = 
#ifdef HAVE_ACE
    "%s(compview[,compview[,...compview]] [path] :host host_str :port port_int :socket :string|:str :eps :idraw) -- export in %s format ";
#else
  "%s(compview[,compview[,...compview]] [path] :string|:str :eps :idraw) -- export in %s format ";
#endif
  if (!_docstring) {
    _docstring = new char[strlen(df)+strlen(appname())+1];
    sprintf(_docstring, df, "%s", appname() );
  }
  return _docstring;
}

void ExportFunc::execute() {
    static int _host_symid = symbol_add("host");
    static int _port_symid = symbol_add("port");
    static int _sock_symid = symbol_add("socket");
    static int _string_symid = symbol_add("string");
    static int _str_symid = symbol_add("str");
    static int _eps_symid = symbol_add("eps");
    static int _idraw_symid = symbol_add("idraw");
    ComValue compviewv(stack_arg(0));
    ComValue file(stack_arg(1));
    ComValue host(stack_key(_host_symid));
    ComValue port(stack_key(_port_symid));
    ComValue sock(stack_key(_sock_symid));
    ComValue string(stack_key(_string_symid));
    ComValue str(stack_key(_str_symid));
    ComValue eps_flag(stack_key(_eps_symid));
    ComValue idraw_flag(stack_key(_idraw_symid));
    reset_stack();
    if (nargs()==0 || compviewv.null() 
	|| compviewv.type() == ComValue::BlankType) {
        push_stack(ComValue::nullval());
        return;
    }

#ifdef HAVE_ACE
    ACE_SOCK_Stream* socket = nil;
#endif


#if __GNUG__<3
    filebuf fbuf;
    if (file.is_type(ComValue::StringType))
        fbuf.open(file.string_ptr(), "w");

    else if (sock.is_true()) {
#ifdef HAVE_ACE
	ComTerpServ* terp = (ComTerpServ*)comterp();
	ComterpHandler* handler = (ComterpHandler*)terp->handler();
	if (handler) {
	  ACE_SOCK_Stream peer = handler->peer();
	  fbuf.attach(peer.get_handle());
	}
	else
#endif
	  fbuf.attach(fileno(stdout));
    }

    else {
#ifdef HAVE_ACE
        const char* hoststr = nil;
        const char* portstr = nil;
        hoststr = host.type()==ComValue::StringType ? host.string_ptr() : nil;
        portstr = port.type()==ComValue::StringType ? port.string_ptr() : nil;
        u_short portnum = portstr ? atoi(portstr) : port.ushort_val();
    
        if (portnum) {
            socket = new ACE_SOCK_Stream;
            ACE_SOCK_Connector conn;
            ACE_INET_Addr addr (portnum, hoststr);
    
            if (conn.connect (*socket, addr) == -1)
                ACE_ERROR ((LM_ERROR, "%p\n", "open"));
            fbuf.attach(socket->get_handle());
        } else if (comterp()->handler() && comterp()->handler()->get_handle()>-1) {
            fbuf.attach(comterp()->handler()->get_handle());
        } else
#endif
            fbuf.attach(fileno(stdout));
    }

#else

    filebuf* pfbuf;
    FILE* ofptr = nil;

    if (file.is_type(ComValue::StringType)) {
      pfbuf = new filebuf();
      pfbuf->open(file.string_ptr(), input);
    }

    else if (sock.is_true()) {
#ifdef HAVE_ACE
	ComTerpServ* terp = (ComTerpServ*)comterp();
	ComterpHandler* handler = (ComterpHandler*)terp->handler();
	if (handler) {
	  ACE_SOCK_Stream peer = handler->peer();
	  ofptr = fdopen(peer.get_handle(), "r");
	  pfbuf = new fileptr_filebuf(ofptr, output);
	}
	else 
#endif
	  pfbuf = new fileptr_filebuf(stdout, output);
    }

    else {
#ifdef HAVE_ACE
        const char* hoststr = nil;
        const char* portstr = nil;
        hoststr = host.type()==ComValue::StringType ? host.string_ptr() : nil;
        portstr = port.type()==ComValue::StringType ? port.string_ptr() : nil;
        u_short portnum = portstr ? atoi(portstr) : port.ushort_val();
    
        if (portnum) {
            socket = new ACE_SOCK_Stream;
            ACE_SOCK_Connector conn;
            ACE_INET_Addr addr (portnum, hoststr);
    
            if (conn.connect (*socket, addr) == -1)
                ACE_ERROR ((LM_ERROR, "%p\n", "open"));
            pfbuf = new fileptr_filebuf(ofptr = fdopen(socket->get_handle(), "r"), output);
        } else if (comterp()->handler() && comterp()->handler()->get_handle()>-1) {
            pfbuf = new fileptr_filebuf(comterp()->handler()->rdfptr(), output);
        } else
#endif
            pfbuf = new fileptr_filebuf(stdout, output);
    }

#endif
    ostream* out;
    if (string.is_true()||str.is_true())
      out = new strstream();
    else
#if __GNUG__<3      
      out = new ostream(&fbuf);
#else
      out = new ostream(pfbuf);
#endif

    if (!compviewv.is_array()) {

      ComponentView* view = (ComponentView*)compviewv.obj_val();
      OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
      if (!comp) return;
      if (!eps_flag.is_true() && !idraw_flag.is_true()) {
	*out << appname() << "(\n";
	compout(comp, out);
	*out << ")\n";
      } else {
	OverlayPS::idraw_format = idraw_flag.is_true();
	OverlayPS* psv = (OverlayPS*) comp->Create(POSTSCRIPT_VIEW);
	comp->Attach(psv);
	psv->Update();
	psv->Emit(*out);
	comp->Detach(psv);
	delete psv;
      }
      
    } else {

      if (!eps_flag.is_true() && !idraw_flag.is_true()) {
	*out << appname() << "(\n";
	AttributeValueList* avl = compviewv.array_val();
	Iterator i;
	for(avl->First(i);!avl->Done(i); ) {
	  ComponentView* view = (ComponentView*)avl->GetAttrVal(i)->obj_val();
	  OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
	  if (!comp) break;
	  compout(comp, out);
	  avl->Next(i);
	  if (!avl->Done(i)) *out << ",\n";
	}
	*out << ")\n";
      } else {
	AttributeValueList* avl = compviewv.array_val();
	Iterator i;
	for(avl->First(i);!avl->Done(i); ) {
	  ComponentView* view = (ComponentView*)avl->GetAttrVal(i)->obj_val();
	  OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
	  if (!comp) break;
	  OverlayPS* psv = (OverlayPS*) comp->Create(POSTSCRIPT_VIEW);
	  comp->Attach(psv);
	  psv->Update();
	  psv->Emit(*out);
	  comp->Detach(psv);
	  delete psv;
	  avl->Next(i);
	}
      }

    }
    
    if (string.is_true()||str.is_true()) {
      *out << '\0'; out->flush();
      ComValue retval(((strstream*)out)->str());
      push_stack(retval);
    }
    delete out;

#if __GNUG__>=3
    delete pfbuf;
#endif    
    
#ifdef HAVE_ACE
    if (sock.is_false() && socket) {
      if (socket->close () == -1)
	ACE_ERROR ((LM_ERROR, "%p\n", "close"));
      delete(socket);
    }
#endif
}

void ExportFunc::compout(OverlayComp* comp, ostream* out) {
  OverlayScript* ovsv = (OverlayScript*) comp->Create(SCRIPT_VIEW);
  comp->Attach(ovsv);
  ovsv->Update();
  ovsv->Definition(*out);
  delete ovsv;
  AttributeList* attrlist = comp->GetAttributeList();
  *out << *attrlist;
  out->flush();
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
    if (comp) {
      ComValue retval(comp->classid(), new ComponentView(comp));
      retval.object_compview(true);
      push_stack(retval);
    } else {
      push_stack(ComValue::nullval());
    }
}

/*****************************************************************************/

ScreenToDrawingFunc::ScreenToDrawingFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ScreenToDrawingFunc::execute() {
  ComValue coordsv(stack_arg(0));
  reset_stack();
  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlayViewer* viewer = ed ? (OverlayViewer*)ed->GetViewer() : nil;
  if (viewer && coordsv.is_array() && coordsv.array_len()==2) {
    AttributeValueList *avl = coordsv.array_val();
    Iterator i;
    avl->First(i);
    float sx = avl->GetAttrVal(i)->float_val();
    avl->Next(i);
    float sy = avl->GetAttrVal(i)->float_val();
    float dx, dy;
    viewer->ScreenToDrawing(sx, sy, dx, dy);
    AttributeValueList* navl = new AttributeValueList();
    ComValue retval(navl);
    navl->Append(new ComValue(dx));
    navl->Append(new ComValue(dy));
    push_stack(retval);
  }
}

/*****************************************************************************/

DrawingToScreenFunc::DrawingToScreenFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void DrawingToScreenFunc::execute() {
  ComValue coordsv(stack_arg(0));
  reset_stack();
  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlayViewer* viewer = ed ? (OverlayViewer*)ed->GetViewer() : nil;
  if (viewer && coordsv.is_array() && coordsv.array_len()==2) {
    AttributeValueList *avl = coordsv.array_val();
    Iterator i;
    avl->First(i);
    float dx = avl->GetAttrVal(i)->float_val();
    avl->Next(i);
    float dy = avl->GetAttrVal(i)->float_val();
    float sx, sy;
    viewer->DrawingToScreen(dx, dy, sx, sy);
    AttributeValueList* navl = new AttributeValueList();
    ComValue retval(navl);
    navl->Append(new ComValue(/* (int) */sx));
    navl->Append(new ComValue(/* (int) */sy));
    push_stack(retval);
  }
}

/*****************************************************************************/

GraphicToDrawingFunc::GraphicToDrawingFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void GraphicToDrawingFunc::execute() {
  ComValue viewval(stack_arg(0));
  ComValue coordsv(stack_arg(1));
  reset_stack();
  if (!viewval.is_object()) {
    push_stack(ComValue::nullval());
    return;
  }
  
  ComponentView* view = (ComponentView*)viewval.obj_val();
  OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
  Graphic* gr = comp ? comp->GetGraphic() : nil;

  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlayViewer* viewer = ed ? (OverlayViewer*)ed->GetViewer() : nil;
  if (gr && viewer && coordsv.is_array() && coordsv.array_len()==2) {
    AttributeValueList *avl = coordsv.array_val();
    Iterator i;
    avl->First(i);
    float gx = avl->GetAttrVal(i)->float_val();
    avl->Next(i);
    float gy = avl->GetAttrVal(i)->float_val();
    float sx, sy, dx, dy;
    viewer->GraphicToScreen(gr, gx, gy, sx, sy);
    viewer->ScreenToDrawing(sx, sy, dx, dy);
    AttributeValueList* navl = new AttributeValueList();
    ComValue retval(navl);
    navl->Append(new ComValue(dx));
    navl->Append(new ComValue(dy));
    push_stack(retval);
  }
}

/*****************************************************************************/

DrawingToGraphicFunc::DrawingToGraphicFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void DrawingToGraphicFunc::execute() {
  ComValue viewval(stack_arg(0));
  ComValue coordsv(stack_arg(1));
  reset_stack();
  if (!viewval.is_object()) {
    push_stack(ComValue::nullval());
    return;
  }

  ComponentView* view = (ComponentView*)viewval.obj_val();
  OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
  Graphic* gr = comp ? comp->GetGraphic() : nil;
  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlayViewer* viewer = ed ? (OverlayViewer*)ed->GetViewer() : nil;
  if (gr && viewer && coordsv.is_array() && coordsv.array_len()==2) {
    AttributeValueList *avl = coordsv.array_val();
    Iterator i;
    avl->First(i);
    float dx = avl->GetAttrVal(i)->float_val();
    avl->Next(i);
    float dy = avl->GetAttrVal(i)->float_val();
    float sx, sy, gx, gy;
    viewer->DrawingToScreen(dx, dy, sx, sy);
    viewer->ScreenToGraphic(sx, sy, gr, gx, gy);
    AttributeValueList* navl = new AttributeValueList();
    ComValue retval(navl);
    navl->Append(new ComValue(/* (int) */gx));
    navl->Append(new ComValue(/* (int) */gy));
    push_stack(retval);
  }
}




