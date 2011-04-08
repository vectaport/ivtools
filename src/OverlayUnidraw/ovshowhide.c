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
 * HideViewCmd implementation.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
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

ClassId HideViewCmd::GetClassId () { return HIDE_VIEW_CMD; }

boolean HideViewCmd::IsA (ClassId id) {
    return HIDE_VIEW_CMD == id || Command::IsA(id);
}

HideViewCmd::HideViewCmd (Viewer* viewer, ControlInfo* c) : Command(c) { _viewer = viewer; }
HideViewCmd::HideViewCmd (Viewer* viewer, Editor* ed) : Command(ed) { _viewer = viewer; }
HideViewCmd::~HideViewCmd () { }

Command* HideViewCmd::Copy () {
    HideViewCmd* copy = new HideViewCmd(_viewer, CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void HideViewCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void HideViewCmd::Execute () {
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
	GraphicView* views = _viewer->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Interpret(this);
    }

    if (s) s->Clear();
    unidraw->Update();

    return;
}

void HideViewCmd::Unexecute () {
    Editor* ed = GetEditor();

    Clipboard* cb = GetClipboard();
    if (!cb) return;

    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicView* views = _viewer->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Uninterpret(this);
    }

    unidraw->Update();

    return;
}

/*****************************************************************************/

ClassId UnhideViewsCmd::GetClassId () { return UNHIDE_VIEWS_CMD; }

boolean UnhideViewsCmd::IsA (ClassId id) {
    return UNHIDE_VIEWS_CMD == id || Command::IsA(id);
}

UnhideViewsCmd::UnhideViewsCmd (ControlInfo* c) : Command(c) { }
UnhideViewsCmd::UnhideViewsCmd (Editor* ed) : Command(ed) { }
UnhideViewsCmd::~UnhideViewsCmd () { }

Command* UnhideViewsCmd::Copy () {
    UnhideViewsCmd* copy = new UnhideViewsCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

boolean UnhideViewsCmd::Reversible () { return false; }

void UnhideViewsCmd::Execute () {
    Editor* ed = GetEditor();

    Selection* s = ed->GetSelection();
    if (s->IsEmpty()) {
	return;
    }

    Iterator i;
    for (s->First(i); !s->Done(i); s->Next(i)) {
	GraphicView* view = s->GetView(i);
	view->GetSubject()->Interpret(this);
    }

    s->Clear();
    unidraw->Update();

    return;
}

/*****************************************************************************/

ClassId DesensitizeViewCmd::GetClassId () { return DESENSITIZE_VIEW_CMD; }

boolean DesensitizeViewCmd::IsA (ClassId id) {
    return DESENSITIZE_VIEW_CMD == id || Command::IsA(id);
}

DesensitizeViewCmd::DesensitizeViewCmd (Viewer* viewer, ControlInfo* c) : Command(c) { _viewer = viewer; }
DesensitizeViewCmd::DesensitizeViewCmd (Viewer* viewer, Editor* ed) : Command(ed) { _viewer = viewer; }
DesensitizeViewCmd::~DesensitizeViewCmd () { }

Command* DesensitizeViewCmd::Copy () {
    DesensitizeViewCmd* copy = new DesensitizeViewCmd(_viewer, CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void DesensitizeViewCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void DesensitizeViewCmd::Execute () {
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
	GraphicView* views = _viewer->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Interpret(this);
    }

    if (s) s->Clear();
    unidraw->Update();

    return;
}

void DesensitizeViewCmd::Unexecute () {
    Editor* ed = GetEditor();

    Clipboard* cb = GetClipboard();
    if (!cb) return;

    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicView* views = _viewer->GetGraphicView();
	GraphicView* view = views->GetGraphicView(cb->GetComp(i));
	view->Uninterpret(this);
    }

    unidraw->Update();

    return;
}

/*****************************************************************************/

ClassId SensitizeViewsCmd::GetClassId () { return SENSITIZE_VIEWS_CMD; }

boolean SensitizeViewsCmd::IsA (ClassId id) {
    return SENSITIZE_VIEWS_CMD == id || Command::IsA(id);
}

SensitizeViewsCmd::SensitizeViewsCmd (ControlInfo* c) : Command(c) { }
SensitizeViewsCmd::SensitizeViewsCmd (Editor* ed) : Command(ed) { }
SensitizeViewsCmd::~SensitizeViewsCmd () { }

Command* SensitizeViewsCmd::Copy () {
    SensitizeViewsCmd* copy = new SensitizeViewsCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

boolean SensitizeViewsCmd::Reversible () { return false; }

void SensitizeViewsCmd::Execute () {
    Editor* ed = GetEditor();

    Selection* s = ed->GetSelection();
    if (s->IsEmpty()) {
	return;
    }

    Iterator i;
    for (s->First(i); !s->Done(i); s->Next(i)) {
	GraphicView* view = s->GetView(i);
	view->GetSubject()->Interpret(this);
    }

    s->Clear();
    unidraw->Update();

    return;
}

