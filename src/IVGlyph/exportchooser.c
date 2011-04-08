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
 * ExportChooser -- select a file/command to export graphics to
 */

#include <IVGlyph/exportchooser.h>
#include <IVGlyph/enumform.h>
#include <IVGlyph/textform.h>

#include <Unidraw/globals.h>

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

implementActionCallback(ExportChooserImpl)

ExportChooser::ExportChooser(
    const String& dir, WidgetKit* kit, Style* s, const char** formats,
    int nformats, const char** commands, OpenFileChooserAction* a, 
    boolean execute_flag_button, boolean by_pathname_button
) : PrintChooser(s) {
    impl_ = new ExportChooserImpl(execute_flag_button, by_pathname_button);
    set_formats(formats, nformats, commands);
    ExportChooserImpl& fc = *(ExportChooserImpl*)impl_;
    fc.name_ = new CopyString(dir);
    fc.kit_ = kit;
    fc.init(this, s, a);
}

void ExportChooser::set_formats(const char** formats, int nformats, const char** commands) {
    ((ExportChooserImpl*)impl_)->set_formats(formats, nformats, commands); 
}

const char* ExportChooser::format() { 
    return ((ExportChooserImpl*)impl_)->_obse
	? ((ExportChooserImpl*)impl_)->_obse->labelvalue().string()
	: "idraw";
}

boolean ExportChooser::idraw_format() { 
    return ((ExportChooserImpl*)impl_)->_obse 
	? strncmp(((ExportChooserImpl*)impl_)->_obse->labelvalue().string(), "idraw", 5) == 0 
	: false;
}

boolean ExportChooser::postscript_format() { 
    return ((ExportChooserImpl*)impl_)->_obse 
	? strncmp(((ExportChooserImpl*)impl_)->_obse->labelvalue().string(), "idraw", 5) == 0  || strcasecmp(((ExportChooserImpl*)impl_)->_obse->labelvalue().string(), "EPS") == 0 
	: false;
}

boolean ExportChooser::svg_format() { 
    return ((ExportChooserImpl*)impl_)->_obse 
	? strncmp(((ExportChooserImpl*)impl_)->_obse->labelvalue().string(), "SVG", 3) == 0 
	: false;
}

boolean ExportChooser::execute_flag() { 
    return ((ExportChooserImpl*)impl_)->_execute_flag;
}
boolean ExportChooser::execute_flag_button() { 
    return ((ExportChooserImpl*)impl_)->_execute_flag_button;
}
boolean ExportChooser::by_pathname_flag() { 
    return ((ExportChooserImpl*)impl_)->_by_pathname_flag;
}
boolean ExportChooser::by_pathname_flag_button() { 
    return ((ExportChooserImpl*)impl_)->_by_pathname_flag_button;
}


/** class ExportChooserImpl **/

ExportChooserImpl::ExportChooserImpl(boolean execute_flag_button, boolean by_pathname_button) {
    _nformats = 0;
    _formats = nil;
    _commands = nil;
    _obse = nil;
    _editor = nil;
    _execute_flag_button = execute_flag_button;
    _execute_flag = false;
    _by_pathname_flag_button = by_pathname_button;
    _by_pathname_flag = true;
}

void ExportChooserImpl::build() {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    kit.push_style();
    kit.style(s);
    String caption("");
    s->find_attribute("caption", caption);
    String subcaption("Enter pathname for exporting selected graphics:");
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
    Action* printer = nil;
    if (_execute_flag_button) {
	printer = new ActionCallback(ExportChooserImpl)(
	    this, &ExportChooserImpl::to_printer_callback
	);
    }
    Action* by_pathname = new ActionCallback(ExportChooserImpl)(
	this, &ExportChooserImpl::by_pathname_callback
    );
    if (editor_ == nil) {
	editor_ = DialogKit::instance()->field_editor(
	    *dir_->path(), s,
	    new FieldEditorCallback(PrintChooserImpl)(
		(PrintChooserImpl*)this, &PrintChooserImpl::accept_editor,
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
   Glyph* hbox = layout.hbox();
    Button* exec_bttn = nil;
    if (_execute_flag_button || _by_pathname_flag_button) 
	hbox->append(layout.hglue(5.0));
    if (_execute_flag_button) {
	exec_bttn = kit.check_box("to command", printer);
	exec_bttn->state()->set(_execute_flag ? 0xffff : 0x0000, _execute_flag);
	hbox->append(layout.vcenter(exec_bttn));
	hbox->append(layout.hglue(5.0));
    }
    Button* byfn_bttn = nil;
    if (_by_pathname_flag_button) {
	byfn_bttn = kit.check_box("save by path", by_pathname);
	byfn_bttn->state()->set(_by_pathname_flag ? 0xffff : 0x0000, _by_pathname_flag);
	hbox->append(layout.vcenter(byfn_bttn));
	hbox->append(layout.hglue(5.0));
    }
    Glyph* vbox = layout.vbox();
    if (_execute_flag_button || _by_pathname_flag_button) {
	vbox->append(hbox);
	vbox->append(layout.vspace(5.0));
    }
    vbox->append(
	layout.hbox(
	    layout.hglue(5.0), 
	    layout.vcenter(_editor),
	    layout.hglue(5.0)));
    vbox->append(
	layout.vspace(15.0));
    vbox->append(
	layout.hbox(
	    layout.hglue(10.0),
	    layout.vcenter(kit.default_button(open, accept)),
	    layout.hglue(10.0, 0.0, 5.0),
	    layout.vcenter(kit.push_button(close, cancel)),
	    layout.hglue(10.0)));
    g->append(vbox);

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

const char* ExportChooserImpl::format() { 
    return _obse ? _obse->labelvalue().string() : nil; 
}

void ExportChooserImpl::set_formats(const char** formats, int nformats, const char** commands) {
    if (!formats) return;

    /* set up memory */
    int i;
    for (i= 0; i= _nformats; i++) {
	delete _formats[i];
	delete _commands[i];
    }
    delete _formats;
    delete _commands;
    _nformats = nformats;
    _formats = new char*[_nformats];
    if (commands) 
	_commands = new char*[_nformats];
    for (i=0; i<nformats; i++) {
	_formats[i] = strdup(formats[i]);
	if (commands) 
	    _commands[i] = strdup(commands[i]);
    }

    /* build enum and observer*/
    StringList* list = new StringList;
    String* str_i;
    for (i=0; i<_nformats; i++) {
	str_i = new String(_formats[i]);
	list->append(*str_i);
    }
    _obse = new ObservableEnum(list);
    _editor = new ExportEnumEditor(_obse, "Format", this);
}

void ExportChooserImpl::free() {
    for (int i=0; i<_nformats; i++) 
	delete _formats[i];
    delete _formats;
    _formats = nil;
    _nformats = 0;
    OpenFileChooserImpl::free();
}

void ExportChooserImpl::to_printer_callback() {
    _to_printer = !_to_printer;
    if (!_to_printer) {
        if (strcmp(editor_->text()->string(), command(format()))==0)
	  editor_->field( "./" );
    } else {
        if (strcmp(editor_->text()->string(), "./")==0)
	  editor_->field(command(format()));
    }
}

void ExportChooserImpl::by_pathname_callback() {
    _by_pathname_flag = !_by_pathname_flag;
}

const char* ExportChooserImpl::command(const char* format) {
    int index = 0;
    while (index < _nformats) {
	if (strcmp(format, _formats[index])==0)
	    break;
	index++;
    }
    if (index==_nformats)
	return "ghostview";
    else {
	if (_commands) 
	    return _commands[index];
	else
	    return _formats[index];
    }
}

/*****************************************************************************/

declareEnumActionCallback(ExportEnumEditor)
implementEnumActionCallback(ExportEnumEditor)

ExportEnumEditor::ExportEnumEditor(ObservableEnum* obs, char* labl, ExportChooserImpl* eci)
: RadioEnumEditor()
{
    lab = labl;
    _group = new TelltaleGroup;
    _group->ref();
    _obs = obs;
    _obs->attach(this);
    build();
    update(_obs);
    _eci = eci;
}

void ExportEnumEditor::build() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    mainglyph = lk.vbox();
    mainglyph->append(lk.hcenter(wk.label(lab)));
    buildbox();
    mainglyph->append(lk.hcenter(_buttonbox));
    body(wk.inset_frame(lk.margin(mainglyph, 10)));
}

void ExportEnumEditor::buildbox() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    Glyph* glu = lk.vspace(5);
    _buttonbox = lk.vbox();
    Style *style_ = new Style(Session::instance()->style());
    style_->attribute("frameThickness", "2.5");
    style_->attribute("radioScale", "2.5");
    wk.push_style();
    wk.style(style_);
    for (int i = 0; i < _obs->maxvalue(); i++) {
	Action* action = new EnumActionCallback(ExportEnumEditor)(
	    this, &ExportEnumEditor::edit, _obs->labelvalue(i)
	);
	Button* button = wk.radio_button(_group, _obs->labelvalue(i), action);
	_buttonbox->append(lk.vbox(glu, button));
    }
    wk.pop_style();
}

void ExportEnumEditor::edit(String i) {
    String oldstring = _obs->labelvalue();
    _obs->setvalue(_obs->value(i));
    if (_eci->_to_printer && oldstring==_eci->editor_->text()->string()) 
	_eci->editor_->field(_eci->command(_obs->labelvalue(_obs->value(i)).string()));
}

