/*
 * Copyright (c) 1994-1995 Vectaport Inc.
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
 * FixViewCmd and UnfixViewCmd implementation.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovfixview.h>
#include <OverlayUnidraw/ovshowhide.h>
#include <OverlayUnidraw/ovunidraw.h>

#include <IVGlyph/gdialogs.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/dialogs.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Graphic/graphic.h>

#include <IV-look/dialogs.h>
#include <IV-look/kit.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/window.h>

#include <OS/string.h>

#include <stream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/

ClassId FixViewCmd::GetClassId () { return FIX_VIEW_CMD; }

boolean FixViewCmd::IsA (ClassId id) {
    return FIX_VIEW_CMD == id || Command::IsA(id);
}

FixViewCmd::FixViewCmd (ControlInfo* c, boolean size, boolean location) 
: Command(c) {
    _size = size;
    _location = location;
 }

FixViewCmd::FixViewCmd (Editor* ed, boolean size, boolean location) 
: Command(ed) { 
    _size = size;
    _location = location;
}

FixViewCmd::~FixViewCmd () { }

Command* FixViewCmd::Copy () {
    FixViewCmd* copy = new FixViewCmd(CopyControlInfo(), _size, _location);
    InitCopy(copy);
    return copy;
}

void FixViewCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void FixViewCmd::Execute () {
    Editor* ed = GetEditor();

    Selection* s = ed->GetSelection();
    Clipboard* cb = GetClipboard();
    if (!cb) {
	if (!s) return;
	SetClipboard(cb  = new Clipboard);
	cb->Init(s);
    }

    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicView* views = ed->GetViewer()->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Interpret(this);
    }

    return;
}

void FixViewCmd::Unexecute () {
    Editor* ed = GetEditor();

    Clipboard* cb = GetClipboard();
    if (!cb) return;

    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicView* views = ed->GetViewer()->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Uninterpret(this);
    }

    return;
}

/*****************************************************************************/

ClassId UnfixViewCmd::GetClassId () { return UNFIX_VIEW_CMD; }

boolean UnfixViewCmd::IsA (ClassId id) {
    return UNFIX_VIEW_CMD == id || Command::IsA(id);
}

UnfixViewCmd::UnfixViewCmd (ControlInfo* c, boolean size, boolean location) 
: Command(c) {
    _size = size;
    _location = location;
 }

UnfixViewCmd::UnfixViewCmd (Editor* ed, boolean size, boolean location) 
: Command(ed) { 
    _size = size;
    _location = location;
}

UnfixViewCmd::~UnfixViewCmd () { }

Command* UnfixViewCmd::Copy () {
    UnfixViewCmd* copy = new UnfixViewCmd(CopyControlInfo(), _size, _location);
    InitCopy(copy);
    return copy;
}

void UnfixViewCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void UnfixViewCmd::Execute () {
    Editor* ed = GetEditor();

    Selection* s = ed->GetSelection();
    Clipboard* cb = GetClipboard();
    if (!cb) {
	if (!s) return;
	SetClipboard(cb  = new Clipboard);
	cb->Init(s);
    }

    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicView* views = ed->GetViewer()->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Interpret(this);
    }

    return;
}

void UnfixViewCmd::Unexecute () {
    Editor* ed = GetEditor();

    Clipboard* cb = GetClipboard();
    if (!cb) return;

    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicView* views = ed->GetViewer()->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Uninterpret(this);
    }

    return;
}

