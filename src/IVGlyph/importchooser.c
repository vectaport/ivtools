/*
 * Copyright (c) 1995-1996 Vectaport Inc.
 * Copyright (c) 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
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
 * ImportChooser -- select a file/filter to receive postscript
 */

#include <IVGlyph/importchooser.h>
#include <IVGlyph/textform.h>

#include <IV-look/choice.h>
#include <IV-look/dialogs.h>
#include <IV-look/fbrowser.h>
#include <IV-look/kit.h>

#include <InterViews/action.h>
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/hit.h>
#include <InterViews/input.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/scrbox.h>
#include <InterViews/style.h>
#include <InterViews/target.h>

#include <OS/directory.h>
#include <OS/string.h>

#include <iostream.h>
#include <stdio.h>
#include <string.h>

implementActionCallback(ImportChooserImpl)
implementFieldEditorCallback(ImportChooserImpl)

ImportChooser*  ImportChooser::instance_ = nil;

ImportChooser::ImportChooser(
    const String& dir, WidgetKit* kit, Style* s, OpenFileChooserAction* a,
    boolean centered_bttn, boolean by_pathname_bttn, boolean from_command_bttn,
    boolean auto_convert_bttn
) : OpenFileChooser(s) {
    impl_ = new ImportChooserImpl();
    ImportChooserImpl& ic = *(ImportChooserImpl*)impl_;
    ic.name_ = new CopyString(dir);
    ic.kit_ = kit;
    if (centered_bttn) {
      ic._centered_action = new ActionCallback(ImportChooserImpl)
	(&ic, &ImportChooserImpl::centered_callback);
      ic._cbutton = kit->check_box("centered", ic._centered_action);
      ic._cbutton->state()->set(ic._centered ? 0xffff : 0x0000,ic._centered);
    } else ic._cbutton = nil;
    if (by_pathname_bttn) {
      ic._by_pathname_action = new ActionCallback(ImportChooserImpl)
	(&ic, &ImportChooserImpl::by_pathname_callback);
      ic._fbutton = kit->check_box("save by path", ic._by_pathname_action);
      ic._fbutton->state()->set(ic._by_pathname ? 0xffff : 0x0000,ic._by_pathname); 
    } else ic._fbutton = nil;
    if (from_command_bttn) {
      ic._from_command_action = new ActionCallback(ImportChooserImpl)
	(&ic, &ImportChooserImpl::from_command_callback);
      ic._mbutton = kit->check_box("from command", ic._from_command_action);
      ic._mbutton->state()->set(ic._from_command ? 0xffff : 0x0000,ic._from_command); 
    } else ic._mbutton = nil;
    if (auto_convert_bttn) {
      ic._auto_convert_action = new ActionCallback(ImportChooserImpl)
	(&ic, &ImportChooserImpl::auto_convert_callback);
      ic._abutton = kit->check_box("auto convert", ic._auto_convert_action);
      ic._abutton->state()->set(ic._auto_convert ? 0xffff : 0x0000,ic._auto_convert); 
    } else ic._abutton = nil;
    ic.init(this, s, a);
}

#if 0
ImportChooser::ImportChooser( Style* s ) : OpenFileChooser(s) {
}
#endif
ImportChooser::~ImportChooser() {
  if (this==instance_) instance_ = nil;
}

ImportChooser& ImportChooser::instance() {
    if (!instance_) {
        Style* style;
	style = new Style(Session::instance()->style());
	style->attribute("subcaption", "Import graphic from file:");
	style->attribute("open", "Import");
	instance_ = new ImportChooser(".", WidgetKit::instance(), style);
	Resource::ref(instance_);
    }
    return *instance_;
}

void ImportChooser::instance(ImportChooser* instance) {
    if (instance_)
        Unref(instance_);
    Resource::ref(instance);
    instance_ = instance;
}

boolean ImportChooser::centered() { return ((ImportChooserImpl*)impl_)->_centered; }
boolean ImportChooser::by_pathname() { return ((ImportChooserImpl*)impl_)->_by_pathname; }
boolean ImportChooser::from_command() { return ((ImportChooserImpl*)impl_)->_from_command; }
boolean ImportChooser::auto_convert() { return ((ImportChooserImpl*)impl_)->_auto_convert; }

void ImportChooser::set_centered(boolean v) { 
  ((ImportChooserImpl*)impl_)->_centered = v;
  ImportChooserImpl& ic = *(ImportChooserImpl*)impl_;
  ic._cbutton->state()->set(ic._centered ? 0xffff : 0x0000,ic._centered); 
 }

void ImportChooser::set_by_pathname(boolean v) { 
  ((ImportChooserImpl*)impl_)->_by_pathname = v;
  ImportChooserImpl& ic = *(ImportChooserImpl*)impl_;
  ic._fbutton->state()->set(ic._by_pathname ? 0xffff : 0x0000,ic._by_pathname); 
 }

void ImportChooser::set_from_command(boolean v) { 
  ((ImportChooserImpl*)impl_)->_from_command = v;
  ImportChooserImpl& ic = *(ImportChooserImpl*)impl_;
  ic._mbutton->state()->set(ic._from_command ? 0xffff : 0x0000,ic._from_command); 
 }

void ImportChooser::set_auto_convert(boolean v) { 
  ((ImportChooserImpl*)impl_)->_auto_convert = v;
  ImportChooserImpl& ic = *(ImportChooserImpl*)impl_;
  ic._abutton->state()->set(ic._auto_convert ? 0xffff : 0x0000,ic._auto_convert); 
 }

/** class ImportChooserImpl **/

ImportChooserImpl::ImportChooserImpl () {
  _centered = true;
  _by_pathname = true;
  _from_command = false;
  _auto_convert = false;

}

void ImportChooserImpl::build() {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    kit.push_style();
    kit.style(s);
    String caption("");
    s->find_attribute("caption", caption);
    String subcaption("Enter pathname for importing graphics:");
    s->find_attribute("subcaption", subcaption);
    String open("Open");
    s->find_attribute("open", open);
    String close("Cancel");
    s->find_attribute("cancel", close);
    long rows = 10;
    s->find_attribute("rows", rows);
    const Font* f = kit.font();
    FontBoundingBox bbox;
    f->font_bbox(bbox);
    Coord height = rows * (bbox.ascent() + bbox.descent()) + 1.0;
    Coord width;
    if (!s->find_attribute("width", width)) {
	width = 16 * f->width('m') + 3.0;
    }

    Action* accept = new ActionCallback(OpenFileChooserImpl)(
	(OpenFileChooserImpl*)this, &OpenFileChooserImpl::accept_browser
    );
    Action* cancel = new ActionCallback(OpenFileChooserImpl)(
	(OpenFileChooserImpl*)this, &OpenFileChooserImpl::cancel_browser
    );
    delete editor_;
    editor_ = DialogKit::instance()->field_editor(
	*dir_->path(), s,
	new FieldEditorCallback(ImportChooserImpl)(
	    this, &ImportChooserImpl::accept_editor,
	    &ImportChooserImpl::cancel_editor
	)
    );
    fbrowser_ = new FileBrowser(kit_, accept, cancel);

    fchooser_->remove_all_input_handlers();
    fchooser_->append_input_handler(editor_);
    fchooser_->append_input_handler(fbrowser_);

    caption_ = new ObservableText(caption.string());
    captionview_ = new TextObserver(caption_, "");
    subcaption_ = new ObservableText(subcaption.string());
    subcaptionview_ = new TextObserver(subcaption_, "");

    Glyph* g = layout.vbox();
    g->append(layout.rmargin(subcaptionview_, 5.0, fil, 0.0));
    g->append(layout.rmargin(captionview_, 5.0, fil, 0.0));
    g->append(layout.vglue(5.0, 0.0, 2.0));
    g->append(editor_);
    g->append(layout.vglue(15.0, 0.0, 12.0));
    g->append(
	layout.hbox(
	    layout.vcenter(
		kit.inset_frame(
		    layout.margin(
			layout.natural_span(fbrowser_, width, height), 1.0
		    )
		),
		1.0
	    ),
	    layout.hspace(4.0),
	    kit.vscroll_bar(fbrowser_->adjustable())
	)
    );
    g->append(layout.vspace(10.0));
    if (s->value_is_on("filter")) {
	FieldEditorAction* action = new FieldEditorCallback(OpenFileChooserImpl)(
	    this, &OpenFileChooserImpl::accept_filter, nil
	);
	filter_ = add_filter(
	    s, "filterPattern", "", "filterCaption", "Filter:", g, action
	);
	if (s->value_is_on("directoryFilter")) {
	    directory_filter_ = add_filter(
		s, "directoryFilterPattern", "",
		"directoryFilterCaption", "Directory Filter:", g, action
	    );
	} else {
	    directory_filter_ = nil;
	}
    } else {
	filter_ = nil;
	directory_filter_ = nil;
    }

    Glyph* vb = layout.vbox();
    int from_left = 100;
    if (_cbutton && _fbutton && _mbutton && _abutton) {
      vb->append(
		 layout.hbox(
			     layout.hglue(10.0),
			     layout.hfixed(_cbutton, from_left),
			     layout.hglue(10.0),
			     layout.hfixed(_fbutton, from_left),
			     layout.hglue(10.0)
			     ));
      vb->append(layout.vspace(10.0));
      vb->append(
		 layout.hbox(
			     layout.hglue(10.0),
			     layout.hfixed(_mbutton, from_left),
			     layout.hglue(10.0),
			     layout.hfixed(_abutton, from_left),
			     layout.hglue(10.0)
			     ));
      vb->append(layout.vspace(10.0));
    } else {
      if (_cbutton) {
	vb->append(
		   layout.hbox(
			       layout.hglue(10.0),
			       layout.hfixed(_cbutton, from_left),
			       layout.hglue(10.0)
			       ));
	vb->append(layout.vspace(10.0));
      }
      if (_mbutton) {
	vb->append(
		   layout.hbox(
			       layout.hglue(10.0),
			       layout.hfixed(_mbutton, from_left),
			       layout.hglue(10.0)
			       ));
	vb->append(layout.vspace(10.0));
      }
      if (_fbutton) {
	vb->append(
		   layout.hbox(
			       layout.hglue(10.0),
			       layout.hfixed(_fbutton, from_left),
			       layout.hglue(10.0)
			       ));
	vb->append(layout.vspace(10.0));
      }
      if (_abutton) {
	vb->append(
		   layout.hbox(
			       layout.hglue(10.0),
			       layout.hfixed(_abutton, from_left),
			       layout.hglue(10.0)
			       ));
	vb->append(layout.vspace(10.0));
      }
    }

    vb->append(layout.vspace(5.0));
    vb->append(
	       layout.hbox(
			   layout.hglue(10.0),
			   layout.vcenter(kit.default_button(open, accept)),
			   layout.hglue(10.0, 0.0, 5.0),
			   layout.vcenter(kit.push_button(close, cancel)),
			   layout.hglue(10.0)
			   )
	       );
    
    g->append(vb);
    fchooser_->body(
	layout.back(
	    layout.vcenter(kit.outset_frame(layout.margin(g, 5.0)), 1.0),
	    new Target(nil, TargetPrimitiveHit)
	)
    );
    fchooser_->focus(editor_);
    kit.pop_style();
    load();
}

void ImportChooserImpl::centered_callback() {
    _centered = !_centered;
}
   
void ImportChooserImpl::by_pathname_callback() {
    _by_pathname = !_by_pathname;
}

void ImportChooserImpl::from_command_callback() {
    _from_command = !_from_command;
}

void ImportChooserImpl::auto_convert_callback() {
    _auto_convert = !_auto_convert;
}

void ImportChooserImpl::accept_editor(FieldEditor* e) {
  const String* fstring = e->text();
  boolean urlflag = OpenFileChooser::urltest(fstring->string());
  if (!_from_command && !urlflag) {
    String* path = Directory::canonical(*fstring);
    e->field(*path);
    if (chdir(*path)) {
      /* chdir has copied the string */
      delete path;
    } else {
      selected_ = path;
      fchooser_->dismiss(true);
      e->select(path->rindex('/') + 1, path->length());
    } 
  } else {
    selected_ = e->text();
    fchooser_->dismiss(true);
    e->select(0, selected_->length());
  }
}

void ImportChooserImpl::cancel_editor(FieldEditor*) {
    fchooser_->dismiss(false);
}

