/*
 * Copyright (c) 2000 IET Inc.
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

#include <ComUnidraw/groupfunc.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovviewer.h>
#include <Unidraw/Commands/macro.h>
#include <Unidraw/Commands/struct.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <iostream.h>

/*****************************************************************************/

GrowGroupFunc::GrowGroupFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void GrowGroupFunc::execute() {
    ComValue groupval(stack_arg(0));
    ComValue grval(stack_arg(1));

    reset_stack();
    if (!groupval.object_compview() && !grval.object_compview()) return;

    OverlayViewer* viewer = (OverlayViewer*)GetEditor()->GetViewer();

    ComponentView* groupview = (ComponentView*)groupval.obj_val();
    OverlayComp* groupcomp = groupview ? (OverlayComp*)groupview->GetSubject() : nil;
 
    ComponentView* grview = (ComponentView*)grval.obj_val();
    OverlayComp* grcomp = grview ? (OverlayComp*)grview->GetSubject() : nil;

    if (groupcomp && grcomp) {

      /* first determine if group has non-zero members */
      Iterator i;
      groupcomp->First(i);
      if (groupcomp->Done(i)) {
	push_stack(ComValue::nullval());
	return;
      }

      MacroCmd* mcmd = new MacroCmd(GetEditor());

      /* ungroup */
      Clipboard* ucb = new Clipboard();
      ucb->Append(groupcomp);
      UngroupCmd* ucmd = new UngroupCmd(GetEditor());
      ucmd->SetClipboard(ucb);
      mcmd->Append(ucmd);

      /* regroup */
      Clipboard* gcb = new Clipboard();
      for(; !groupcomp->Done(i); groupcomp->Next(i))
	gcb->Append(groupcomp->GetComp(i));
      gcb->Append(grcomp);
      OvGroupCmd* gcmd = new OvGroupCmd(GetEditor());
      OverlaysComp* grgroup = new OverlaysComp();
      grgroup->SetAttributeList(groupcomp->attrlist());
      gcmd->SetGroup(grgroup);
      gcmd->SetClipboard(gcb);
      mcmd->Append(gcmd);

      execute_log(mcmd);
      ComValue retval(OverlaysComp::class_symid(), new ComponentView(grgroup));
      retval.object_compview(true);
      push_stack(retval);
      return;
    }

    push_stack(ComValue::nullval());
}

/*****************************************************************************/

TrimGroupFunc::TrimGroupFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void TrimGroupFunc::execute() {
    ComValue groupval(stack_arg(0));
    ComValue grval(stack_arg(1));

    reset_stack();
    if (!groupval.object_compview() && !grval.object_compview()) return;

    OverlayViewer* viewer = (OverlayViewer*)GetEditor()->GetViewer();

    ComponentView* groupview = (ComponentView*)groupval.obj_val();
    OverlayComp* groupcomp = groupview ? (OverlayComp*)groupview->GetSubject() : nil;
 
    ComponentView* grview = (ComponentView*)grval.obj_val();
    OverlayComp* grcomp = grview ? (OverlayComp*)grview->GetSubject() : nil;

    if (groupcomp && grcomp) {

      /* first determine if individual graphic really is in the group graphic */
      Iterator i;
      groupcomp->First(i);
      boolean found = false;
      while (!groupcomp->Done(i) && !found) {
	GraphicComp* subcomp = groupcomp->GetComp(i);
	if (subcomp==grcomp) found = true;
	groupcomp->Next(i);
      }
      if (!found) {
	push_stack(ComValue::nullval());
	return;
      }

      MacroCmd* mcmd = new MacroCmd(GetEditor());

      /* ungroup */
      Clipboard* ucb = new Clipboard();
      ucb->Append(groupcomp);
      UngroupCmd* ucmd = new UngroupCmd(GetEditor());
      ucmd->SetClipboard(ucb);
      mcmd->Append(ucmd);

      /* regroup */
      Clipboard* gcb = new Clipboard();
      for(groupcomp->First(i); !groupcomp->Done(i); groupcomp->Next(i))
	if (groupcomp->GetComp(i) != grcomp) 
	  gcb->Append(groupcomp->GetComp(i));
      OvGroupCmd* gcmd = new OvGroupCmd(GetEditor());
      OverlaysComp* grgroup = new OverlaysComp();
      grgroup->SetAttributeList(groupcomp->attrlist());
      gcmd->SetGroup(grgroup);
      gcmd->SetClipboard(gcb);
      mcmd->Append(gcmd);

      execute_log(mcmd);
      ComValue retval(OverlaysComp::class_symid(), new ComponentView(grgroup));
      retval.object_compview(true);
      push_stack(retval);
      return;
    }

    push_stack(ComValue::nullval());
}




