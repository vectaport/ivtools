/*
 * Copyright (c) 1997 Vectaport Inc.
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
 * SlctByAttrCmd implementation.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/slctbyattr.h>

#include <ComGlyph/attrdialog.h>
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

ClassId SlctByAttrCmd::GetClassId () { return SLCT_BY_ATTR_CMD; }

boolean SlctByAttrCmd::IsA (ClassId id) {
    return SLCT_BY_ATTR_CMD == id || Command::IsA(id);
}

SlctByAttrCmd::SlctByAttrCmd (Editor* ed, AttrDialog* t) 
: Command(ed) { 
    Init(t);
}

SlctByAttrCmd::SlctByAttrCmd (ControlInfo* c, AttrDialog* t) 
: Command(c) { 
    Init(t);
}

void SlctByAttrCmd::Init(AttrDialog* t) {
    calculator_ = t ? t : &AttrDialog::instance();
    Resource::ref(calculator_);

    clipboard_ = new Clipboard();
    comps_ = nil;
}

Clipboard* SlctByAttrCmd::clipboard() { return clipboard_; }

void SlctByAttrCmd::Execute () { 
    Editor* ed = GetEditor();

    if (!comps_) {
        comps_ = (OverlaysComp*)ed->GetComponent();
	ComTerpServ* terp = calculator_->comterpserv();
	Iterator* iter = new Iterator;
      
        const char* nextcomm = "next_";
        terp->add_command(nextcomm, new NextAttrListFunc(terp, calculator_, comps_, iter, clipboard_));
        calculator_->next_expr(nextcomm);

        const char* truecomm = "true_";
        terp->add_command(truecomm, new TrueAttrListFunc(terp, calculator_, comps_, iter, clipboard_));
        calculator_->true_expr(truecomm);

        const char* falsecomm = "false_";
        terp->add_command(falsecomm, new FalseAttrListFunc(terp, calculator_, comps_, iter, clipboard_));
        calculator_->false_expr(falsecomm);

        const char* donecomm = "done_";
        terp->add_command(donecomm, new DoneAttrListFunc(terp, calculator_, comps_, iter, clipboard_, ed->GetViewer()));
        calculator_->done_expr(donecomm);
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

boolean SlctByAttrCmd::Reversible () { return false; }

Command* SlctByAttrCmd::Copy () {
    SlctByAttrCmd* copy = new SlctByAttrCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

Clipboard* SlctByAttrCmd::PostDialog () {
  return nil;
}


AttrListFunc::AttrListFunc(ComTerp* comterp, AttrDialog* attrdialog, 
			   OverlaysComp* comps, Iterator* i, 
			   Clipboard* cb, Selection* sel)
: ComFunc(comterp) 
{
    attrdialog_ = attrdialog;
    Resource::ref(attrdialog_);
    comps_ = comps;
    compiter_ = i;
    clipboard_ = cb;
    selection_ = sel;
}

AttrListFunc::~AttrListFunc() {
    Unref(attrdialog_);
}


/* move to next component in the list */

NextAttrListFunc::NextAttrListFunc
(ComTerp* comterp, AttrDialog* attrdialog, OverlaysComp* comps, Iterator* i, Clipboard* cb) 
: AttrListFunc(comterp, attrdialog, comps, i, cb) {
    comps_->First(*compiter_);
    OverlayComp* comp = (OverlayComp*)comps_->GetComp(*compiter_);
    if (comp)
        _comterp->set_attributes(comp->GetAttributeList());
}

void NextAttrListFunc::execute() { 
    comps_->Next(*compiter_);
    OverlayComp* comp = (OverlayComp*)comps_->GetComp(*compiter_);
     _comterp->set_attributes(comp ? comp->GetAttributeList() : nil);
    push_stack(comps_->Done(*compiter_) ? ComValue::falseval() : ComValue::trueval());
    return; 
}

/* if expression evaluates to true for this component, add to clipboard */

TrueAttrListFunc::TrueAttrListFunc
(ComTerp* comterp, AttrDialog* attrdialog, OverlaysComp* comps, Iterator* i, Clipboard* cb) 
: AttrListFunc(comterp, attrdialog, comps, i, cb) {}

void TrueAttrListFunc::execute() { 
    OverlayComp* comp = (OverlayComp*)comps_->GetComp(*compiter_);
    clipboard_->Append(comp);
    push_stack(ComValue::trueval());
    return; 
}

/* if expression evaluates to false for this component, do nothing */

FalseAttrListFunc::FalseAttrListFunc
(ComTerp* comterp, AttrDialog* attrdialog,  OverlaysComp* comps, Iterator* i, Clipboard* cb) 
: AttrListFunc(comterp, attrdialog, comps, i, cb) {}

void FalseAttrListFunc::execute() { 
    push_stack(ComValue::falseval());
    return; 
}

/* at the end build new selection from clipboard */

DoneAttrListFunc::DoneAttrListFunc
(ComTerp* comterp, AttrDialog* attrdialog, OverlaysComp* comps, Iterator* i, Clipboard* cb, Viewer* v) 
: AttrListFunc(comterp, attrdialog, comps, i, cb) {
    viewer_ = v;
}

void DoneAttrListFunc::execute() { 
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
}





