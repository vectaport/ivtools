/*
 * Copyright (c) 1997-2000 Vectaport Inc.
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

#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/framestates.h>
#include <FrameUnidraw/frameviews.h>

#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovunidraw.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/ctrlinfo.h>
#include <Unidraw/dialogs.h>
#include <Unidraw/selection.h>
#include <Unidraw/statevars.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Graphic/damage.h>

#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>

#include <OS/math.h>

#include <stdio.h>
#include <string.h>

/*****************************************************************************/

CreateFrameCmd::CreateFrameCmd(ControlInfo* i, boolean after)
: Command(i)
{
    _after = after;
}

CreateFrameCmd::CreateFrameCmd(Editor* e, boolean after)
: Command(e)
{
    _after = after;
}

ClassId CreateFrameCmd::GetClassId() { return CREATEFRAME_CMD; }
boolean CreateFrameCmd::IsA(ClassId id) {
    return id == CREATEFRAME_CMD || Command::IsA(id);
}

Command* CreateFrameCmd::Copy() {
    Command* copy = new CreateFrameCmd(CopyControlInfo(), _after);
    InitCopy(copy);
    return copy;
}

boolean CreateFrameCmd::Reversible() { return true; }

void CreateFrameCmd::Execute() {
    GetEditor()->GetComponent()->Interpret(this);
}

void CreateFrameCmd::Unexecute() {
    GetEditor()->GetComponent()->Uninterpret(this);
}

/*****************************************************************************/

DeleteFrameData::DeleteFrameData (void* v, boolean restore_after) : VoidData(v) {
    _after = restore_after;
}

/*****************************************************************************/

DeleteFrameCmd::DeleteFrameCmd(ControlInfo* i)
: Command(i)
{    
}

DeleteFrameCmd::DeleteFrameCmd(Editor* e)
: Command(e)
{
}

ClassId DeleteFrameCmd::GetClassId() { return DELETEFRAME_CMD; }
boolean DeleteFrameCmd::IsA(ClassId id) {
    return id == DELETEFRAME_CMD || Command::IsA(id);
}

Command* DeleteFrameCmd::Copy() {
    Command* copy = new DeleteFrameCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

boolean DeleteFrameCmd::Reversible() { return true; }

void DeleteFrameCmd::Execute() {
    GetEditor()->GetComponent()->Interpret(this);
}

void DeleteFrameCmd::Unexecute() {
    GetEditor()->GetComponent()->Uninterpret(this);
}

/*****************************************************************************/

boolean MoveFrameCmd::_func_on= false;
char* MoveFrameCmd::_move_func = nil;
char* MoveFrameCmd::_absmove_func = nil;
MoveFrameCmd* MoveFrameCmd::_default = nil;

MoveFrameCmd::MoveFrameCmd(ControlInfo* i, int motion, boolean allowbg)
: Command(i)
{
  init(motion, allowbg);
}

MoveFrameCmd::MoveFrameCmd(Editor* e, int motion, boolean allowbg)
: Command(e)
{
  init(motion, allowbg);
}

void MoveFrameCmd::init(int motion, boolean allowbg) {
  _requestmotion = motion;
  _plannedmotion = _actualmotion = 0;
  _allowbg = allowbg;
  _wraparound = false;
}

ClassId MoveFrameCmd::GetClassId() { return MOVEFRAME_CMD; }
boolean MoveFrameCmd::IsA(ClassId id) {
    return id == MOVEFRAME_CMD || Command::IsA(id);
}

Command* MoveFrameCmd::Copy() {
    MoveFrameCmd* copy = new MoveFrameCmd(CopyControlInfo(), _requestmotion, _allowbg);
    copy->wraparound(wraparound());
    InitCopy(copy);
    return copy;
}

boolean MoveFrameCmd::Reversible() { return true; }

void MoveFrameCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

const char* MoveFrameCmd::MoveFuncFormat() 
{
  return _func_on ? (_move_func ? _move_func : "timeframe(%d)" ) : nil; 
}

const char* MoveFrameCmd::AbsMoveFuncFormat() 
{
  return _func_on ? (_absmove_func ? _absmove_func : "timeframe(%d :abs)" ) : nil; 
}

void MoveFrameCmd::FuncEnable(const char* movefunc, const char* absmovefunc) 
{ 
  _func_on=true; 
  if (movefunc) { 
    delete _move_func; 
    _move_func = strdup(movefunc);
  } 
  if (absmovefunc) { 
    delete _absmove_func; 
    _absmove_func = strdup(absmovefunc);
  } 
}

void MoveFrameCmd::Execute() {
    FrameEditor* ed = (FrameEditor*) GetEditor();
    FrameIdrawComp *comp = ((FrameIdrawComp*)ed->GetComponent());
    ed->GetViewer()->GetSelection()->Clear();
    FramesView* fv = (FramesView*)ed->GetViewer()->GetGraphicView();
    Iterator frameptr;
    fv->SetView(ed->GetFrame(), frameptr);
    OverlaysView* prev = ed->GetFrame();
    _actualmotion = 0;
    FrameNumberState* fnumstate = ed->framenumstate();
    int ofnum = fnumstate->framenumber();
    if (!_allowbg && ofnum + _requestmotion <= 0)
      _plannedmotion = -ofnum + 1;
    else
      _plannedmotion = _requestmotion;
    for (int i = 0; i < Math::abs(_plannedmotion); i++) {
	if (!fv->Done(frameptr)) {
	    if (_plannedmotion > 0)
		fv->Next(frameptr);
	    else
		fv->Prev(frameptr);
	    _actualmotion++;
	}
    }
    if (fv->Done(frameptr)) {
	if (plannedmotion() > 0)
	    fv->Prev(frameptr);
	else
	    fv->Next(frameptr);
	_actualmotion--;
	if (wraparound()) {
	  if (requestmotion() > 0) {
	    fv->First(frameptr);
	    fv->Next(frameptr);
	  } else 
	    fv->Last(frameptr);
	}
    }
    ed->SetFrame((FrameView*)fv->GetView(frameptr));
    if (ed->GetFrame() && prev != ed->GetFrame()) {
	Damage* damage = ed->GetViewer()->GetDamage();
	damage->Incur(prev->GetGraphic());
	damage->Incur(ed->GetFrame()->GetGraphic());
    }
    ed->UpdateFrame(true);
    int fnum = fv->Index(frameptr);
    fnumstate->framenumber(fnum);
    ComTerpServ* comterp = ed->GetComTerp();
    const char* funcformat = MoveFuncFormat();
    if (funcformat && comterp) {
      char buf[BUFSIZ];
      sprintf(buf, funcformat, _requestmotion);
      ComValue& retval = comterp->run(buf);
    }
    unidraw->Update();
}

void MoveFrameCmd::Unexecute() {
    FrameEditor* ed = (FrameEditor*) GetEditor();
    FrameIdrawComp* comp = ((FrameIdrawComp*)ed->GetComponent());
    FramesView* fv = (FramesView*)ed->GetViewer()->GetGraphicView();
    OverlaysView* prev = ed->GetFrame();
    Iterator frameptr;
    fv->SetView(ed->GetFrame(), frameptr);
    Damage* damage = ed->GetViewer()->GetDamage();
    damage->Incur(ed->GetFrame()->GetGraphic());
    for (int i = 0; i < _actualmotion; i++) {
	if (_plannedmotion > 0)
	    fv->Prev(frameptr);
	else
	    fv->Next(frameptr);
    }
    ed->SetFrame((FrameView*)fv->GetView(frameptr));
    damage->Incur(ed->GetFrame()->GetGraphic());
    ed->UpdateFrame(true);
    FrameNumberState* fnumstate = ed->framenumstate();
    fnumstate->framenumber(fv->Index(frameptr));
    ComTerpServ* comterp = ed->GetComTerp();
    const char* funcformat = MoveFuncFormat();
    if (funcformat && comterp) {
      char buf[BUFSIZ];
      sprintf(buf, funcformat, -_requestmotion);
      ComValue& retval = comterp->run(buf);
    }
    unidraw->Update();
}

void MoveFrameCmd::clr_wraparound() {
  wraparound(false);
}

void MoveFrameCmd::set_wraparound() {
  wraparound(true);
}

implementActionCallback(MoveFrameCmd)

/*****************************************************************************/

FrameBeginCmd::FrameBeginCmd(ControlInfo* i)
: MoveFrameCmd(i, 0)
{
}

FrameBeginCmd::FrameBeginCmd(Editor* e)
: MoveFrameCmd(e, 0)
{
}

ClassId FrameBeginCmd::GetClassId() { return FRAMEBEGIN_CMD; }
boolean FrameBeginCmd::IsA(ClassId id) {
    return id == FRAMEBEGIN_CMD || MoveFrameCmd::IsA(id);
}

Command* FrameBeginCmd::Copy() {
    Command* copy = new FrameBeginCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void FrameBeginCmd::Execute() {
    FrameEditor* ed = (FrameEditor*) GetEditor();
    FrameIdrawComp *comp = ((FrameIdrawComp*)ed->GetComponent());
    ed->GetViewer()->GetSelection()->Clear();
    FramesView* fv = (FramesView*)ed->GetViewer()->GetGraphicView();
    Iterator frameptr;
    fv->SetView( ed->GetFrame(), frameptr );
    Damage* damage = ed->GetViewer()->GetDamage();
    damage->Incur(ed->GetFrame()->GetGraphic());
    int before = fv->Index(frameptr);
    fv->First(frameptr);
    fv->Next(frameptr);
    if (fv->Done(frameptr))
        fv->First(frameptr);    
    int after = fv->Index(frameptr);
    ed->SetFrame((FrameView*)fv->GetView(frameptr));
    damage->Incur(ed->GetFrame()->GetGraphic());
    ed->UpdateFrame(true);
    FrameNumberState* fnumstate = ed->framenumstate();
    int fnum = fv->Index(frameptr);
    fnumstate->framenumber(fnum);
    _requestmotion = before-after;
    _actualmotion = Math::abs(_requestmotion);
    const char* funcformat = AbsMoveFuncFormat();
    ComTerpServ* comterp = ed->GetComTerp();
    if (funcformat && comterp) {
      char buf[BUFSIZ];
      sprintf(buf, funcformat, _allowbg ? 0 : 1);
      ComValue& retval = comterp->run(buf);
    }
    unidraw->Update();
}

/*****************************************************************************/

FrameEndCmd::FrameEndCmd(ControlInfo* i)
: MoveFrameCmd(i, 0)
{
}

FrameEndCmd::FrameEndCmd(Editor* e)
: MoveFrameCmd(e, 0)
{
}

ClassId FrameEndCmd::GetClassId() { return FRAMEEND_CMD; }
boolean FrameEndCmd::IsA(ClassId id) {
    return id == FRAMEEND_CMD || MoveFrameCmd::IsA(id);
}

Command* FrameEndCmd::Copy() {
    Command* copy = new FrameEndCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void FrameEndCmd::Execute() {
    FrameEditor* ed = (FrameEditor*) GetEditor();
    FrameIdrawComp *comp = ((FrameIdrawComp*)ed->GetComponent());
    ed->GetViewer()->GetSelection()->Clear();
    FramesView* fv = (FramesView*)ed->GetViewer()->GetGraphicView();
    Iterator frameptr;
    fv->SetView(ed->GetFrame(), frameptr);
    Damage* damage = ed->GetViewer()->GetDamage();
    damage->Incur(fv->GetView(frameptr)->GetGraphic());
    int before = fv->Index(frameptr);
    fv->Last(frameptr);
    int after = fv->Index(frameptr);
    ed->SetFrame((FrameView*)fv->GetView(frameptr));
    damage->Incur(ed->GetFrame()->GetGraphic());
    ed->UpdateFrame(true);
    FrameNumberState* fnumstate = ed->framenumstate();
    int fnum = fv->Index(frameptr);
    fnumstate->framenumber(fnum);
    _requestmotion = after-before;
    _actualmotion = Math::abs(_requestmotion);
    const char* funcformat = AbsMoveFuncFormat();
    ComTerpServ* comterp = ed->GetComTerp();
    if (funcformat && comterp) {
      char buf[BUFSIZ];
      sprintf(buf, funcformat, fnum);
      ComValue& retval = comterp->run(buf);
    }
    unidraw->Update();
}

/*****************************************************************************/

CreateMoveFrameCmd::CreateMoveFrameCmd(ControlInfo* i, boolean after)
: MacroCmd(i)
{
    _after = after;
    Append(new CreateFrameCmd(i->Copy(), _after));
    Append(new MoveFrameCmd(i->Copy(), _after ? +1 : -1));
}

CreateMoveFrameCmd::CreateMoveFrameCmd(Editor* e, boolean after)
: MacroCmd(e)
{
    _after = after;
    Append(new CreateFrameCmd(e, _after));
    Append(new MoveFrameCmd(e, _after ? +1 : -1));
}

ClassId CreateMoveFrameCmd::GetClassId() { return CREATEMOVEFRAME_CMD; }
boolean CreateMoveFrameCmd::IsA(ClassId id) {
    return id == CREATEMOVEFRAME_CMD || Command::IsA(id);
}

Command* CreateMoveFrameCmd::Copy() {
    Command* copy = new CreateMoveFrameCmd(CopyControlInfo(), _after);
    InitCopy(copy);
    return copy;
}

CreateFrameCmd* CreateMoveFrameCmd::createframecmd() {
  Iterator i;  First(i);  return (CreateFrameCmd*)GetCommand(i);
}

MoveFrameCmd* CreateMoveFrameCmd::moveframecmd() {
  Iterator i;  First(i);  Next(i); return (MoveFrameCmd*)GetCommand(i);
}

/*****************************************************************************/ 
CopyMoveFrameCmd::CopyMoveFrameCmd(ControlInfo* i, boolean after)
: MacroCmd(i)
{
  _after = after;
}

CopyMoveFrameCmd::CopyMoveFrameCmd(Editor* e, boolean after)
: MacroCmd(e)
{
    _after = after;
}

ClassId CopyMoveFrameCmd::GetClassId() { return COPYMOVEFRAME_CMD; }
boolean CopyMoveFrameCmd::IsA(ClassId id) {
    return id == COPYMOVEFRAME_CMD || Command::IsA(id);
}

Command* CopyMoveFrameCmd::Copy() {
    Command* copy = new CopyMoveFrameCmd(CopyControlInfo(), _after);
    InitCopy(copy);
    return copy;
}

void CopyMoveFrameCmd::Execute() {
  FrameEditor* ed = (FrameEditor*)GetEditor();
  Append(new OvSlctAllCmd(ed));
  Append(new FrameCopyCmd(ed));
  Append(new CreateFrameCmd(ed, _after));
  Append(new MoveFrameCmd(ed, _after ? +1 : -1));
  Append(new PasteCmd(ed));
  MacroCmd::Execute();
}

/*****************************************************************************/

ClassId FrameGroupCmd::GetClassId () { return FRAME_GROUP_CMD; }

boolean FrameGroupCmd::IsA (ClassId id) {
    return FRAME_GROUP_CMD == id || OvGroupCmd::IsA(id);
}

FrameGroupCmd::FrameGroupCmd (ControlInfo* c, OverlayComp* d) : OvGroupCmd(c, d) { }
FrameGroupCmd::FrameGroupCmd (Editor* ed, OverlayComp* d) : OvGroupCmd(ed, d) { }

Command* FrameGroupCmd::Copy () {
    OverlayComp* dest = (_group == nil) ? nil : (OverlayComp*) _group->Copy();
    Command* copy = new FrameGroupCmd(CopyControlInfo(), dest);
    InitCopy(copy);
    return copy;
}

void FrameGroupCmd::Execute () {
    Clipboard* cb = GetClipboard();
    FrameEditor* ed = (FrameEditor*)GetEditor();
    FrameIdrawView* top = (FrameIdrawView*)ed->GetViewer()->GetGraphicView();
    GraphicView* views = ((FrameEditor*)ed)->GetFrame();

    if (cb == nil) {
        SetClipboard(cb = new Clipboard);
        Selection* s = ed->GetSelection();

        if (s->Number() > 1) {
            Iterator i;
            s->Sort(views);
            for (s->First(i); !s->Done(i); s->Next(i)) {
                s->GetView(i)->Interpret(this);
            }
        }

    } else {
        Clipboard* oldcb = cb;
        SetClipboard(cb = new Clipboard);

        Iterator i;
        for (oldcb->First(i); !oldcb->Done(i); oldcb->Next(i)) {
            oldcb->GetComp(i)->Interpret(this);
        }
        delete oldcb;
    }

    if (!cb->IsEmpty()) {
        if (_group == nil) {
            SetGroup(MakeOverlaysComp());
        }
	ed->GetComponent()->Interpret(this);
        _executed = true;
    }
}

OverlaysComp* FrameGroupCmd::MakeOverlaysComp() {
    return new FrameOverlaysComp();
}

/*****************************************************************************/

ClassId FrameUngroupCmd::GetClassId () { return FRAME_UNGROUP_CMD; }

boolean FrameUngroupCmd::IsA (ClassId id) {
    return FRAME_UNGROUP_CMD == id || UngroupCmd::IsA(id);
}

FrameUngroupCmd::FrameUngroupCmd (ControlInfo* c) : UngroupCmd(c) { Init(); }
FrameUngroupCmd::FrameUngroupCmd (Editor* ed) : UngroupCmd(ed) { Init(); }

FrameUngroupCmd::~FrameUngroupCmd () {
}

Command* FrameUngroupCmd::Copy () {
    Command* copy = new FrameUngroupCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void FrameUngroupCmd::Execute () {
    Clipboard* cb = GetClipboard();
    FrameEditor* ed = (FrameEditor*)GetEditor();
    FrameIdrawView* top = (FrameIdrawView*)ed->GetViewer()->GetGraphicView();
    GraphicView* views = ((FrameEditor*)ed)->GetFrame();

    if (cb == nil) {
        Selection* s = ed->GetSelection();

        if (s->IsEmpty()) {
            return;
        }

        SetClipboard(cb = new Clipboard);
        s->Sort(views);
        Iterator i;
        for (s->First(i); !s->Done(i); s->Next(i)) {
            s->GetView(i)->Interpret(this);
        }

    } else {
        Clipboard* oldcb = cb;
        SetClipboard(cb = new Clipboard);
        Iterator i;
        for (oldcb->First(i); !oldcb->Done(i); oldcb->Next(i)) {
            oldcb->GetComp(i)->Interpret(this);
        }
        delete oldcb;
    }

    if (!cb->IsEmpty()) {
	ed->GetComponent()->Interpret(this);
        _executed = true;
    }
}


/*****************************************************************************/

ClassId FrameFrontCmd::GetClassId () { return FRAME_FRONT_CMD; }
boolean FrameFrontCmd::IsA (ClassId id) { return FRAME_FRONT_CMD==id || FrontCmd::IsA(id);}

FrameFrontCmd::FrameFrontCmd (ControlInfo* c) : FrontCmd(c) { }
FrameFrontCmd::FrameFrontCmd (Editor* ed) : FrontCmd(ed) { }

Command* FrameFrontCmd::Copy () {
    Command* copy = new FrameFrontCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void FrameFrontCmd::Execute () {
    Clipboard* cb = GetClipboard();
    Editor* ed = GetEditor();

    if (cb == nil) {
        Selection* s = ed->GetSelection();

        if (s->IsEmpty()) {
            return;
        }

        SetClipboard(cb = new Clipboard);
	FrameIdrawView* top = (FrameIdrawView*)ed->GetViewer()->GetGraphicView();
	GraphicView* views = ((FrameEditor*)ed)->GetFrame();
        s->Sort(views);
        Iterator i;

        for (s->First(i); !s->Done(i); s->Next(i)) {
            s->GetView(i)->Interpret(this);
        }

    } else {
        Clipboard* oldcb = cb;
        SetClipboard(cb = new Clipboard);

        Iterator i;
        for (oldcb->First(i); !oldcb->Done(i); oldcb->Next(i)) {
            oldcb->GetComp(i)->Interpret(this);
        }
        delete oldcb;
    }

    if (!cb->IsEmpty()) {
        ed->GetComponent()->Interpret(this);
    }
}

/*****************************************************************************/

ClassId FrameBackCmd::GetClassId () { return FRAME_BACK_CMD; }
boolean FrameBackCmd::IsA (ClassId id) { return FRAME_BACK_CMD == id || BackCmd::IsA(id);}

FrameBackCmd::FrameBackCmd (ControlInfo* c) : BackCmd(c) { }
FrameBackCmd::FrameBackCmd (Editor* ed) : BackCmd(ed) { }

Command* FrameBackCmd::Copy () {
    Command* copy = new FrameBackCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void FrameBackCmd::Execute () {
    Clipboard* cb = GetClipboard();
    Editor* ed = GetEditor();

    if (cb == nil) {
        Selection* s = ed->GetSelection();

        if (s->IsEmpty()) {
            return;
        }

        SetClipboard(cb = new Clipboard);
	FrameIdrawView* top = (FrameIdrawView*)ed->GetViewer()->GetGraphicView();
	GraphicView* views = ((FrameEditor*)ed)->GetFrame();
        s->Sort(views);
        Iterator i;

        for (s->First(i); !s->Done(i); s->Next(i)) {
            s->GetView(i)->Interpret(this);
        }

    } else {
        Clipboard* oldcb = cb;
        SetClipboard(cb = new Clipboard);

        Iterator i;
        for (oldcb->First(i); !oldcb->Done(i); oldcb->Next(i)) {
            oldcb->GetComp(i)->Interpret(this);
        }
        delete oldcb;
    }

    if (!cb->IsEmpty()) {
        ed->GetComponent()->Interpret(this);
    }
}

/*****************************************************************************/

ClassId FrameCopyCmd::GetClassId () { return FRAME_COPY_CMD; }
boolean FrameCopyCmd::IsA (ClassId id) {
    return FRAME_COPY_CMD == id || CopyCmd::IsA(id);
}

FrameCopyCmd::FrameCopyCmd (ControlInfo* c, Clipboard* cb) : CopyCmd(c, cb) { }
FrameCopyCmd::FrameCopyCmd (Editor* ed, Clipboard* cb) : CopyCmd(ed, cb) { }

Command* FrameCopyCmd::Copy () {
    Command* copy = new FrameCopyCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void FrameCopyCmd::Execute () {
    Editor* editor = GetEditor();
    Selection* s = editor->GetSelection();

    if (!s->IsEmpty()) {
        Clipboard* cb = GetClipboard();
        cb = (cb == nil) ? unidraw->GetCatalog()->GetClipboard() : cb; 

        GraphicView* top = editor->GetViewer()->GetGraphicView();
	GraphicView* views = ((FrameEditor*)editor)->GetFrame();
        s->Sort(views);

        cb->DeleteComps();
        cb->CopyInit(s);
    }
}

/*****************************************************************************/

ClassId FrameNewViewCmd::GetClassId () { return OVNEWVIEW_CMD; }

boolean FrameNewViewCmd::IsA (ClassId id) {
    return FRAMENEWVIEW_CMD == id || OvNewViewCmd::IsA(id);
}

FrameNewViewCmd::FrameNewViewCmd (ControlInfo* c) : OvNewViewCmd(c) { }
FrameNewViewCmd::FrameNewViewCmd (Editor* ed) : OvNewViewCmd(ed) { }

Command* FrameNewViewCmd::Copy () {
    Command* copy = new FrameNewViewCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void FrameNewViewCmd::Execute () {
    Editor* ed = GetEditor();
    Editor* newEd = new FrameEditor((OverlayComp*)GetGraphicComp());

    *newEd->GetState("ModifStatusVar") = *ed->GetState("ModifStatusVar");

    unidraw->Open(newEd);
}


/*****************************************************************************/

ShowOtherFrameCmd::ShowOtherFrameCmd(ControlInfo* i, int offset)
: Command(i)
{
    _offset = offset;
    _old_offset = 0;
}

ShowOtherFrameCmd::ShowOtherFrameCmd(Editor* e, int offset)
: Command(e)
{
    _offset = offset;
    _old_offset = 0;
}

ClassId ShowOtherFrameCmd::GetClassId() { return SHOWOTHERFRAME_CMD; }
boolean ShowOtherFrameCmd::IsA(ClassId id) {
    return id == SHOWOTHERFRAME_CMD || Command::IsA(id);
}

Command* ShowOtherFrameCmd::Copy() {
    Command* copy = new ShowOtherFrameCmd(CopyControlInfo(), _offset);
    InitCopy(copy);
    return copy;
}

boolean ShowOtherFrameCmd::Reversible() { return true; }

void ShowOtherFrameCmd::Log () { ((OverlayUnidraw*)unidraw)->Log(this, false); }

void ShowOtherFrameCmd::Execute() {
  FrameEditor* ed = (FrameEditor*) GetEditor();
  _old_offset = ed->OtherFrame();
  ed->OtherFrame(_offset);
  ed->UpdateFrame();
  unidraw->Update();
}

void ShowOtherFrameCmd::Unexecute() {
  FrameEditor* ed = (FrameEditor*) GetEditor();
  ed->OtherFrame(_old_offset);
  ed->UpdateFrame();
  unidraw->Update();
}

/*****************************************************************************/ 
AutoNewFrameCmd* AutoNewFrameCmd::_default = nil;

AutoNewFrameCmd::AutoNewFrameCmd(ControlInfo* i)
: MacroCmd(i)
{
}

AutoNewFrameCmd::AutoNewFrameCmd(Editor* e)
: MacroCmd(e)
{
}

ClassId AutoNewFrameCmd::GetClassId() { return AUTONEWFRAME_CMD; }
boolean AutoNewFrameCmd::IsA(ClassId id) {
    return id == AUTONEWFRAME_CMD || Command::IsA(id);
}

Command* AutoNewFrameCmd::Copy() {
    Command* copy = new AutoNewFrameCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

boolean AutoNewFrameCmd::Reversible() { return true; }

void AutoNewFrameCmd::Log () { 
 boolean document_changed  = false;
  ((OverlayUnidraw*)unidraw)->Log(this, document_changed); 
}

void AutoNewFrameCmd::Execute() {
  FrameEditor* ed = (FrameEditor*) GetEditor();
  ed->ToggleAutoNewFrame();
}

void AutoNewFrameCmd::Unexecute() {
  FrameEditor* ed = (FrameEditor*) GetEditor();
  ed->ToggleAutoNewFrame();
}

implementActionCallback(AutoNewFrameCmd)

