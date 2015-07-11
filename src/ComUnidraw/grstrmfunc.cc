/*
 * Copyright (c) 2011 Wave Semiconductor
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

#include <ComUnidraw/grstrmfunc.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <Attribute/attrlist.h>
#include <Unidraw/iterator.h>

#define TITLE "GrStrmFunc"

/*****************************************************************************/

int GrStreamFunc::_symid;

GrStreamFunc::GrStreamFunc(ComTerp* comterp) : StreamFunc(comterp) {
}

void GrStreamFunc::execute() {
  ComValue convertv(stack_arg_post_eval(0));
  
  if (convertv.object_compview()) {
    reset_stack();
    
    static StreamNextFunc* snfunc = nil;
    if (!snfunc) {
      snfunc = new StreamNextFunc(comterp());
      snfunc->funcid(symbol_add("stream"));
    }

    AttributeValueList* avl = new AttributeValueList();
    Component* comp = ((ComponentView*)convertv.obj_val())->GetSubject();
    if (!comp->IsA(OVERLAYS_COMP)) {
      push_stack(ComValue::nullval());
      return;
    }
    OverlaysComp* ovcomps = (OverlaysComp*)comp;
    Iterator it;
    for(ovcomps->First(it); !ovcomps->Done(it); ovcomps->Next(it)) {
      OverlayComp* subcomp = (OverlayComp*) ovcomps->GetComp(it);
      AttributeValue* av = 
        new AttributeValue(new OverlayViewRef(subcomp), subcomp->classid());
      avl->Append(av);
    }
    ComValue stream(snfunc, avl);
    stream.stream_mode(-1); // for internal use (use by this func)
    push_stack(stream);
    
  } else {
    
    StreamFunc strmfunc(comterp());
    strmfunc.exec(funcstate()->nargs(), funcstate()->nkeys(), pedepth());
    return;
    
  }
  
}

#if 0  
  if (convertv.is_stream()) {
    
    /* stream copy */
    AttributeValueList* old_avl = convertv.stream_list();
    AttributeValueList* new_avl = new AttributeValueList(old_avl);
    ComValue retval(convertv.stream_func(), new_avl);
    retval.stream_mode(convertv.stream_mode());
    push_stack(retval);
    
  } else {
    
    /* conversion operator */

    static StreamNextFunc* snfunc = nil;
    if (!snfunc) {
      snfunc = new StreamNextFunc(comterp());
      snfunc->funcid(symbol_add("stream"));
    }

    if (convertv.is_array()) {
      AttributeValueList* avl = new AttributeValueList(convertv.array_val());
      ComValue stream(snfunc, avl);
      stream.stream_mode(-1); // for internal use (use by this func)
      push_stack(stream);
    } else if (convertv.is_attributelist()) {
      AttributeValueList* avl = new AttributeValueList();
      AttributeList* al = (AttributeList*)convertv.obj_val();
      Iterator i;
      for(al->First(i); !al->Done(i); al->Next(i)) {
	Attribute* attr = al->GetAttr(i);
	AttributeValue* av = 
	  new AttributeValue(Attribute::class_symid(), (void*)attr);
	avl->Append(av);
      }
      ComValue stream(snfunc, avl);
      stream.stream_mode(-1); // for internal use (use by this func)
      push_stack(stream);
    }
    
  }

#endif

/*****************************************************************************/

int GrDepthFunc::_symid;

GrDepthFunc::GrDepthFunc(ComTerp* comterp) : StreamFunc(comterp) {
}

void GrDepthFunc::execute() {
  ComValue compsv(stack_arg_post_eval(0));
  reset_stack();
  
  if (compsv.object_compview()) {
    
    static GrDepthNextFunc* snfunc = nil;
    if (!snfunc) {
      snfunc = new GrDepthNextFunc(comterp());
      snfunc->funcid(symbol_add("depthnext"));
    }

    AttributeValueList* avl = new AttributeValueList();
    Component* comp = ((ComponentView*)compsv.obj_val())->GetSubject();
    if (!comp->IsA(OVERLAYS_COMP)) {
      push_stack(ComValue::nullval());
      return;
    }
    OverlaysComp* ovcomps = (OverlaysComp*)comp;

    ComValue* av = 
      new ComValue(new OverlayViewRef(ovcomps), ovcomps->classid());
    avl->Append(av);
    avl->Append(new ComValue());
    ComValue* av2 = 
      new ComValue(new OverlayViewRef((OverlaysComp*)ovcomps->GetParent()), ovcomps->classid());
    avl->Append(av2);
    
    ComValue stream(snfunc, avl);
    stream.stream_mode(-1); // for internal use (use by this func)
    stream.type(ComValue::StreamType);
    push_stack(stream);
    
  }
}

/*****************************************************************************/

int GrDepthNextFunc::_symid;

GrDepthNextFunc::GrDepthNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void GrDepthNextFunc::execute() {
  ComValue operand1(stack_arg(0));

  /* invoked by next func */
  reset_stack();
  AttributeValueList* avl = operand1.stream_list();
  if (avl) {
    Iterator i;
    avl->First(i);
    ComValue* currval = (ComValue*)avl->GetAttrVal(i);
    avl->Next(i);
    ComValue* prevval = (ComValue*)avl->GetAttrVal(i);
    avl->Next(i);
    ComValue* stopval = (ComValue*)avl->GetAttrVal(i);
    
    /* depth-first seach */
    if (currval->is_known()) {
      OverlayComp* currcomp = (OverlayComp*)currval->geta(OverlayComp::class_symid(), OVERLAY_COMP);
      OverlayComp* prevcomp = (OverlayComp*)prevval->geta(OverlayComp::class_symid(), OVERLAY_COMP);
      OverlayComp* stopcomp = (OverlayComp*)stopval->geta(OverlayComp::class_symid(), OVERLAY_COMP);
      OverlayComp* nextcomp = currcomp->DepthNext(prevcomp);
      while(nextcomp && nextcomp==(OverlayComp*)currcomp->GetUp()) {
	prevcomp=currcomp;
	currcomp=nextcomp;
        nextcomp = currcomp->DepthNext(prevcomp);
      }
      if(nextcomp && nextcomp!=stopcomp) {
        ((OverlayViewRef*)currval->obj_val())->SetSubject(nextcomp);
        if (!prevcomp) {
          ComValue new_prevval(new OverlayViewRef(currcomp), currcomp->classid());;
          *prevval = new_prevval;
	} else
	  ((OverlayViewRef*)prevval->obj_val())->SetSubject(currcomp);
        ComValue av(new OverlayViewRef(nextcomp), nextcomp->classid());
        push_stack(av);
        return;
      }
    }
    
  }
  push_stack(ComValue::nullval());

  return;
}

