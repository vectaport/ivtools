/*
 * Copyright (c) 1994, 1995, 1998 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University 
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
 * OvExportCmd implementation.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovexport.h>
#include <OverlayUnidraw/scriptview.h>

#include <IVGlyph/exportchooser.h>
#include <IVGlyph/gdialogs.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/dialogs.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Graphic/graphic.h>

#include <IV-look/dialogs.h>
#include <IV-look/kit.h>
#include <InterViews/cursor.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <InterViews/display.h>

#include <OS/string.h>

#include <stream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/

ClassId OvExportCmd::GetClassId () { return OV_EXPORT_CMD; }

boolean OvExportCmd::IsA (ClassId id) {
    return OV_EXPORT_CMD == id || Command::IsA(id);
}

OvExportCmd::OvExportCmd (ControlInfo* c, ExportChooser* f) : Command(c) { 
    Init(f); 
}

OvExportCmd::OvExportCmd (Editor* ed, ExportChooser* f) : Command(ed) { 
    Init(f); 
}

OvExportCmd::~OvExportCmd () { 
    Resource::unref(chooser_); 
}

void OvExportCmd::Init (ExportChooser* f) {
    chooser_ = f;
    Resource::ref(chooser_);
}

Command* OvExportCmd::Copy () {
    OvExportCmd* copy = new OvExportCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

boolean OvExportCmd::Reversible () { return false; }

void OvExportCmd::Execute () {
    Editor* ed = GetEditor();

    Selection* s = ed->GetSelection();
    if (s->IsEmpty()) {
      GAcknowledgeDialog::post
	(ed->GetWindow(), "Nothing selected for export", nil, "no selection");
      return;
    }

    Style* style;
    boolean reset_caption = false;
    if (chooser_ == nil) {
	style = new Style(Session::instance()->style());
	style->attribute("subcaption", "Export selected graphics to file:");
	style->attribute("open", "Export");
	const char *formats[] = {"PostScript", "idraw", "drawtool"};
	const char *commands[] = {"ghostview %s", "idraw %s", "drawtool %s"};
	chooser_ = new ExportChooser(".", WidgetKit::instance(), style,
				     formats, sizeof(formats)/sizeof(char*), commands, nil, true);
	Resource::ref(chooser_);
    } else {
	style = chooser_->style();
    }
    boolean again; 
    while (again = chooser_->post_for(ed->GetWindow())) {
	const String* str = chooser_->selected();
	if (str != nil) {
	    NullTerminatedString ns(*str);
	    const char* name = ns.string();
	    style->attribute("caption", "              " );
	    chooser_->twindow()->repair();
	    chooser_->twindow()->display()->sync();

	    Catalog* catalog = unidraw->GetCatalog();
	    boolean ok = true;

	    if (!chooser_->to_printer() && catalog->Exists(name) && catalog->Writable(name)) {
		char buf[CHARBUFSIZE];
		sprintf(buf, "\"%s\" already exists,", name);
		GConfirmDialog* dialog = new GConfirmDialog(buf, "Overwrite?");
	    	Resource::ref(dialog);
	    	ok = dialog->post_for(ed->GetWindow());
	    	Resource::unref(dialog);
	    }

	    if (ok) { 
		ed->GetWindow()->cursor(hourglass);
		chooser_->twindow()->cursor(hourglass);
		if (Export(ns.string())) {
		    again = false;
		    break;
		}
		style->attribute("caption", "Export failed!" );
		reset_caption = true;
		ed->GetWindow()->cursor(arrow);
		chooser_->twindow()->cursor(arrow);
	    }
	}
    }

    chooser_->unmap();
    if (reset_caption) {
      style->attribute("caption", "              " );
    }
    if (!again)
	ed->GetWindow()->cursor(arrow);
    return;
}

boolean OvExportCmd::Export (const char* pathname) {
    Editor* editor = GetEditor();
    Selection* s = editor->GetSelection();
    OverlayIdrawComp* real_top = (OverlayIdrawComp*)editor->GetComponent();
    boolean ok = false;

    if (!s->IsEmpty()) {
	OverlayIdrawComp* false_top = new OverlayIdrawComp();
	Iterator i;
	s->First(i);
	while (!s->Done(i)) {
	    if (chooser_->idraw_format() || chooser_->postscript_format())
		false_top->Append(new OverlayComp(s->GetView(i)->GetGraphicComp()->GetGraphic()->Copy()));
	    else
		false_top->Append((OverlayComp*)s->GetView(i)->GetGraphicComp()->Copy());
	    s->Next(i);
	}
	
	OverlayPS* ovpsv;
	if (chooser_->idraw_format() || chooser_->postscript_format())
	    ovpsv = (OverlayPS*) false_top->Create(POSTSCRIPT_VIEW);
	else
	    ovpsv = (OverlayPS*) false_top->Create(SCRIPT_VIEW);
        if (ovpsv != nil) {

            filebuf fbuf;
	    char* tmpfilename;
		
	    if (chooser_->to_printer()) {
		tmpfilename = tmpnam(nil);
		false_top->SetPathName(tmpfilename);
		ok = fbuf.open(tmpfilename, output) != 0;
	    } else {
		ok = fbuf.open(pathname, output) != 0;
	    }

            if (ok) {
                ostream out(&fbuf);
                false_top->Attach(ovpsv);
		ovpsv->SetCommand(this);
		if (!chooser_->idraw_format() && !chooser_->postscript_format())
		    ((OverlayIdrawScript*)ovpsv)->SetByPathnameFlag(chooser_->by_pathname_flag());
                ovpsv->Update();
                ok = ovpsv->Emit(out);
		fbuf.close();

		if (chooser_->to_printer()) {
		    char cmd[CHARBUFSIZE];
		    if (strstr(pathname, "%s")) {
		        char buf[CHARBUFSIZE];
		        sprintf(buf, pathname, tmpfilename);    
			sprintf(cmd, "(%s;rm %s)&", buf, tmpfilename);
		    } else
			sprintf(cmd, "(%s %s;rm %s)&", pathname, tmpfilename, tmpfilename);
		    ok = system(cmd) == 0;
		}
            } 
            delete ovpsv;        
	}

	delete false_top;
    }
    return ok;
}

const char* OvExportCmd::format() { return chooser_->format(); }
boolean OvExportCmd::idraw_format() { return chooser_->idraw_format(); }
