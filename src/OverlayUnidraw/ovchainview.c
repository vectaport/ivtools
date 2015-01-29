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
 * Chain(Unchain)ViewersCmd and Chain(Unchain)ViewerCmd implementation.
 */

#include <OverlayUnidraw/ovchainview.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovviewer.h>

#include <IVGlyph/gdialogs.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/dialogs.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>

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

ClassId ChainViewersCmd::GetClassId () { return CHAIN_VIEWERS_CMD; }

boolean ChainViewersCmd::IsA (ClassId id) {
    return CHAIN_VIEWERS_CMD == id || Command::IsA(id);
}

ChainViewersCmd::ChainViewersCmd (
    Viewer* viewer, ControlInfo* c, boolean pan, boolean zoom) 
: Command(c) { 
    _viewer = viewer;
    _pan = pan;
    _zoom = zoom;
}

ChainViewersCmd::ChainViewersCmd (
    Viewer* viewer, Editor* ed, boolean pan, boolean zoom) 
: Command(ed) { 
    _viewer = viewer; 
    _pan = pan;
    _zoom = zoom;
}

ChainViewersCmd::~ChainViewersCmd () { }

Command* ChainViewersCmd::Copy () {
    ChainViewersCmd* copy = new ChainViewersCmd(_viewer, CopyControlInfo(), _pan, _zoom);
    InitCopy(copy);
    return copy;
}

void ChainViewersCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void ChainViewersCmd::Execute () {
    Iterator i;
    for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
	OverlayViewer* viewer = (OverlayViewer*)unidraw->GetEditor(i)->GetViewer();
	viewer->Chain(_pan, _zoom);
    }

    unidraw->Update();

    return;
}

void ChainViewersCmd::Unexecute () {
    Editor* ed = GetEditor();

    Iterator i;
    for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
	OverlayViewer* viewer = (OverlayViewer*)unidraw->GetEditor(i)->GetViewer();
	viewer->Unchain(_pan, _zoom);
    }

    unidraw->Update();

    return;
}

/*****************************************************************************/

ClassId UnchainViewersCmd::GetClassId () { return UNCHAIN_VIEWERS_CMD; }

boolean UnchainViewersCmd::IsA (ClassId id) {
    return UNCHAIN_VIEWERS_CMD == id || Command::IsA(id);
}

UnchainViewersCmd::UnchainViewersCmd (
    Viewer* viewer, ControlInfo* c, boolean pan, boolean zoom) 
: Command(c) { 
  _viewer = viewer; 
  _pan = pan;
  _zoom = zoom;
}

UnchainViewersCmd::UnchainViewersCmd (
    Viewer* viewer, Editor* ed, boolean pan, boolean zoom) 
: Command(ed) { 
  _viewer = viewer; 
  _pan = pan;
  _zoom = zoom;
}

UnchainViewersCmd::~UnchainViewersCmd () { }

Command* UnchainViewersCmd::Copy () {
    UnchainViewersCmd* copy = new UnchainViewersCmd(_viewer, CopyControlInfo(), _pan, _zoom);
    InitCopy(copy);
    return copy;
}

void UnchainViewersCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void UnchainViewersCmd::Execute () {
    Iterator i;
    for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
	OverlayViewer* viewer = (OverlayViewer*)unidraw->GetEditor(i)->GetViewer();
	viewer->Unchain(_pan, _zoom);
    }

    unidraw->Update();

    return;
}

void UnchainViewersCmd::Unexecute () {
    Editor* ed = GetEditor();

    Iterator i;
    for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
	OverlayViewer* viewer = (OverlayViewer*)unidraw->GetEditor(i)->GetViewer();
	viewer->Chain(_pan, _zoom);
    }

    unidraw->Update();

    return;
}

/*****************************************************************************/

ClassId ChainViewerCmd::GetClassId () { return CHAIN_VIEWER_CMD; }

boolean ChainViewerCmd::IsA (ClassId id) {
    return CHAIN_VIEWER_CMD == id || Command::IsA(id);
}

ChainViewerCmd::ChainViewerCmd (
    ControlInfo* c, boolean pan, boolean zoom)
: Command(c) { 
  _pan = pan;
  _zoom = zoom;
}

ChainViewerCmd::ChainViewerCmd (
    Editor* ed, boolean pan, boolean zoom)
: Command(ed) { 
  _pan = pan;
  _zoom = zoom;
}

ChainViewerCmd::~ChainViewerCmd () { }

Command* ChainViewerCmd::Copy () {
    ChainViewerCmd* copy = new ChainViewerCmd(CopyControlInfo(), _pan, _zoom);
    InitCopy(copy);
    return copy;
}

void ChainViewerCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void ChainViewerCmd::Execute () {
    OverlayViewer* viewer = (OverlayViewer*)GetEditor()->GetViewer();
    viewer->Chain(_pan, _zoom);
    unidraw->Update();
    return;
}

void ChainViewerCmd::Unexecute () {
    OverlayViewer* viewer = (OverlayViewer*)GetEditor()->GetViewer();
    viewer->Unchain(_pan, _zoom);
    unidraw->Update();
    return;
}

/*****************************************************************************/

ClassId UnchainViewerCmd::GetClassId () { return UNCHAIN_VIEWER_CMD; }

boolean UnchainViewerCmd::IsA (ClassId id) {
    return UNCHAIN_VIEWER_CMD == id || Command::IsA(id);
}

UnchainViewerCmd::UnchainViewerCmd (
    ControlInfo* c, boolean pan, boolean zoom)
: Command(c) { 
  _pan = pan;
  _zoom = zoom;
}

UnchainViewerCmd::UnchainViewerCmd (
    Editor* ed, boolean pan, boolean zoom)
: Command(ed) { 
  _pan = pan;
  _zoom = zoom;
}

UnchainViewerCmd::~UnchainViewerCmd () { }

Command* UnchainViewerCmd::Copy () {
    UnchainViewerCmd* copy = new UnchainViewerCmd(CopyControlInfo(), _pan, _zoom);
    InitCopy(copy);
    return copy;
}

void UnchainViewerCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void UnchainViewerCmd::Execute () {
    OverlayViewer* viewer = (OverlayViewer*)GetEditor()->GetViewer();
    viewer->Unchain(_pan, _zoom);
    unidraw->Update();
    return;
}

void UnchainViewerCmd::Unexecute () {
    OverlayViewer* viewer = (OverlayViewer*)GetEditor()->GetViewer();
    viewer->Chain(_pan, _zoom);
    unidraw->Update();
    return;
}

