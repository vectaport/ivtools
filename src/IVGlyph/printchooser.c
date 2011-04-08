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
 * PrintChooser -- select a file/filter to receive postscript
 */

#include <IVGlyph/printchooser.h>
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
#include <InterViews/scrbox.h>
#include <InterViews/style.h>
#include <InterViews/target.h>

#include <OS/directory.h>
#include <OS/string.h>

#include <iostream.h>
#include <stdio.h>

implementActionCallback(PrintChooserImpl)
implementFieldEditorCallback(PrintChooserImpl)

PrintChooser::PrintChooser(
    const String& dir, WidgetKit* kit, Style* s, OpenFileChooserAction* a
) : OpenFileChooser(s) {
    impl_ = new PrintChooserImpl;
    PrintChooserImpl& fc = *(PrintChooserImpl*)impl_;
    fc.name_ = new CopyString(dir);
    fc.kit_ = kit;
    fc.init(this, s, a);
}

PrintChooser::PrintChooser( Style* s ) : OpenFileChooser(s) {
}

boolean PrintChooser::to_printer() { return ((PrintChooserImpl*)impl_)->_to_printer; }
boolean PrintChooser::idraw_format() { return ((PrintChooserImpl*)impl_)->_idraw_format; }

/** class PrintChooserImpl **/

PrintChooserImpl::PrintChooserImpl() {
    _to_printer = false;
    _idraw_format = false;
}

void PrintChooserImpl::build() {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    kit.push_style();
    kit.style(s);
    String caption("");
    s->find_attribute("caption", caption);
    String subcaption("Enter pathname for saving postscript:");
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
    Action* printer = new ActionCallback(PrintChooserImpl)(
	this, &PrintChooserImpl::to_printer_callback
    );
    Action* idraw = new ActionCallback(PrintChooserImpl)(
	this, &PrintChooserImpl::idraw_format_callback
    );
    if (editor_ == nil) {
	editor_ = DialogKit::instance()->field_editor(
	    *dir_->path(), s,
	    new FieldEditorCallback(PrintChooserImpl)(
		this, &PrintChooserImpl::accept_editor,
		&PrintChooserImpl::cancel_editor
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
	layout.vbox(
	    layout.hbox(
		layout.hglue(10.0),
		layout.vcenter(kit.check_box("pipe to command", printer)),
		layout.hspace(10.0),
		layout.vcenter(kit.check_box("idraw format", idraw)),
		layout.hglue(10.0)
	    ),
	    layout.vspace(15.0),
	    layout.hbox(
		layout.hglue(10.0),
		layout.vcenter(kit.default_button(open, accept)),
		layout.hglue(10.0, 0.0, 5.0),
		layout.vcenter(kit.push_button(close, cancel)),
		layout.hglue(10.0)
	    )
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

void PrintChooserImpl::to_printer_callback() {
    _to_printer = !_to_printer;
    editor_->field(_to_printer ? "ghostview" : "./" );
}

void PrintChooserImpl::idraw_format_callback() {
    _idraw_format = !_idraw_format;
}

void PrintChooserImpl::accept_editor(FieldEditor* e) {
    if (_to_printer) {
	selected_ = e->text();
	fchooser_->dismiss(true);
    } else {
	String* path =  Directory::canonical(*e->text());
	e->field(*path);
	if (chdir(*path)) {
	    /* chdir has copied the string */
	    delete path;
	} else {
	    selected_ = path;
	    fchooser_->dismiss(true);
	    e->select(path->rindex('/') + 1, path->length());
	}
    }
}

void PrintChooserImpl::cancel_editor(FieldEditor* fe) {
    OpenFileChooserImpl::cancel_editor(fe);
}

