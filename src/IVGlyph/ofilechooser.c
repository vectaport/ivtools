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
 * OpenFileChooser -- select a file
 */

#include <IVGlyph/ofilechooser.h>
#include <IVGlyph/textform.h>

#include <IV-look/choice.h>
#include <IV-look/dialogs.h>
#include <IV-look/fbrowser.h>
#include <IV-look/kit.h>

#include <InterViews/action.h>
#include <InterViews/cursor.h>
#include <InterViews/dialog.h>
#include <InterViews/display.h>
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/handler.h>
#include <InterViews/hit.h>
#include <InterViews/input.h>
#include <InterViews/layout.h>
#include <InterViews/scrbox.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <InterViews/window.h>

#include <OS/string.h>

#include <iostream.h>
#include <stdio.h>
#include <string.h>


/* class DialogHandler -- helper for class Dialog */

class DialogHandler : public Handler {
public:
    DialogHandler(Dialog*);
    virtual ~DialogHandler();

    virtual boolean event(Event&);
private:
    Dialog* dialog_;
};

implementActionCallback(OpenFileChooserImpl)
implementFieldEditorCallback(OpenFileChooserImpl)

OpenFileChooser::OpenFileChooser(
    const String& dir, WidgetKit* kit, Style* s, OpenFileChooserAction* a
) : Dialog(nil, s) {
    impl_ = new OpenFileChooserImpl;
    OpenFileChooserImpl& fc = *impl_;
    fc.name_ = new CopyString(dir);
    fc.kit_ = kit;
    fc.init(this, s, a);
    _t = nil;
}

OpenFileChooser::OpenFileChooser( Style* s ) : Dialog(nil, s) {
    _t = nil;
}

OpenFileChooser::~OpenFileChooser() {
    impl_->free();
    delete impl_;
}

const String* OpenFileChooser::selected() const {
    return impl_->selected_;
}

void OpenFileChooser::reread() {
    OpenFileChooserImpl& fc = *impl_;
    if (!fc.chdir(*fc.dir_->path())) {
	/* should generate an error message */
    }
}

void OpenFileChooser::dismiss(boolean accept) {
    Dialog::dismiss(accept);
    OpenFileChooserImpl& fc = *impl_;
    if (fc.action_ != nil) {
	fc.action_->execute(this, accept);
    }
}

TransientWindow* OpenFileChooser::twindow() {
    if (!_t || !_t->is_mapped())
	return nil;
    else
	return _t;
}

boolean OpenFileChooser::post_for_aligned(Window* w, float x_align, float y_align) {
    if (!_t) {
	_t = new TransientWindow(this);
	_t->style(new Style(style()));
	_t->transient_for(w);
	_t->wm_delete(new DialogHandler(this));
	_t->place(w->left() + 0.5 * w->width(), w->bottom() + 0.5 * w->height());
	_t->align(x_align, y_align);
	_t->map();
    }
    boolean b = run();
    _t->display()->sync();
    return b;
}

void OpenFileChooser::unmap() {
    _t->unmap();
    _t->display()->sync();
    delete _t;
    _t = nil;
}

boolean OpenFileChooser::saveas_chooser() { return false; }

void OpenFileChooser::updatecaption() { 
  if (impl_) impl_->updatecaption(); 
}

boolean OpenFileChooser::url_use_ok() {
  return bincheck("ivdl") || bincheck("w3c") || bincheck("curl") || bincheck("wget");
}

boolean OpenFileChooser::urltest(const char* buf) {
  if (!buf) return false;
  static boolean file_url_ok = url_use_ok();
  return 
    strncasecmp("http://", buf, 7)==0 || 
    strncasecmp("ftp://", buf, 6)==0 ||
    file_url_ok && strncasecmp("file:/", buf, 6)==0;
}

int OpenFileChooser::bintest(const char* command) {
  char combuf[BUFSIZ];
  sprintf( combuf, "wr=`which %s`; echo $wr", command );
  FILE* fptr = popen(combuf, "r");
  char testbuf[BUFSIZ];	
  fgets(testbuf, BUFSIZ, fptr);  
  pclose(fptr);
  if (strncmp(testbuf+strlen(testbuf)-strlen(command)-1, 
	      command, strlen(command)) != 0) {
    return -1;
  }
  return 0;
}

boolean OpenFileChooser::bincheck(const char* command) {
  int status = bintest(command);
  return !status;
}

/** class OpenFileChooserImpl **/

void OpenFileChooserImpl::init(
    OpenFileChooser* chooser, Style* s, OpenFileChooserAction* a
) {
    fchooser_ = chooser;
    fbrowser_ = nil;
    editor_ = nil;
    filter_ = nil;
    directory_filter_ = nil;
    filter_map_ = nil;
    dir_ = Directory::open(*name_);
    if (dir_ == nil) {
	dir_ = Directory::current();
	/* and what if we can't read the current directory? */
    }
    Resource::ref(a);
    action_ = a;
    style_ = new Style(s);
    Resource::ref(style_);
    style_->alias("OpenFileChooser");
    style_->alias("Dialog");
    update_ = new ActionCallback(OpenFileChooserImpl)(
	this, &OpenFileChooserImpl::updatecaption
    );
    style_->add_trigger_any(update_);
    build();
}

void OpenFileChooserImpl::free() {
    delete name_;
    delete dir_;
    delete filter_map_;
    Resource::unref(action_);
    style_->remove_trigger_any(update_);
    Resource::unref(style_);
}

void OpenFileChooserImpl::build() {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    kit.push_style();
    kit.style(s);
    String caption("");
    s->find_attribute("caption", caption);
    String subcaption("Open file:");
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
	this, &OpenFileChooserImpl::accept_browser
    );
    Action* cancel = new ActionCallback(OpenFileChooserImpl)(
	this, &OpenFileChooserImpl::cancel_browser
    );
    if (editor_ == nil) {
	editor_ = DialogKit::instance()->field_editor(
	    *dir_->path(), s,
	    new FieldEditorCallback(OpenFileChooserImpl)(
		this, &OpenFileChooserImpl::accept_editor,
		&OpenFileChooserImpl::cancel_editor
	    )
	);
    }
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
    g->append(	
	layout.hbox(
	    layout.hglue(10.0),
	    layout.vcenter(kit.default_button(open, accept)),
	    layout.hglue(10.0, 0.0, 5.0),
	    layout.vcenter(kit.push_button(close, cancel)),
	    layout.hglue(10.0)
	    )
        );

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

void OpenFileChooserImpl::updatecaption() {
    Style* s = style_;
    String caption("");
    s->find_attribute("caption", caption);
    caption_->textvalue(caption.string());
    caption_->notify();
    String subcaption("");
    s->find_attribute("subcaption", subcaption);
    subcaption_->textvalue(subcaption.string());
    subcaption_->notify();
}

void OpenFileChooserImpl::clear() {
    Browser& b = *fbrowser_;
    b.select(-1);
    GlyphIndex n = b.count();
    for (GlyphIndex i = 0; i < n; i++) {
	b.remove_selectable(0);
	b.remove(0);
    }
}

void OpenFileChooserImpl::load() {
    Directory& d = *dir_;
    FileBrowser& b = *fbrowser_;
    WidgetKit& kit = *kit_;
    kit.push_style();
    kit.style(style_);
    const LayoutKit& layout = *LayoutKit::instance();
    int dircount = d.count();
    delete filter_map_;
    int* index = new int[dircount];
    filter_map_ = index;
    for (int i = 0; i < dircount; i++) {
	const String& f = *d.name(i);
	boolean is_dir = d.is_directory(i);
	if ((is_dir && filtered(f, directory_filter_)) ||
	    (!is_dir && filtered(f, filter_))
	) {
	    Glyph* name = kit.label(f);
	    if (is_dir) {
		name = layout.hbox(name, kit.label("/"));
	    }
	    Glyph* label = new Target(
		layout.h_margin(name, 3.0, 0.0, 0.0, 15.0, fil, 0.0),
		TargetPrimitiveHit
	    );
	    TelltaleState* t = new TelltaleState(TelltaleState::is_enabled);
	    b.append_selectable(t);
	    b.append(new ChoiceItem(t, label, kit.bright_inset_frame(label)));
	    *index++ = i;
	}
    }
    b.refresh();
    kit.pop_style();
}

FieldEditor* OpenFileChooserImpl::add_filter(
    Style* s,
    const char* pattern_attribute, const char* default_pattern,
    const char* caption_attribute, const char* default_caption,
    Glyph* body, FieldEditorAction* action
) {
    String pattern(default_pattern);
    s->find_attribute(pattern_attribute, pattern);
    String caption(default_caption);
    s->find_attribute(caption_attribute, caption);
    FieldEditor* e = DialogKit::instance()->field_editor(pattern, s, action);
    fchooser_->append_input_handler(e);
    WidgetKit& kit = *kit_;
    LayoutKit& layout = *LayoutKit::instance();
    body->append(
	layout.hbox(
	    layout.vcenter(kit.fancy_label(caption), 0.5),
	    layout.hspace(2.0),
	    layout.vcenter(e, 0.5)
	)
    );
    body->append(layout.vspace(10.0));
    return e;
}

boolean OpenFileChooserImpl::filtered(const String& name, FieldEditor* e) {
    if (e == nil) {
	return true;
    }
    const String* s = e->text();
    if (s == nil || s->length() == 0) {
	return true;
    }
    return s == nil || s->length() == 0 || Directory::match(name, *s);
}

void OpenFileChooserImpl::accept_browser() {
    int i = int(fbrowser_->selected());
    if (i == -1) {
	accept_editor(editor_);
	return;
    }
    i = filter_map_[i];
    const String& path = *dir_->path();
    const String& name = *dir_->name(i);
    int length = path.length() + name.length();
    char* tmp = new char[length + 1];
    sprintf(
	tmp, "%.*s%.*s",
	path.length(), path.string(), name.length(), name.string()
    );
    editor_->field(tmp);
    selected_ = editor_->text();
    if (dir_->is_directory(i)) {
	if (chdir(String(tmp, length))) {
	    editor_->field(*dir_->path());
	    fchooser_->focus(editor_);
	} else {
	    /* should generate an error message */
	}
    } else {
	fchooser_->dismiss(true);
    }
    delete tmp;
}

void OpenFileChooserImpl::cancel_browser() {
    selected_ = nil;
    fchooser_->dismiss(false);
}

void OpenFileChooserImpl::accept_editor(FieldEditor* e) {
    boolean urlflag = OpenFileChooser::urltest(e->text()->string());
    String* path = (String *) (urlflag ? e->text() : Directory::canonical(*e->text()));
    e->field(*path);
    if (!urlflag && chdir(*path)) {
	/* chdir has copied the string */
	delete path;
    } else {
	selected_ = path;
	fchooser_->dismiss(true);
	e->select(path->rindex('/') + 1, path->length());
    }
}

void OpenFileChooserImpl::cancel_editor(FieldEditor*) {
    fchooser_->dismiss(false);
}

void OpenFileChooserImpl::accept_filter(FieldEditor*) {
    clear();
    load();
}

boolean OpenFileChooserImpl::chdir(const String& name) {
    Directory* d = Directory::open(name);
    if (d != nil) {
	dir_->close();
	delete dir_;
	dir_ = d;
	clear();
	load();
	return true;
    }
    return false;
}

/** class OpenFileChooserAction **/

OpenFileChooserAction::OpenFileChooserAction() { }
OpenFileChooserAction::~OpenFileChooserAction() { }
void OpenFileChooserAction::execute(OpenFileChooser*, boolean) { }
