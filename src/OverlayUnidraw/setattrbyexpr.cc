/*
 * Copyright (c) 1998 Vectaport Inc.
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
 * SetAttrByExprCmd implementation.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/setattrbyexpr.h>

#include <ComGlyph/attrdialog.h>
#include <IVGlyph/gdialogs.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>
#include <InterViews/style.h>
#include <InterViews/window.h>

/*****************************************************************************/

ClassId SetAttrByExprCmd::GetClassId () { return SLCT_BY_ATTR_CMD; }

boolean SetAttrByExprCmd::IsA (ClassId id) {
    return SETATTRBYEXPR_CMD == id || Command::IsA(id);
}

SetAttrByExprCmd::SetAttrByExprCmd (Editor* ed, AttrDialog* t) 
: Command(ed) { 
    Init(t);
}

SetAttrByExprCmd::SetAttrByExprCmd (ControlInfo* c, AttrDialog* t) 
: Command(c) { 
    Init(t);
}

void SetAttrByExprCmd::Init(AttrDialog* t) {
    calculator_ = t ? t : &AttrDialog::instance();
    Resource::ref(calculator_);

    clipboard_ = new Clipboard();
    comps_ = nil;
}

Clipboard* SetAttrByExprCmd::clipboard() { return clipboard_; }

void SetAttrByExprCmd::Execute () { 
    Editor* ed = GetEditor();
    if (ed->GetSelection()->IsEmpty()) {
      GAcknowledgeDialog::post(ed->GetWindow(), "Compute Attributes:  Selection is empty", nil, "no selection");
      return;
    }

    if (!comps_) {
        comps_ = (OverlaysComp*)ed->GetComponent();
	ComTerpServ* terp = calculator_->comterpserv();
	Iterator* iter = new Iterator;
	
	const char* nextcomm = "next_";
	terp->add_command(nextcomm, new NextInSelectionFunc(terp, calculator_, GetEditor()->GetSelection(), iter));
	calculator_->next_expr(nextcomm);
#if 0
	
	const char* truecomm = "true_";
	terp->add_command(truecomm, new BothSetAttrFunc(terp, calculator_, comps_, iter, clipboard_));
	calculator_->true_expr(truecomm);
	
	const char* falsecomm = "false_";
	terp->add_command(falsecomm, new BothSetAttrFunc(terp, calculator_, comps_, iter, clipboard_));
	calculator_->false_expr(falsecomm);
	
	const char* donecomm = "done_";
	terp->add_command(donecomm, new DoneSetAttrFunc(terp, calculator_, comps_, iter, clipboard_, ed->GetViewer()));
	calculator_->done_expr(donecomm);

#endif
    }
    
    Style* style;
    boolean reset_caption = false;
    if (calculator_ == nil) {
        calculator_ = &AttrDialog::instance();
	Resource::ref(calculator_);
    } else {
	style = calculator_->style();
    }
    clipboard_->Clear();
    calculator_->post_for(ed->GetWindow());
    return;
}

boolean SetAttrByExprCmd::Reversible () { return false; }

Command* SetAttrByExprCmd::Copy () {
    SetAttrByExprCmd* copy = new SetAttrByExprCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

Clipboard* SetAttrByExprCmd::PostDialog () {
  return nil;
}


/* move to next component in the selection */

NextInSelectionFunc::NextInSelectionFunc
(ComTerp* comterp, AttrDialog* attrdialog, Selection* sel, Iterator* i) 
: AttrListFunc(comterp, attrdialog, nil, i, nil, sel) {

    selection_->First(*compiter_);
    OverlayView* view = (OverlayView*)selection_->GetView(*compiter_);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
    if (comp)
        _comterp->set_attributes(comp->GetAttributeList());
}

void NextInSelectionFunc::execute() { 
    selection_->Next(*compiter_);
    OverlayView* view = (OverlayView*)selection_->GetView(*compiter_);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
    _comterp->set_attributes(comp ? comp->GetAttributeList() : nil);
    push_stack(selection_->Done(*compiter_) ? ComValue::falseval() : ComValue::trueval());
    return; 
}

/* something to get called everytime, regardless if true or false value */

BothSetAttrFunc::BothSetAttrFunc
(ComTerp* comterp, AttrDialog* attrdialog, OverlaysComp* comps, Iterator* i, Clipboard* cb) 
: AttrListFunc(comterp, attrdialog, comps, i, cb) {}

void BothSetAttrFunc::execute() { 
#if 0 // copy of TrueAttrListFunc::execute
    OverlayComp* comp = (OverlayComp*)comps_->GetComp(*compiter_);
    clipboard_->Append(comp);
    push_stack(ComValue::trueval());
    return; 
#endif
}

/* at the end do nothing, at least nothing yet */

DoneSetAttrFunc::DoneSetAttrFunc
(ComTerp* comterp, AttrDialog* attrdialog, OverlaysComp* comps, Iterator* i, Clipboard* cb, Viewer* v) 
: AttrListFunc(comterp, attrdialog, comps, i, cb) {
    viewer_ = v;
}

void DoneSetAttrFunc::execute() { 
#if 0 // copy of DoneAttrListFunc::execute
    Iterator i;
    viewer_->GetSelection()->Clear();
    for (clipboard_->First(i); !clipboard_->Done(i); clipboard_->Next(i)) {
        OverlayComp* comp = (OverlayComp*)clipboard_->GetComp(i);
	OverlayView* view = comp->FindView(viewer_);
	if (view)
	    viewer_->GetSelection()->Append(view);
    }
    comps_->First(*compiter_);
    clipboard_->Clear();
    viewer_->Update();
    push_stack(ComValue::trueval());
    return; 
#endif
}




