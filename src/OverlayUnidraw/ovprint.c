/*
 * Copyright (c) 1995 Vectaport Inc.
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
 * OvPrintCmd implementation.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/ovpsview.h>

#include <IVGlyph/gdialogs.h>
#include <IVGlyph/printchooser.h>

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

#include <OS/string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stream.h>
#include <string.h>
#include <fstream.h>

/*****************************************************************************/

ClassId OvPrintCmd::GetClassId () { return OVPRINT_CMD; }

boolean OvPrintCmd::IsA (ClassId id) {
    return OVPRINT_CMD == id || Command::IsA(id);
}

OvPrintCmd::OvPrintCmd (ControlInfo* c, PrintChooser* f) : Command(c) { Init(f); }
OvPrintCmd::OvPrintCmd (Editor* ed, PrintChooser* f) : Command(ed) { Init(f); }
OvPrintCmd::~OvPrintCmd () { Resource::unref(chooser_); }
void OvPrintCmd::Init (PrintChooser* f) {
    chooser_ = f;
    Resource::ref(chooser_);
}

Command* OvPrintCmd::Copy () {
    OvPrintCmd* copy = new OvPrintCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

boolean OvPrintCmd::Reversible () { return false; }

void OvPrintCmd::Execute () {
    Editor* ed = GetEditor();

    Style* style;
    boolean reset_caption = false;
    if (chooser_ == nil) {
	style = new Style(Session::instance()->style());
	style->attribute("subcaption", "Save postscript to file:");
	style->attribute("open", "Print");
	chooser_ = new PrintChooser(".", WidgetKit::instance(), Session::instance()->style());
	Resource::ref(chooser_);
    } else {
	style = chooser_->style();
    }

    /* loop on reading input until an ok break */
    boolean again;
    while (again = chooser_->post_for(ed->GetWindow())) {
	const String* str = chooser_->selected();
	if (str != nil) {
	    NullTerminatedString ns(*str);
	    const char* name = ns.string();
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

		filebuf fbuf;
		char* tmpfilename;
		
		if (chooser_->to_printer()) {
		    tmpfilename = tmpnam(nil);
		    ok = fbuf.open(tmpfilename, output) != 0;
		} else {
		    ok = fbuf.open(ns.string(), output) != 0;
		}

		if (ok) {
		    ed->GetWindow()->cursor(hourglass);
		    chooser_->twindow()->cursor(hourglass);
		    ostream out(&fbuf);
		    GraphicComp* comps = GetGraphicComp();
		    OverlayPS* ovpsv = (OverlayPS*) comps->Create(POSTSCRIPT_VIEW);
		    comps->Attach(ovpsv);
		    ovpsv->SetCommand(this);
		    ovpsv->Update();
		    ok = ovpsv->Emit(out);
		    out.flush();
		    delete ovpsv;

		    if (chooser_->to_printer()) {
			ok = print(ns.string(), tmpfilename) == 0;
		    }
		}
		if (ok) {
		    again = false;
		    break;
		} else {
		    style->attribute("caption", "");
		    style->attribute("caption", "Printing to file failed!");
		    reset_caption = true;
		    ed->GetWindow()->cursor(arrow);
		    chooser_->twindow()->cursor(arrow);
		}
	    }
	}
    }
    /* end of looping on reading input until an ok break */

    chooser_->unmap();
    if (reset_caption) {
	style->attribute("caption", "");
    }
    if (!again)
	ed->GetWindow()->cursor(arrow);
    return;
}

int OvPrintCmd::print (const char* print_cmd, const char* file) {
    
    char cmd[CHARBUFSIZE];
    if (strstr(print_cmd, "%s")) {
        char buf[CHARBUFSIZE];
	sprintf(buf, print_cmd, file);
        sprintf(cmd, "(%s;rm %s)&", buf, file);
    } else 
        sprintf(cmd, "(%s %s ;rm %s)&", print_cmd, file, file);
    return system(cmd);
}

boolean OvPrintCmd::to_printer() { return chooser_->to_printer(); }
boolean OvPrintCmd::idraw_format() { return chooser_->idraw_format(); }
