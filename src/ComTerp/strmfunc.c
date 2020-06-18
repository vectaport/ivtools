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
#include <ComTerp/comfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/iofunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Unidraw/iterator.h>

#define TITLE "StrmFunc"

/*****************************************************************************/

StrmFunc::StrmFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void StrmFunc::print_stream(std::ostream& out, AttributeValue& streamv) {
  static int indent=0;
  if(!streamv.is_stream()) 
    out << "NOT A STREAM\n";
  else {
    for (int i=0; i<indent; i++) out << ' ';
    out << "func = " << (streamv.stream_func() ? symbol_pntr(((ComFunc*)streamv.stream_func())->funcid()) : "NOFUNC")
              << ", avl = " << *streamv.stream_list() << "\n";
    if (streamv.stream_func()==NULL) 
      out << "Unexpected NOFUNC\n";
    Iterator it;
    streamv.stream_list()->First(it);
    int j=0;
    while(!streamv.stream_list()->Done(it)) {
      if (streamv.stream_list()->GetAttrVal(it)->is_stream()) {
        indent+=3;
        out << "depth=" << indent/3 << ", argc="  << j << ":  ";
        StrmFunc::print_stream(out, *streamv.stream_list()->GetAttrVal(it));
        indent-=3;
      }
      j++;
      streamv.stream_list()->Next(it);
    }
    out.flush();
  }
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
      snfunc->funcid(symbol_add("streamnext"));
    }

    if (operand1.is_array()) {
      AttributeValueList* avl = new AttributeValueList(operand1.array_val());
      ComValue stream(snfunc, avl);
      stream.stream_mode(STREAM_INTERNAL); // for internal use (use by this func)
      push_stack(stream);
    } 

    else if (operand1.is_attributelist()) {
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
      stream.stream_mode(STREAM_INTERNAL); // for internal use (use by this func)
      push_stack(stream);
    }

    else {
      AttributeValueList* avl = new AttributeValueList();
      avl->Append(new AttributeValue(operand1));
      ComValue stream(snfunc, avl);
      stream.stream_mode(STREAM_INTERNAL); // for internal use (use by this func)
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

    // if FileObj or PipeObj read next newline terminated string and return
    if (((ComValue*)retval)->is_fileobj() || ((ComValue*)retval)->is_pipeobj()) {
      ComValue fpobj((ComValue*)retval);
      comterp()->push_stack(fpobj);
      GetStringFunc func(comterp());
      func.exec(1,0);
      if (comterp()->stack_top().is_null()) {
	if (fpobj.is_fileobj()) {
	  FileObj *fileobj = (FileObj*)fpobj.geta(FileObj::class_symid());
	  fileobj->close();
	  avl->Remove(retval);
	  delete retval;
	} else if (fpobj.is_pipeobj()) {
	  PipeObj *pipeobj = (PipeObj*)fpobj.geta(PipeObj::class_symid());
	  pipeobj->close();
          avl->Remove(retval);
          delete retval;
	}
      }
      return;
    }

    // if ListType remove and return the front of the list
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
  stream.stream_mode(STREAM_INTERNAL); // for internal use (use by ConcatNextFunc)
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
	NextFunc::execute_impl(comterp(), valone, false);
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
	NextFunc::execute_impl(comterp(), valtwo, false);
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
    // fprintf(stderr, "RepeatFunc::execute nargs()=%d\n", nargs());
    ComValue operand1(stack_arg(0));

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

    ComValue operand2(stack_arg(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }

    int n = operand2.int_val();
    if (n<=0) return;

    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(operand1));
    avl->Append(new AttributeValue(operand2));
    ComValue stream(this, avl);
    stream.stream_mode(STREAM_INTERNAL); // for internal use (use by this func)
    push_stack(stream);
}

/*****************************************************************************/

IterateFunc::IterateFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void IterateFunc::execute() {
    // fprintf(stderr, "IterateFunc::execute nargs()=%d\n", nargs());
    ComValue operand1(stack_arg(0));

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

    ComValue operand2(stack_arg(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }

    int start = operand1.int_val();
    int stop = operand2.int_val();
    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(operand1));
    avl->Append(new AttributeValue(operand2));
    avl->Append(new AttributeValue(operand1));
    ComValue stream(this, avl);
    stream.stream_mode(STREAM_INTERNAL); // for internal use (use by this func)
    push_stack(stream);
}

/*****************************************************************************/

int NextFunc::_next_depth = 0;

NextFunc::NextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void NextFunc::execute() {
    ComValue streamv(stack_arg_post_eval(0));
    static int skim_symid = symbol_add("skim");
    ComValue skimflag(stack_key(skim_symid));
    reset_stack();

    execute_impl(comterp(), streamv, skimflag.is_true());
}

void NextFunc::execute_impl(ComTerp* comterp, ComValue& streamv, boolean skim) {

    _next_depth++;

    if (!streamv.is_stream()) {
      _next_depth--;
      return;
    }

    // handle nested stream
    if (!skim) {
      AttributeValueList* avl = streamv.stream_list();
      Iterator i;
      avl->First(i);
      if (!avl->Done(i)) {
	AttributeValue* val =  avl->GetAttrVal(i);
	if (val->is_stream() && val->stream_mode()&STREAM_NESTED) {
	  // fprintf(stderr, "NextFunc: Handling nested stream\n");
	  ComValue cval(*val);
	  NextFunc::execute_impl(comterp, cval, false);
	  if (!comterp->stack_top().is_null()) {
	    return;
	  }
	  avl->Remove(val);
          delete val;
	  comterp->pop_stack();
	}
      }
    }


    int outside_stackh = comterp->stack_height();

    // fprintf(stderr, "NextFunc:  stream:  mode=%d, name=%s, depth=%d\n", streamv.stream_mode(), symbol_pntr(((ComFunc*)streamv.stream_func())->funcid()), _next_depth);

    if (streamv.stream_mode()&STREAM_INTERNAL) {

      /* internal execution of next mechanism -- handled by stream func */
      comterp->push_stack(streamv);
      if(((ComFunc*)streamv.stream_func())->comterp()!=comterp) {
	((ComFunc*)streamv.stream_func())->comterp(comterp); // just in case
	 fprintf(stderr, "unexpected need to fix comterp in stream_func\n");
	 }
      ((ComFunc*)streamv.stream_func())->exec(1, 0);
      if (comterp->stack_top().is_null() && 
	  comterp->stack_height()>outside_stackh) 
	streamv.stream_list()->clear();
      else if (comterp->stack_height()==outside_stackh)
	comterp->push_stack(ComValue::blankval());

      // fprintf(stderr, "NextFunc: after next on internal stack top of type %s\n", comterp->stack_top().type_name());
      if (comterp->stack_top().is_stream()) {
	fprintf(stderr, "NextFunc:  Nested stream that could be further expanded, internal type\n");
      }

    } else if (streamv.stream_mode()&STREAM_EXTERNAL) {

      /* external execution of stream mechanism -- handled by this func */
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
	    if (val->stream_mode()&STREAM_INTERNAL && val->stream_func()) {
	      // fprintf(stderr, "NextFunc: handling internal mode stream argument\n");
	      /* internal use */
	      comterp->push_stack(*val);

              // fprintf(stdout, "Stack before stream_func exec\n");
              // comterp->print_stack();

	      ((ComFunc*)val->stream_func())->exec(1,0);

      	      if (comterp->stack_top().is_stream()) {
		fprintf(stderr, "NextFunc:  Nested stream that could be further expanded, internal argument type\n");
	      }

	    } else {
	      // fprintf(stderr, "NextFunc: handling external mode stream argument\n");

	      /* external use */
	      ComValue cval(*val);
              // fprintf(stderr, "before: strm arg 0x%lx, stack_top %d\n", val, comterp->stack_height());

              // fprintf(stdout, "Stack before NextFunc::execute_impl\n");
              // comterp->print_stack();

	      NextFunc::execute_impl(comterp, cval, false);
              // fprintf(stderr, "after:  strm arg 0x%lx, stack_top %d\n", val, comterp->stack_height());

	      if (comterp->stack_top().is_stream()) {
		fprintf(stderr, "NextFunc:  Nested stream that could be further expanded, external argument type\n");
	      }

	    }
	    
	    if (comterp->stack_top().is_null() && 
		comterp->stack_height()>inside_stackh) {
	      
	      /* sub-stream return null, zero it, and return null for this one */
	      val->stream_list()->clear();
	      streamv.stream_list()->clear();
	      while (comterp->stack_height()>outside_stackh) comterp->pop_stack();
	      comterp->push_stack(ComValue::nullval());
	      _next_depth--;
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

        // fprintf(stdout, "Stack before streamed func %s\n", symbol_pntr(funcptr->funcid()));
        // comterp->print_stack();

	funcptr->exec(narg, nkey);

	// recurse until not a stream
	while (comterp->stack_top().is_stream() && !skim) {
	  ComValue *newstream = new ComValue(comterp->pop_stack());
	  execute_impl(comterp, *newstream, false);

  	  // insert this stream at the front of the parent stream, to be recognized and dealt with by NextFunc
	  newstream->stream_mode(newstream->stream_mode()|STREAM_NESTED);
          streamv.stream_list()->Prepend(newstream);

	}
      }

      if (comterp->stack_top().is_null() &&
	  comterp->stack_height() > outside_stackh) 
	streamv.stream_list()->clear();
      else if (comterp->stack_height()==outside_stackh)
	comterp->push_stack(ComValue::blankval());

    } else 
      comterp->push_stack(ComValue::nullval());

    _next_depth--;
}


/*****************************************************************************/

EachFunc::EachFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void EachFunc::execute() {
  ComValue strmv(stack_arg_post_eval(0));
  static int skim_symid = symbol_add("skim");
  ComValue skimflag(stack_key(skim_symid));
  reset_stack();

  if (strmv.is_stream()) {
      
    int cnt = 0;
    /* traverse stream */
    boolean done = false;
    while (!done) {
      NextFunc::execute_impl(comterp(), strmv, skimflag.is_true());
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

/*****************************************************************************/

int FilterFunc::_symid;

FilterFunc::FilterFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void FilterFunc::execute() {
  ComValue streamv(stack_arg_post_eval(0));
  ComValue filterv(stack_arg_post_eval(1));
  reset_stack();

  /* setup for filtering */
  static FilterNextFunc* flfunc = nil;
  
  if (!flfunc) {
    flfunc = new FilterNextFunc(comterp());
    flfunc->funcid(symbol_add("filter"));
  }
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(streamv));
  avl->Append(new AttributeValue(filterv));
  ComValue stream(flfunc, avl);
  stream.stream_mode(STREAM_INTERNAL); // for internal use (use by FilterNextFunc)
  push_stack(stream);
}

/*****************************************************************************/

int FilterNextFunc::_symid;

FilterNextFunc::FilterNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void FilterNextFunc::execute() {
  ComValue operand1(stack_arg(0));

  /* invoked by next func */
  reset_stack();
  AttributeValueList* avl = operand1.stream_list();
  if (avl) {
    Iterator i;
    avl->First(i);
    AttributeValue* strmval = avl->GetAttrVal(i);
    avl->Next(i);
    AttributeValue* filterval = avl->GetAttrVal(i);
    
    /* filter stream */
    if (strmval->is_known()) {
      if (strmval->is_stream()) {

	boolean done = false;
	while(!done) {
	  ComValue strm2filt(*strmval);
	  NextFunc::execute_impl(comterp(), strm2filt, false);
	  if (comterp()->stack_top().is_unknown()) {
	    *strmval = ComValue::nullval();
	    push_stack(*strmval);
	    comterp()->pop_stack();
	    done = true;
	  } else {
	    if (comterp()->stack_top().is_object() &&
		comterp()->stack_top().class_symid()==filterval->symbol_val()) 
	      done = true;
	    else
	      comterp()->pop_stack();
	  }
	}

      } else {
	push_stack(*strmval);
	*strmval = ComValue::nullval();
      }
    }
    
  } else
    push_stack(ComValue::nullval());

  return;
}

