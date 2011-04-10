/*
 * Copyright (c) 2001 Scott Johnston
 * Copyright (c) 1998-1999 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * precise commands
 */

#include <OverlayUnidraw/ovprecise.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovpage.h>
#include <Unidraw/Commands/brushcmd.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/catalog.h>
#include <Unidraw/unidraw.h>
#include <IVGlyph/enumform.h>
#include <IVGlyph/stredit.h>
#include <Unidraw/editor.h>
#include <Unidraw/viewer.h>
#include <InterViews/window.h>
#include <stdio.h>
#include <string.h>
#include <strstream>

/*****************************************************************************/

char* OvPreciseMoveCmd::_default_movestr = nil;
int OvPreciseMoveCmd::_default_enumval = 0;

ClassId OvPreciseMoveCmd::GetClassId () { return OVPRECISEMOVE_CMD; }

boolean OvPreciseMoveCmd::IsA (ClassId id) {
    return OVPRECISEMOVE_CMD == id || PreciseMoveCmd::IsA(id);
}

OvPreciseMoveCmd::OvPreciseMoveCmd (ControlInfo* c) : PreciseMoveCmd(c) {}
OvPreciseMoveCmd::OvPreciseMoveCmd (Editor* ed) : PreciseMoveCmd(ed) {}
OvPreciseMoveCmd::~OvPreciseMoveCmd () {}

Command* OvPreciseMoveCmd::Copy () {
    Command* copy = new OvPreciseMoveCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

Glyph* OvPreciseMoveCmd::unit_buttons() {
    StringList* list = new StringList;
    String* str_i;
    str_i = new String("Pixels");
    list->append(*str_i);
    str_i = new String("Points");
    list->append(*str_i);
    str_i = new String("Centimeters");
    list->append(*str_i);
    str_i = new String("Inches");
    list->append(*str_i);
    _unit_enum = new ObservableEnum(list);
    _unit_enum->setvalue(_default_enumval);
    return new RadioEnumEditor(_unit_enum, "Units", true, true);
}

void OvPreciseMoveCmd::Execute () {
    if (!_default_movestr) 
      _default_movestr = strdup("1.0 1.0");
    char* movestr = 
      StrEditDialog::post(GetEditor()->GetWindow(),
			  "Enter X and Y movement:",
			  _default_movestr, nil, unit_buttons());

    int cur_unit = _unit_enum->intvalue();
    _default_enumval = cur_unit;

    if (movestr) {
      std::istrstream in(movestr);
      float xmove = 0, ymove = 0;
      in >> xmove >> ymove;

      switch (cur_unit) {
      case 1:   xmove *= ivpoints; ymove *= ivpoints; break;
      case 2:   xmove *= ivcm; ymove *= ivcm; break;
      case 3:   xmove *= ivinches; ymove *= ivinches; break;
      }

      if (xmove!=0.0 || ymove!=0.0) {
	MoveCmd* moveCmd = new MoveCmd(GetEditor(), xmove, ymove);
	moveCmd->Execute();
	moveCmd->Log();
      }
      delete _default_movestr;
      _default_movestr = movestr;
    }
}

/*****************************************************************************/

ClassId OvPreciseScaleCmd::GetClassId () { return OVPRECISESCALE_CMD; }

boolean OvPreciseScaleCmd::IsA (ClassId id) {
    return OVPRECISESCALE_CMD == id || PreciseScaleCmd::IsA(id);
}

OvPreciseScaleCmd::OvPreciseScaleCmd (ControlInfo* c) : PreciseScaleCmd(c) {}
OvPreciseScaleCmd::OvPreciseScaleCmd (Editor* ed) : PreciseScaleCmd(ed) {}
OvPreciseScaleCmd::~OvPreciseScaleCmd () {}

Command* OvPreciseScaleCmd::Copy () {
    Command* copy = new OvPreciseScaleCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvPreciseScaleCmd::Execute () {
    static char* default_scalestr = strdup("1.0 1.0");
    char* scalestr = 
      StrEditDialog::post(GetEditor()->GetWindow(),
			  "Enter X and Y scaling:",
			  default_scalestr);
    if (scalestr) {
      std::istrstream in(scalestr);
      float xscale = 0.0, yscale = 0.0;
      in >> xscale >> yscale;
      if (xscale !=0.0 && yscale != 0.0) {
	ScaleCmd* scaleCmd = new ScaleCmd(GetEditor(), xscale, yscale);
	scaleCmd->Execute();
	scaleCmd->Log();
      }
      delete default_scalestr;
      default_scalestr = scalestr;
    }
}

/*****************************************************************************/

ClassId OvPreciseRotateCmd::GetClassId () { return OVPRECISEROTATE_CMD; }

boolean OvPreciseRotateCmd::IsA (ClassId id) {
    return OVPRECISEROTATE_CMD == id || PreciseRotateCmd::IsA(id);
}

OvPreciseRotateCmd::OvPreciseRotateCmd (ControlInfo* c) : PreciseRotateCmd(c) {}
OvPreciseRotateCmd::OvPreciseRotateCmd (Editor* ed) : PreciseRotateCmd(ed) {}
OvPreciseRotateCmd::~OvPreciseRotateCmd () {}

Command* OvPreciseRotateCmd::Copy () {
    Command* copy = new OvPreciseRotateCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvPreciseRotateCmd::Execute () {
    static char* default_rotatestr = strdup("45.0");
    char* rotatestr = 
      StrEditDialog::post(GetEditor()->GetWindow(),
			  "Enter rotation in degrees:",
			  default_rotatestr);
    if (rotatestr) {
      std::istrstream in(rotatestr);
      float angle = 0.0;
      in >> angle;
      if (angle!=0.0) {
	RotateCmd* rotateCmd = new RotateCmd(GetEditor(), angle);
	rotateCmd->Execute();
	rotateCmd->Log();
      }
      delete default_rotatestr;
      default_rotatestr = rotatestr;
    }
}

/*****************************************************************************/

ClassId OvPrecisePageCmd::GetClassId () { return OVPRECISEPAGE_CMD; }

boolean OvPrecisePageCmd::IsA (ClassId id) {
    return OVPRECISEPAGE_CMD == id || PrecisePageCmd::IsA(id);
}

OvPrecisePageCmd::OvPrecisePageCmd (ControlInfo* c) : PrecisePageCmd(c) {}
OvPrecisePageCmd::OvPrecisePageCmd (Editor* ed) : PrecisePageCmd(ed) {}
OvPrecisePageCmd::~OvPrecisePageCmd () {}

Command* OvPrecisePageCmd::Copy () {
    Command* copy = new OvPrecisePageCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvPrecisePageCmd::Execute () {
    static char* default_pagestr = nil;
    if (!default_pagestr) {
      OverlayPage* page = (OverlayPage*) GetEditor()->GetViewer()->GetPage();
      char buffer[BUFSIZ];
      sprintf( buffer, "%d %d\0", (int) ((PageGraphic*)page->GetGraphic())->Width(), 
	(int) ((PageGraphic*)page->GetGraphic())->Height());
      default_pagestr = strdup(buffer);
    }
    char* pagestr = 
      StrEditDialog::post(GetEditor()->GetWindow(),
			  "Enter width and height of page:",
			  default_pagestr);
    if (pagestr) {
      std::istrstream in(pagestr);
      int xpage = 0, ypage = 0;
      in >> xpage >> ypage;
      if (xpage !=0 && ypage != 0) {
	Viewer* viewer = GetEditor()->GetViewer();
	viewer->SetPage(new OverlayPage(xpage, ypage, true));
	viewer->Update();
      }
      delete default_pagestr;
      default_pagestr = pagestr;
    }
}

/*****************************************************************************/

ClassId OvPreciseBrushCmd::GetClassId () { return OVPRECISEBRUSH_CMD; }

boolean OvPreciseBrushCmd::IsA (ClassId id) {
    return OVPRECISEBRUSH_CMD == id || Command::IsA(id);
}

OvPreciseBrushCmd::OvPreciseBrushCmd (ControlInfo* c) : Command(c) {}
OvPreciseBrushCmd::OvPreciseBrushCmd (Editor* ed) : Command(ed) {}
OvPreciseBrushCmd::~OvPreciseBrushCmd () {}

Command* OvPreciseBrushCmd::Copy () {
    Command* copy = new OvPreciseBrushCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvPreciseBrushCmd::Execute () {
    static char* default_widthstr = strdup("0.0");
    char* widthstr = 
      StrEditDialog::post(GetEditor()->GetWindow(),
			  "Enter brush width in pixels:",
			  default_widthstr);
    if (widthstr) {
      std::istrstream in(widthstr);
      float width = 0;
      in >> width;
      if (width>=0.0) {
	Catalog* catalog = unidraw->GetCatalog();
	PSBrush* br = catalog->FindBrush(0xffff, width);
	BrushCmd* brushCmd = new BrushCmd(GetEditor(), br);
	brushCmd->Execute();
	brushCmd->Log();
      }
      delete default_widthstr;
      default_widthstr = widthstr;
    }
}

