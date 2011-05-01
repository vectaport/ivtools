/*
 * Copyright (c) 1994, 1995 Vectaport Inc., Cider Press
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
 * Implementation of overlay specific commands.
 */

#include <OverlayUnidraw/ovcamcmds.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovdialog.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovpage.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovviewer.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/grid.h>
#include <Unidraw/iterator.h>
#include <Unidraw/statevars.h>
#include <Unidraw/viewer.h>

#include <IV-look/dialogs.h>
#include <IV-look/kit.h>

#include <InterViews/perspective.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/window.h>

#include <IV-2_6/InterViews/perspective.h>

#include <OS/math.h>
#include <OS/string.h>

#include <stdio.h>
#include <stream.h>

/*****************************************************************************/

ClassId CameraMotionCmd::GetClassId () { return CAMERA_MOTION_CMD; }

boolean CameraMotionCmd::IsA (ClassId id) {
    return CAMERA_MOTION_CMD == id || Command::IsA(id);
}

CameraMotionCmd::CameraMotionCmd(ControlInfo* i) : Command(i) {}

CameraMotionCmd::CameraMotionCmd(Editor* e) : Command(e) {}

#if 0
void CameraMotionCmd::MoveCamera(IntCoord dx, IntCoord dy) {
    OverlayViewer* v = (OverlayViewer*) GetEditor()->GetViewer();
    Perspective basep = *v->GetPerspective();

    if (!v->Chained()) {
	basep.cury += dy;
	basep.curx += dx;
	v->Adjust(basep);
    } else {
	Iterator i;
	for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
	    v = (OverlayViewer*) unidraw->GetEditor(i)->GetViewer();
	    if (v->Chained()) {
		Perspective p = *v->GetPerspective();
		p.cury += dy * p.height / basep.height;
		p.curx += dx * p.width / basep.width;
		v->Adjust(p);
	    }
	}
    }
}
#endif
    

/*****************************************************************************/

ClassId ZoomCmd::GetClassId () { return ZOOM_CMD; }

boolean ZoomCmd::IsA (ClassId id) {
    return ZOOM_CMD == id || CameraMotionCmd::IsA(id);
}

ZoomCmd::ZoomCmd(ControlInfo* i, float zf)
: CameraMotionCmd(i)
{
    _zf = zf;
}

ZoomCmd::ZoomCmd(Editor* e, float zf)
: CameraMotionCmd(e)
{
    _zf = zf;
}

void ZoomCmd::Execute() {
    Viewer* v = GetEditor()->GetViewer();
    Perspective p = *v->GetPerspective();
    int cx, cy;

    cx = p.curx + p.curwidth/2;
    cy = p.cury + p.curheight/2;
    p.curwidth = Math::round(float(p.curwidth) / _zf);
    p.curheight = Math::round(float(p.curheight) / _zf);
    p.curx = cx - p.curwidth/2;
    p.cury = cy - p.curheight/2;

    v->Adjust(p);
}

void ZoomCmd::Unexecute() {
    Viewer* v = GetEditor()->GetViewer();
    Perspective p = *v->GetPerspective();
    int cx, cy;

    cx = p.curx + p.curwidth/2;
    cy = p.cury + p.curheight/2;
    p.curwidth = Math::round(float(p.curwidth) * _zf);
    p.curheight = Math::round(float(p.curheight) * _zf);
    p.curx = cx - p.curwidth/2;
    p.cury = cy - p.curheight/2;

    v->Adjust(p);
}

boolean ZoomCmd::Reversible() { return false; }

Command* ZoomCmd::Copy() {
    Command* copy = new ZoomCmd(CopyControlInfo(), _zf);
    InitCopy(copy);
    return copy;
}

void ZoomCmd::Read(istream& in) {
    Command::Read(in);
    in >> _zf;
}

void ZoomCmd::Write(ostream& out) {
    Command::Write(out);
    out << _zf << " ";
}

/*****************************************************************************/

ClassId PreciseZoomCmd::GetClassId () { return PRECISEZOOM_CMD; }

boolean PreciseZoomCmd::IsA (ClassId id) {
    return PRECISEZOOM_CMD == id || CameraMotionCmd::IsA(id);
}

PreciseZoomCmd::PreciseZoomCmd(ControlInfo* i)
: CameraMotionCmd(i)
{
    _dialog = nil;
}

PreciseZoomCmd::PreciseZoomCmd(Editor* e)
: CameraMotionCmd(e)
{
    _dialog = nil;
}

PreciseZoomCmd::~PreciseZoomCmd() {
    if (_dialog)
	delete _dialog;
}

void PreciseZoomCmd::Execute() {
    float factor = 0.0;
    Editor* ed = GetEditor();

    if (_dialog == nil) {
	_dialog = new ZoomDialog();
    }

    ed->InsertDialog(_dialog);
    boolean accepted = _dialog->Accept();
    ed->RemoveDialog(_dialog);

    if (accepted) {
	_dialog->GetValue(factor);
	if (factor > 0.0) {
	    ZoomCmd* zoomCmd = new ZoomCmd(ed, factor);
	    zoomCmd->Execute();
	    zoomCmd->Log();
	}
    }
}

boolean PreciseZoomCmd::Reversible() { return false; }

Command* PreciseZoomCmd::Copy() {
    Command* copy = new PreciseZoomCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

/*****************************************************************************/

ClassId PanCmd::GetClassId () { return PAN_CMD; }

boolean PanCmd::IsA (ClassId id) {
    return PAN_CMD == id || CameraMotionCmd::IsA(id);
}

PanCmd::PanCmd(ControlInfo* i, IntCoord px, IntCoord py)
: CameraMotionCmd(i)
{
    _px = px;
    _py = py;
}

PanCmd::PanCmd(Editor* e, IntCoord px, IntCoord py)
: CameraMotionCmd(e)
{
    _px = px;
    _py = py;
}

void PanCmd::Execute() {
    OverlayViewer* v = (OverlayViewer*) GetEditor()->GetViewer();
    Perspective basep = *v->GetPerspective();
    basep.curx += _px;
    basep.cury += _py;
    v->Adjust(basep);
}

void PanCmd::Unexecute() {
    OverlayViewer* v = (OverlayViewer*) GetEditor()->GetViewer();
    Perspective basep = *v->GetPerspective();
    basep.cury -= _px;
    basep.curx -= _py;
    v->Adjust(basep);
}

boolean PanCmd::Reversible() { return false; }

Command* PanCmd::Copy() {
    Command* copy = new PanCmd(CopyControlInfo(), _px, _py);
    InitCopy(copy);
    return copy;
}

void PanCmd::Read(istream& in) {
    Command::Read(in);
    in >> _px >> _py;
}

void PanCmd::Write(ostream& out) {
    Command::Write(out);
    out << _px << " " << _py << " ";
}

/*****************************************************************************/

ClassId FixedPanCmd::GetClassId () { return FIXEDPAN_CMD; }

boolean FixedPanCmd::IsA (ClassId id) {
    return FIXEDPAN_CMD == id || CameraMotionCmd::IsA(id);
}

FixedPanCmd::FixedPanCmd(ControlInfo* i, PanAmount xpan, PanAmount ypan)
: CameraMotionCmd(i)
{
    _xpan = xpan;
    _ypan = ypan;
}

FixedPanCmd::FixedPanCmd(Editor* e, PanAmount xpan, PanAmount ypan)
: CameraMotionCmd(e)
{
    _xpan = xpan;
    _ypan = ypan;
}

void FixedPanCmd::Execute() {
    Viewer* v = GetEditor()->GetViewer();
    Perspective p = *v->GetPerspective();
    IntCoord px, py;
    switch(_xpan) {
    case NO_PAN:
	px = 0;
	break;
    case PLUS_SMALL_PAN:
	px = p.sx;
	break;
    case PLUS_LARGE_PAN:
	px = p.lx;
	break;
    case MINUS_SMALL_PAN:
	px = - p.sx;
	break;
    case MINUS_LARGE_PAN:
	px = - p.lx;
	break;
    }
    switch(_ypan) {
    case NO_PAN:
	py = 0;
	break;
    case PLUS_SMALL_PAN:
	py = p.sy;
	break;
    case PLUS_LARGE_PAN:
	py = p.ly;
	break;
    case MINUS_SMALL_PAN:
	py = - p.sy;
	break;
    case MINUS_LARGE_PAN:
	py = - p.ly;
	break;
    }
    PanCmd* pcmd = new PanCmd(GetEditor(), px, py);
    pcmd->Execute();
    pcmd->Log();
}

boolean FixedPanCmd::Reversible() { return false; }

Command* FixedPanCmd::Copy() {
    Command* copy = new FixedPanCmd(CopyControlInfo(), _xpan, _ypan);
    InitCopy(copy);
    return copy;
}

/*****************************************************************************/

ClassId PrecisePanCmd::GetClassId () { return PRECISEPAN_CMD; }

boolean PrecisePanCmd::IsA (ClassId id) {
    return PRECISEPAN_CMD == id || CameraMotionCmd::IsA(id);
}

PrecisePanCmd::PrecisePanCmd(ControlInfo* i)
: CameraMotionCmd(i)
{
    _dialog = nil;
}

PrecisePanCmd::PrecisePanCmd(Editor* e)
: CameraMotionCmd(e)
{
    _dialog = nil;
}

PrecisePanCmd::~PrecisePanCmd() {
    if (_dialog)
	delete _dialog;
}

void PrecisePanCmd::Execute() {
    float x = 0.0, y = 0.0;
    Editor* ed = GetEditor();

    if (_dialog == nil) {
	_dialog = new PanDialog();
    }

    ed->InsertDialog(_dialog);
    boolean accepted = _dialog->Accept();
    ed->RemoveDialog(_dialog);

    if (accepted) {
	_dialog->GetValues(x, y);
	if (x != 0.0 || y != 0.0) {
	    PanCmd* panCmd = new PanCmd(ed, Math::round(x), Math::round(y));
	    panCmd->Execute();
	    panCmd->Log();
	}
    }
}

boolean PrecisePanCmd::Reversible() { return false; }

Command* PrecisePanCmd::Copy() {
    Command* copy = new PrecisePanCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

