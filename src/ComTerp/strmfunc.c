/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 1994-1997 Vectaport Inc.
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

#include <ComTerp/strmfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Unidraw/iterator.h>

#define TITLE "StrmFunc"

#define STREAM_MECH

/*****************************************************************************/

StrmFunc::StrmFunc(ComTerp* comterp) : ComFunc(comterp) {
}

/*****************************************************************************/

int StreamFunc::_symid;

StreamFunc::StreamFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void StreamFunc::execute() {
  ComValue operand1(stack_arg_post_eval(0));
  
  reset_stack();
  
  if (operand1.is_stream()) {
    
    /* stream copy */
    AttributeValueList* old_avl = operand1.stream_list();
    AttributeValueList* new_avl = new AttributeValueList(old_avl);
    ComValue retval(operand1.stream_func(), new_avl);
    retval.stream_mode(operand1.stream_mode());
    push_stack(retval);
    
  } else {
    
    /* conversion operator */

    static StreamNextFunc* snfunc = nil;
    if (!snfunc) {
      snfunc = new StreamNextFunc(comterp());
      snfunc->funcid(symbol_add("stream"));
    }

    if (operand1.is_array()) {
      AttributeValueList* avl = new AttributeValueList(operand1.array_val());
      ComValue stream(snfunc, avl);
      stream.stream_mode(-1); // for internal use (use by this func)
      push_stack(stream);
    } else if (operand1.is_attributelist()) {
      AttributeValueList* avl = new AttributeValueList();
      AttributeList* al = (AttributeList*)operand1.obj_val();
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
}

/*****************************************************************************/

int StreamNextFunc::_symid;

StreamNextFunc::StreamNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void StreamNextFunc::execute() {
  ComValue operand1(stack_arg(0));
  
  reset_stack();
  
  /* invoked by the next command */
  AttributeValueList* avl = operand1.stream_list();
  if (avl) {
    Iterator i;
    avl->First(i);
    AttributeValue* retval = avl->Done(i) ? nil : avl->GetAttrVal(i);
    if (retval) {
      push_stack(*retval);
      avl->Remove(retval);
      delete retval;
    } else {
      operand1.stream_list(nil);
      push_stack(ComValue::nullval());
    }
  } else
    push_stack(ComValue::nullval());
  
}

/*****************************************************************************/

int ConcatFunc::_symid;

ConcatFunc::ConcatFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void ConcatFunc::execute() {
  ComValue operand1(stack_arg_post_eval(0));
  ComValue operand2(stack_arg_post_eval(1));
  reset_stack();

  /* setup for concatenation */
  static ConcatNextFunc* cnfunc = nil;
  
  if (!cnfunc) {
    cnfunc = new ConcatNextFunc(comterp());
    cnfunc->funcid(symbol_add("concat"));
  }
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(operand1));
  avl->Append(new AttributeValue(operand2));
  ComValue stream(cnfunc, avl);
  stream.stream_mode(-1); // for internal use (use by ConcatNextFunc)
  push_stack(stream);
}

/*****************************************************************************/

int ConcatNextFunc::_symid;

ConcatNextFunc::ConcatNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void ConcatNextFunc::execute() {
  ComValue operand1(stack_arg(0));

  /* invoked by next func */
  reset_stack();
  AttributeValueList* avl = operand1.stream_list();
  if (avl) {
    Iterator i;
    avl->First(i);
    AttributeValue* oneval = avl->GetAttrVal(i);
    avl->Next(i);
    AttributeValue* twoval = avl->GetAttrVal(i);
    boolean done = false;
    
    /* stream first argument until nil */
    if (oneval->is_known()) {
      if (oneval->is_stream()) {
	ComValue valone(*oneval);
	NextFunc::execute_impl(comterp(), valone);
	if (comterp()->stack_top().is_unknown()) {
	  *oneval = ComValue::nullval();
	  comterp()->pop_stack();
	} else
	  done = true;
      } else {
	push_stack(*oneval);
	*oneval = ComValue::nullval();
	done = true;
      }
    }
    
    /* stream 2nd argument until nil */
    if (twoval->is_known() && !done) {
      if (twoval->is_stream()) {
	ComValue valtwo(*twoval);
	NextFunc::execute_impl(comterp(), valtwo);
	if (comterp()->stack_top().is_unknown())
	  *twoval = ComValue::nullval();
      } else {
	push_stack(*twoval);
	*twoval = ComValue::nullval();
      }
    } else if (!done) 
      push_stack(ComValue::nullval());
    
  } else
    push_stack(ComValue::nullval());

  return;
}

/*****************************************************************************/

RepeatFunc::RepeatFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void RepeatFunc::execute() {
    ComValue operand1(stack_arg(0));

#ifdef STREAM_MECH
    if (operand1.is_stream() && nargs()==1) {
      reset_stack();
      AttributeValueList* avl = operand1.stream_list();
      if (avl) {
	Iterator i;
	avl->First(i);
	AttributeValue* repval = avl->GetAttrVal(i);
	avl->Next(i);
	AttributeValue* cntval = avl->GetAttrVal(i);
	if (cntval->int_val()>0)
	  push_stack(*repval);
	else
	  push_stack(ComValue::nullval());
	cntval->int_ref()--;
      } else
	push_stack(ComValue::nullval());
      return;
    } else if (operand1.is_stream()) {
      fprintf(stderr, "no more than doubly nested streams supported as of yet\n");
      push_stack(ComValue::nullval());
      return;
    }
#endif

    ComValue operand2(stack_arg(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }

    int n = operand2.int_val();
    if (n<=0) return;

#ifdef STREAM_MECH
    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(operand1));
    avl->Append(new AttributeValue(operand2));
    ComValue stream(this, avl);
    stream.stream_mode(-1); // for internal use (use by this func)
    push_stack(stream);
#else
    AttributeValueList* avl = new AttributeValueList();
    for (int i=0; i<n; i++) 
        avl->Append(new ComValue(operand1));
    ComValue array(avl);
    push_stack(array);
#endif
}

/*****************************************************************************/

IterateFunc::IterateFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void IterateFunc::execute() {
    ComValue operand1(stack_arg(0));

#ifdef STREAM_MECH
    if (operand1.is_stream() && nargs()==1) {
      reset_stack();
      AttributeValueList* avl = operand1.stream_list();
      if (avl) {
	Iterator i;
	avl->First(i);
	AttributeValue* startval = avl->GetAttrVal(i);
	avl->Next(i);
	AttributeValue* stopval = avl->GetAttrVal(i);
	avl->Next(i);
	AttributeValue* nextval = avl->GetAttrVal(i);
	push_stack(*nextval);
	if (nextval->int_val()==stopval->int_val()) 
	  *nextval = ComValue::nullval();
	else {
	  if (startval->int_val()<=stopval->int_val())
	    nextval->int_ref()++;
	  else
	    nextval->int_ref()--;
	}
      } else
	push_stack(ComValue::nullval());
      return;
    } else if (operand1.is_stream()) {
      fprintf(stderr, "no more than doubly nested streams supported as of yet\n");
      push_stack(ComValue::nullval());
      return;
    }
#endif

    ComValue operand2(stack_arg(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }

    int start = operand1.int_val();
    int stop = operand2.int_val();
#ifdef STREAM_MECH
    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(operand1));
    avl->Append(new AttributeValue(operand2));
    avl->Append(new AttributeValue(operand1));
    ComValue stream(this, avl);
    stream.stream_mode(-1); // for internal use (use by this func)
    push_stack(stream);
#else
    int dir = start>stop ? -1 : 1;

    AttributeValueList* avl = new AttributeValueList();
    for (int i=start; i!=stop; i+=dir) 
        avl->Append(new ComValue(i, ComValue::IntType));
    avl->Append(new ComValue(stop, ComValue::IntType));
    ComValue array(avl);
    push_stack(array);
#endif
}

/*****************************************************************************/

NextFunc::NextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void NextFunc::execute() {
    ComValue streamv(stack_arg_post_eval(0));
    reset_stack();

    execute_impl(comterp(), streamv);
}

void NextFunc::execute_impl(ComTerp* comterp, ComValue& streamv) {

    if (!streamv.is_stream()) return;

    int outside_stackh = comterp->stack_height();

    if (streamv.stream_mode()<0) {

      /* internal execution -- handled by stream func */
      comterp->push_stack(streamv);
      ((ComFunc*)streamv.stream_func())->exec(1, 0);
      if (comterp->stack_top().is_null() && 
	  comterp->stack_height()>outside_stackh) 
	streamv.stream_list()->clear();
      else if (comterp->stack_height()==outside_stackh)
	comterp->push_stack(ComValue::blankval());

    } else if (streamv.stream_mode()>0) {

      /* external execution -- handled by this func */
      ComFunc* funcptr = (ComFunc*)streamv.stream_func();
      AttributeValueList* avl = streamv.stream_list();
      int narg=0;
      int nkey=0;
      if (funcptr && avl) {
	Iterator i;
	avl->First(i);
	while (!avl->Done(i)) {
	  AttributeValue* val =  avl->GetAttrVal(i);

	  if (val->is_stream()) {

	    int inside_stackh = comterp->stack_height();

	    /* stream argument, use stream func to get next one */
	    if (val->stream_mode()<0 && val->stream_func()) {
	      /* internal use */
	      comterp->push_stack(*val);
	      ((ComFunc*)val->stream_func())->exec(1,0);
	    }else {

	      /* external use */
	      ComValue cval(*val);
	      NextFunc::execute_impl(comterp, cval);

	    }
	    
	    if (comterp->stack_top().is_null() && 
		comterp->stack_height()>inside_stackh) {
	      
	      /* sub-stream returnnull, zero it, and return null for this one */
	      val->stream_list()->clear();
	      streamv.stream_list()->clear();
	      while (comterp->stack_height()>outside_stackh) comterp->pop_stack();
	      comterp->push_stack(ComValue::nullval());
	      return;
	    } else if (comterp->stack_height()==inside_stackh)
	      comterp->push_stack(ComValue::blankval());

	    narg++;

	  } else {

	    /* non-stream argument, push as is */
	    comterp->push_stack(*val);
	    if (val->is_key()) 
	      nkey++;
	    else
	      narg++;

	  }
	  avl->Next(i);
	}

	funcptr->exec(narg, nkey);
      }

      if (comterp->stack_top().is_null() &&
	  comterp->stack_height() > outside_stackh) 
	streamv.stream_list()->clear();
      else if (comterp->stack_height()==outside_stackh)
	comterp->push_stack(ComValue::blankval());

    } else 
      comterp->push_stack(ComValue::nullval());
}


/*****************************************************************************/

EachFunc::EachFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void EachFunc::execute() {
  ComValue strmv(stack_arg_post_eval(0));
  reset_stack();

  if (strmv.is_stream()) {
      
    int cnt = 0;
    /* traverse stream */
    boolean done = false;
    while (!done) {
      NextFunc::execute_impl(comterp(), strmv);
      if (comterp()->pop_stack().is_unknown())
	done = true;
      else
	cnt++;
    }

    ComValue retval(cnt, ComValue::IntType);
    push_stack(retval);

  } else 
    push_stack(ComValue::nullval());
    
}

