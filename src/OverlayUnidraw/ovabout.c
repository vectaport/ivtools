/*
 * Copyright (c) 1995-1998 Vectaport Inc.
 * Copyright (c) 1994 Vectaport Inc., Cartoactive Systems
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

#include <InterViews/action.h>
#include <IV-3_1/InterViews/dialog.h>
#include <InterViews/layout.h>
#include <InterViews/polyglyph.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <IV-look/kit.h>
#include <OverlayUnidraw/ovabout.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <string.h>

class OvAboutDialog : public Dialog {
public:
  OvAboutDialog(Glyph*, Style*);
  void dismiss();
};

OvAboutDialog::OvAboutDialog(Glyph* g, Style* s) 
: Dialog(g,s)
{
}

void OvAboutDialog::dismiss() {
    Dialog::dismiss(true);
}

declareActionCallback(OvAboutDialog)
implementActionCallback(OvAboutDialog)

ClassId OvAboutCmd::GetClassId () { return OVABOUT_CMD; }

boolean OvAboutCmd::IsA (ClassId id) {
    return OVABOUT_CMD == id || Command::IsA(id);
}

OvAboutCmd::OvAboutCmd (ControlInfo* c) : Command(c) { Init(); }
OvAboutCmd::OvAboutCmd (Editor* ed) : Command(ed) { Init(); }

void OvAboutCmd::Init() {
    const LayoutKit& layout = *LayoutKit::instance();
    const WidgetKit& kit = *WidgetKit::instance();
    PolyGlyph* vb = layout.vbox(25);
    char banner[] = "\
 |\
binary Copyright (c) 1994-2000 Vectaport Inc.|\
 |\
Permission to use, copy, modify, distribute, and sell this software and|\
its documentation for any purpose is hereby granted without fee, provided|\
that the above copyright notice appear in all copies and that both that|\
copyright notice and this permission notice appear in supporting|\
documentation, and that the names of the copyright holders not be used in|\
advertising or publicity pertaining to distribution of the software|\
without specific, written prior permission.  The copyright holders make|\
no representations about the suitability of this software for any purpose.|\
It is provided \"as is\" without express or implied warranty.|\
 |\
THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS|\
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.|\
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,|\
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING|\
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,|\
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION|\
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.|\
 |\
source and online documentation at http://www.ivtools.org|\
 |";
    vb->append(kit.label(strtok(banner, "|")));
    char* line = strtok(nil, "|");
    while (line) {
	vb->append(kit.label(line));
	line = strtok(nil, "|");
    }
    vb->append(layout.vglue(5));
    Button* but;
    aboutdialog = new OvAboutDialog(kit.outset_frame(layout.margin(vb,5.0)), Session::instance()->style());
    Action* act = new ActionCallback(OvAboutDialog)(this->aboutdialog, &OvAboutDialog::dismiss);
    vb->append(layout.hbox(layout.hglue(), but = kit.push_button("OK", act), layout.hglue()));
    aboutdialog->append_input_handler(but);
    aboutdialog->focus(but);
    Resource::ref(aboutdialog);
}

Command* OvAboutCmd::Copy () {
    Command* copy = new OvAboutCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvAboutCmd::Execute () {
    OverlayEditor* editor = (OverlayEditor*)GetEditor();
    aboutdialog->post_for(editor->GetWindow());
}

boolean OvAboutCmd::Reversible () { return true; }
