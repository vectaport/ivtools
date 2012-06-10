/*
 * Copyright (c) 1997,1999 Vectaport Inc.
 * Copyright (c) 1994-1996 Vectaport Inc., Cartoactive Systems
 * Copyright (c) 1993 David B. Hollenbeck
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
 * RemoteConnectDialog related classes
 */

#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/rcdialog.h>

#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>

#include <IVGlyph/gdialogs.h>
#include <IVGlyph/stredit.h>
#include <IVGlyph/textedit.h>
#include <IVGlyph/textview.h>

#include <IV-look/dialogs.h>
#include <IV-look/kit.h>

#include <InterViews/font.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/style.h>
#include <InterViews/window.h>

#include <stdio.h>
#include <stdlib.h>
using namespace std;
#include <vector>

/*****************************************************************************/

RemoteConnectPopupAction::RemoteConnectPopupAction(Editor* editor) : Action() {
  _editor = editor;
}

void RemoteConnectPopupAction::execute() {
#if 0
  StrEditDialog::accept_custom("Connect");
  StrEditDialog::cancel_custom("Close");
  StrEditDialog* dialog =
    StrEditDialog::map
    (_editor->GetWindow(), 
     "Enter host name to connect:                  ", "localhost", nil, nil, true);

  StrEditDialog::action_custom(new RemoteConnectAction(dialog), nil);
#else
    WidgetKit& kit = *WidgetKit::instance();
    ConnectionsDialog* dlog = new ConnectionsDialog(_editor, ((DrawServ*)unidraw)->linklist(), &kit, kit.style());
    dlog->ref();
    dlog->map_for(_editor->GetWindow());
#endif
}

/*****************************************************************************/

RemoteConnectAction::RemoteConnectAction(StrEditDialog* dialog) : Action() {
  _dialog = dialog;
}

void RemoteConnectAction::execute() {

  if (_dialog && _dialog->text()) 
    ((DrawServ*)unidraw)->linkup(_dialog->text(), 20002, 0);

}

/*****************************************************************************/

declareActionCallback(ConnectionsDialogImpl)
implementActionCallback(ConnectionsDialogImpl)
declareFieldEditorCallback(ConnectionsDialogImpl)
implementFieldEditorCallback(ConnectionsDialogImpl)

class ConnectionsDialogImpl : public Observer {
public:
  virtual void update(Observable* obs);
private:
friend class ConnectionsDialog;

    WidgetKit* kit_;
    Style* style_;
    ConnectionsDialog* dialog_;
    DrawLinkList* list_;
    Patch* patch_;
    EivTextEditor* ete_;
    FieldEditor* hostfe_;
    FieldEditor* portfe_;

    void init(DrawLinkList*, ConnectionsDialog*, Style*);
    void free();
    void build(DrawLinkList*);
    void connect();
    void disconnect();
    void close();
    const char* value();
    void value(const char*, boolean update);
    void clear();
    Patch* table_patch(DrawLinkList*);
    void fe_add(FieldEditor*);
    void fe_clr(FieldEditor*);
    void update_text(boolean);
};

ConnectionsDialog::ConnectionsDialog(Editor* ed, DrawLinkList* list, WidgetKit* kit, Style* s) 
: Dialog(nil, s)
{
  impl_ = new ConnectionsDialogImpl;
  ConnectionsDialogImpl& adi = *impl_;
  adi.kit_ = kit;
  adi.init(list, this, s);
  _ed = ed;
}

ConnectionsDialog::~ConnectionsDialog() {
  impl_->free();
  delete impl_;
}

/** class ConnectionsDialogImpl **/

void ConnectionsDialogImpl::init(DrawLinkList* list, ConnectionsDialog* p, Style* s) {

  list->attach(this);

  dialog_ = p;
  style_ = s;
  list_ = list;

  DialogKit& dk = *DialogKit::instance();
  WidgetKit& wk = *WidgetKit::instance();

  hostfe_ = dk.field_editor("", wk.style(), 
			    new FieldEditorCallback(ConnectionsDialogImpl)
			    (this, &ConnectionsDialogImpl::fe_add, 
			     &ConnectionsDialogImpl::fe_clr));
  portfe_ = dk.field_editor("", wk.style(),
			    new FieldEditorCallback(ConnectionsDialogImpl)
			    (this, &ConnectionsDialogImpl::fe_add, 
			     &ConnectionsDialogImpl::fe_clr));
  s->attribute("rows", "10");
  s->attribute("columns", "30");
  ete_ = new EivTextEditor(s, false);
  ete_->ref();
  ete_->textview()->disable_caret();

  build(list_);
}

void ConnectionsDialogImpl::free() {
  ete_->unref();
}

void ConnectionsDialogImpl::build(DrawLinkList* list) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    const DialogKit& dialog = *DialogKit::instance();
    Style* s = style_;
    String caption("DrawLink Connections");
#if 0
    String connect_str("Connect");
#endif
    String close_str("Close");

#if 0
    Action* connect = new ActionCallback(ConnectionsDialogImpl)(
	this, &ConnectionsDialogImpl::connect
    );
#endif
    Action* close = new ActionCallback(ConnectionsDialogImpl)(
	this, &ConnectionsDialogImpl::close
    );

    Glyph* g = layout.vbox(
	layout.hcenter(kit.fancy_label(caption)),
	layout.vspace(15.0),
	layout.hcenter(layout.hbox(
	    layout.hglue(10.0),
	    layout.vcenter(kit.push_button(close_str, close)),
#if 0
	    layout.hglue(10.0, 0.0, 5.0),
	    layout.vcenter(kit.push_button(connect_str, connect)),
#endif
	    layout.hglue(10.0)
	)),
	layout.vspace(15.0),
	layout.hcenter(table_patch(list)),
	layout.vglue(1)
    );
    dialog_->body(
	kit.outset_frame(layout.margin(g, 5.0))
    );
    dialog_->append_input_handler(hostfe_);
    dialog_->append_input_handler(portfe_);
    dialog_->focus(hostfe_);
}

void ConnectionsDialogImpl::connect() {
  const String* hoststr = hostfe_->text();
  const String* portstr = portfe_->text();

  /* remove terminating spaces */
  int hostlen = strlen(hoststr->string())+1;
  int portlen = strlen(portstr->string())+1;
  char hostbuf[hostlen];
  char portbuf[portlen];
  strcpy(hostbuf, hoststr->string());
  strcpy(portbuf, portstr->string());
  char *ptr;
  ptr = hostbuf + hostlen - 2;
  while (*ptr == ' ' && ptr >= (char*)hostbuf) *ptr-- = '\0';
  ptr = portbuf + portlen - 2;
  while (*ptr == ' ' && ptr >= (char*)portbuf) *ptr-- = '\0';

  if (strlen(hostbuf) > 0) {
    int portnum=20002;
    if (strlen(portbuf) > 0) portnum = atoi(portbuf);

    if(((DrawServ*)unidraw)->selftest(hostbuf, portnum)) {
      char buffer[BUFSIZ];
      snprintf(buffer, BUFSIZ, "%s:%d", hostbuf, portnum);
      GAcknowledgeDialog::map(dialog_->GetEditor()->GetWindow(), "Can't connect to self", buffer, "Can't connect to self");
      return;
    }

    if(((DrawServ*)unidraw)->linkup(hostbuf, portnum, 0)==nil) {
      char buffer[BUFSIZ];
      snprintf(buffer, BUFSIZ, "%s:%d", hostbuf, portnum);
      GAcknowledgeDialog::map(dialog_->GetEditor()->GetWindow(), "Connection refused", buffer, "Connection refused");
    }
  }
}

void ConnectionsDialogImpl::disconnect() {
  const String* hoststr = hostfe_->text();
  const String* portstr = portfe_->text();

  /* remove terminating spaces */
  int hostlen = strlen(hoststr->string())+1;
  int portlen = strlen(portstr->string())+1;
  char hostbuf[hostlen];
  char portbuf[portlen];
  strcpy(hostbuf, hoststr->string());
  strcpy(portbuf, portstr->string());
  char *ptr;
  ptr = hostbuf + hostlen - 2;
  while (*ptr == ' ' && ptr >= (char*)hostbuf) *ptr-- = '\0';
  ptr = portbuf + portlen - 2;
  while (*ptr == ' ' && ptr >= (char*)portbuf) *ptr-- = '\0';

  if (strlen(hostbuf) > 0) {
    int portnum=20002;
    if (strlen(portbuf) > 0) portnum = atoi(portbuf);
    DrawLink* link = ((DrawServ*)unidraw)->linkget(hostbuf, portnum);
    if (link) 
      ((DrawServ*)unidraw)->linkdown(link);
  }
}

void ConnectionsDialogImpl::close() {
    dialog_->dismiss(false);
}

Patch* ConnectionsDialogImpl::table_patch(DrawLinkList* list) {

  DialogKit& dk = *DialogKit::instance();
  WidgetKit& wk = *WidgetKit::instance();
  const LayoutKit& lk = *LayoutKit::instance();
  PolyGlyph* mainglyph = lk.vbox();
  Glyph* glu = lk.vspace(5);
  PolyGlyph* _namebox = lk.vbox();
  PolyGlyph* _valbox = lk.vbox();
  InputHandler* ih = new InputHandler(nil, wk.style());
  Coord wid = wk.font()->width("MMMMMMMMMMMMMMM", 15);
  
  Action* conaction = new ActionCallback(ConnectionsDialogImpl)
    (this, &ConnectionsDialogImpl::connect);
  Button* conbutton = wk.push_button("Connect", conaction);
  Action* disaction = new ActionCallback(ConnectionsDialogImpl)
    (this, &ConnectionsDialogImpl::disconnect);
  Button* disbutton = wk.push_button("Disconnect", disaction);
  mainglyph->append
    (lk.hcenter
     (lk.hbox
      (lk.vcenter(conbutton),
       lk.hspace(10),
       lk.vcenter(disbutton)
       )
      )
     );
  mainglyph->append(lk.vspace(10));
  mainglyph->append
    (lk.hcenter
     (lk.hbox
      (lk.vcenter(lk.hfixed(wk.label("Host"), wid)),
       lk.hspace(10),
       lk.vcenter(lk.hfixed(wk.label("Port"), wid))
       )
      )
     );
  mainglyph->append(lk.vspace(2));
  mainglyph->append
    (lk.hcenter
     (lk.hbox
      (lk.vcenter(lk.hfixed(hostfe_, wid)),
       lk.hspace(10),
       lk.vcenter(lk.hfixed(portfe_, wid))
       )
      )
     );
  mainglyph->append(lk.vspace(15));
  update_text(false);
  mainglyph->append(lk.hcenter(lk.hspace(300)));
  mainglyph->append(lk.hcenter(ete_));
  ih->body(wk.outset_frame(lk.margin(mainglyph, 10)));
  
  patch_ = new Patch(nil);
  patch_->body(ih);
  return patch_;
}

void ConnectionsDialogImpl::fe_add(FieldEditor* fe) {
#if 0
    const String* txt = _namefe->text();
    if (txt->length() > 0) {
	char* buf = new char[strlen(_valfe->text()->string())+2];
	sprintf(buf, "%s\n", _valfe->text()->string());
	_list->add_attr(txt->string(), ParamList::lexscan()->get_attr(buf, strlen(buf)));
	update_text(true);
    }
#endif
}

void ConnectionsDialogImpl::fe_clr(FieldEditor* fe) {
    fe->field("");
}

void ConnectionsDialogImpl::update(Observable* obs) {
  fprintf(stderr, "DrawLinkList has been modified\n");
  update_text(true);
}

void ConnectionsDialogImpl::update_text(boolean update) {
  if (!dialog_->mapped() && update) return;
  Iterator i;
  vector <char> vbuf;
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, "Host                            Port    State\n");
  for(int i=0; i<strlen(buf); i++) vbuf.push_back(buf[i]);
  snprintf(buf, BUFSIZ, "---------------------------------------------\n");
  for(int i=0; i<strlen(buf); i++) vbuf.push_back(buf[i]);
  for (list_->First(i); !list_->Done(i); list_->Next(i)) {
    DrawLink* link = list_->GetDrawLink(i);
    snprintf(buf, BUFSIZ, "%-30.30s  %-6d  %s\n", 
	     link->hostname(), link->portnum(), DrawLink::state_string(link->state()));
    for(int i=0; i<strlen(buf); i++) vbuf.push_back(buf[i]);
  }
  vbuf.push_back('\0');
  ete_->text(&vbuf[0] ? &vbuf[0] : "", update);
}
