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
#include <ComTerp/postfunc.h>
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

  /* stream literal: (val val ...) -- delegate to execute_literal() */
  if (nargs() > 1) {
    execute_literal();
    return;
  }

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

void StreamFunc::execute_literal() {
  /* Handle (val val ...) stream literal syntax.
     Scans _pfbuf to find per-element (offset, count) pairs,
     stores in AVL for lazy per-element evaluation by StreamLiteralNextFunc. */

  static StreamLiteralNextFunc* slnfunc = nil;
  if (!slnfunc) {
    slnfunc = new StreamLiteralNextFunc(comterp());
    slnfunc->funcid(symbol_add("streamliteralnext"));
  }

  /* find start of argument region in _pfbuf */
  ComValue argoffv(comterp()->stack_top());
  int offtop = argoffv.int_val() - comterp()->pfnum();
  int saved_offtop = offtop;
  int argcnt = 0;
  int total = 0;

  /* scan to find total tokens and bottom of arg region.
     nargsfixed() counts fixed-format args + keyword values.
     Compute true positional count by subtracting keyword value count. */
  for (int i = 0; i < nkeys(); i++) {
    argcnt = 0;
    skip_key_in_expr(offtop, argcnt);
    total += argcnt + 1;
  }
  /* nargsfixed() = nargs() - nargskey() already excludes keyword values */
  int npositionals = nargsfixed();
  for (int j = 0; j < npositionals; j++) {
    argcnt = 0;
    skip_arg_in_expr(offtop, argcnt);
    total += argcnt;
  }
  /* offtop is now the bottom of the entire arg region */

  /* copy entire arg region from _pfbuf */
  postfix_token* tokbuf = comterp()->copy_post_eval_expr(total, offtop);

  /* build AVL: [0]=FuncObj(tokbuf), [1]=nremaining (set after scan) */
  AttributeValueList* avl = new AttributeValueList();
  FuncObj* fo = new FuncObj(tokbuf, total);
  ComValue tokval(FuncObj::class_symid(), (void*)fo);
  avl->Append(new AttributeValue(tokval));
  avl->Append(new AttributeValue(0, AttributeValue::IntType)); /* nremaining */

  /* recording scan: start at saved_offtop.
     Keywords first (nkeys() of them), then fixed-format args until offtop. */
  int elem_offset = 0;
  int rescan = saved_offtop;
  int nelem = 0;

/* skip keywords to reach positionals */
  int keys_start = rescan;
  for (int ki = 0; ki < nkeys(); ki++) {
    argcnt = 0;
    skip_key_in_expr(rescan, argcnt);
  }

  /* fixed-format (positional) args first.
     skip_arg_in_expr walks the postfix buffer BACKWARD from the command,
     so pi=0 discovers the LAST source positional, pi=1 the second-to-last,
     etc -- discovery order is the reverse of source order.  tokbuf (from
     copy_post_eval_expr) is in FORWARD source order, so a naive running
     elem_offset assigns (offset,count) pairs as if discovery order matched
     tokbuf order -- correct only when all positionals are the same width
     (e.g. (10 20 30), or ((1 2 3)(4 5 6)) where both elements are width 3),
     and silently wrong for mixed-width elements like (1 (2 3)).

     Fix: collect each pi's size, then compute true forward offsets via a
     reverse-accumulation pass, and append to the AVL in reverse-pi order
     so AVL element 0 corresponds to source positional 0. */
  int* possizes = npositionals>0 ? new int[npositionals] : nil;
  for (int pi = 0; pi < npositionals; pi++) {
    argcnt = 0;
    skip_arg_in_expr(rescan, argcnt);
    possizes[pi] = argcnt;
  }
  int posoffsets_running = 0;
  for (int pi = npositionals-1; pi >= 0; pi--) {
    int off = posoffsets_running;
    posoffsets_running += possizes[pi];
    avl->Append(new AttributeValue(off, AttributeValue::IntType));
    avl->Append(new AttributeValue(possizes[pi], AttributeValue::IntType));
    nelem++;
  }
  elem_offset = posoffsets_running;
  delete [] possizes;

  /* keywords second */
  rescan = keys_start;
  for (int ki = 0; ki < nkeys(); ki++) {
    ComValue& keytoken = comterp()->pfcomvals()[comterp()->pfnum()-1+rescan];
    int key_symid = keytoken.keyid_val();
    int key_narg = keytoken.keynarg_val();
    argcnt = 0;
    skip_key_in_expr(rescan, argcnt);
    ComValue keymarker;
    keymarker.type(ComValue::KeywordType);
    keymarker.symbol_ref() = key_symid;
    keymarker.keynarg_ref() = key_narg;
    avl->Append(new AttributeValue(keymarker));
    if (key_narg > 0) {
      avl->Append(new AttributeValue(elem_offset, AttributeValue::IntType));
      avl->Append(new AttributeValue(argcnt, AttributeValue::IntType));
      elem_offset += argcnt;
    }
    nelem++;
  }

  /* set nremaining now that we know total element count */
  ((AttributeValue*)avl->Get(1))->int_ref() = nelem;

  reset_stack();

  ComValue stream(slnfunc, avl);
  stream.stream_mode(STREAM_INTERNAL);
  push_stack(stream);
}

/*****************************************************************************/

int SpreadFunc::_symid;

SpreadFunc::SpreadFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void SpreadFunc::execute() {
  ComValue operand1(stack_arg_post_eval(0));

  /* Normalize a bare list/attrlist/scalar into an internal stream exactly the
     way $$ (StreamFunc) does, so the drain loop below is uniform.  A stream
     operand is driven as-is. */
  if (!operand1.is_stream()) {
    static StreamNextFunc* snfunc = nil;
    if (!snfunc) {
      snfunc = new StreamNextFunc(comterp());
      snfunc->funcid(symbol_add("streamnext"));
    }
    AttributeValueList* avl;
    if (operand1.is_array())
      avl = new AttributeValueList(operand1.array_val());
    else {
      avl = new AttributeValueList();
      avl->Append(new AttributeValue(operand1));
    }
    ComValue stream(snfunc, avl);
    stream.stream_mode(STREAM_INTERNAL);
    operand1 = stream;
  }

  reset_stack();

  /* Tag for spread and leave exactly ONE value on the stack.  The expansion
     happens in eval_expr_internals, upstream of the command/funcobj dispatch,
     which drains this tagged stream into the enclosing call's positionals.  So
     ~~ obeys the one-value-per-func rule -- it never leaves the stack
     unbalanced -- and works for any consumer (eager command OR funcobj), not
     just the one dispatch branch push-N happened to patch.  The stream isn't
     drained here at all; it's just flagged and handed on. */
  operand1.stream_mode(operand1.stream_mode() | STREAM_SPREAD);
  push_stack(operand1);
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

  if (strmv.is_stream()) {
    /* explicit stream argument -- traverse normally */
    reset_stack();
    int cnt = 0;
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

  } else if (nargs() > 1) {
    /* implicit stream literal -- evaluate remaining fixed-format args.
       First arg already evaluated (strmv). Count it if non-nil. */
    int cnt = strmv.is_nil() ? 0 : 1;
    for (int i = 1; i < nargsfixed(); i++) {
      ComValue val(stack_arg_post_eval(i));
      if (!val.is_nil()) cnt++;
    }
    /* evaluate keyword args for side effects, count as attrlist elements */
    ComValue truedflt(ComValue::trueval());
    AttributeList* keys = stack_keys(false, truedflt);
    if (keys) {
      ALIterator ki;
      keys->First(ki);
      while (!keys->Done(ki)) { cnt++; keys->Next(ki); }
      delete keys;
    }
    reset_stack();
    ComValue retval(cnt, ComValue::IntType);
    push_stack(retval);

  } else {
    /* single non-stream arg -- error */
    fprintf(stderr, "Error: each() requires a stream argument (line %d)\n",
            funcstate()->linenum());
    reset_stack();
    push_stack(ComValue::nullval());
  }
    
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


/*****************************************************************************/

int StreamLiteralNextFunc::_symid = -1;

StreamLiteralNextFunc::StreamLiteralNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void StreamLiteralNextFunc::execute() {
    /* AVL layout:
         [0]       FuncObj wrapping postfix_token* (entire arg buffer)
         [1]       nremaining -- int, decremented each call
         [2..]     element entries -- (offset,count) for positionals,
                   (KeywordType,offset,count) or (KeywordType) for keywords
       Always re-navigate from AVL start to avoid stale iterators after Remove().
    */
    ComValue streamv(stack_arg(0));
    reset_stack();

    AttributeValueList* avl = streamv.stream_list();
    if (!avl) { push_stack(ComValue::nullval()); return; }

    /* navigate fresh from start each time */
    Iterator it;
    avl->First(it);

    /* [0] tokbuf */
    AttributeValue* tokval = avl->GetAttrVal(it);
    if (!tokval || !tokval->is_object(FuncObj::class_symid())) {
        push_stack(ComValue::nullval()); return;
    }
    FuncObj* fo = (FuncObj*)tokval->obj_val();
    postfix_token* tokbuf = fo->toks();

    /* [1] nremaining */
    avl->Next(it);
    AttributeValue* nremval = avl->GetAttrVal(it);
    int nrem = nremval->int_val();
    if (nrem <= 0) { push_stack(ComValue::nullval()); return; }

    /* [2] first element entry -- re-fetch iterator fresh */
    avl->Next(it);
    AttributeValue* firstval = avl->GetAttrVal(it);

    if (firstval->is_type(ComValue::KeywordType)) {
      /* keyword element -- build singleton attrlist (:key val) in C++ */
      int key_symid = firstval->keyid_val();
      int key_narg = firstval->keynarg_val();
      avl->Remove(firstval); delete firstval;

      ComValue keyval(ComValue::trueval()); /* bare flag defaults to true */
      if (key_narg > 0) {
        /* re-navigate fresh after remove */
        avl->First(it); avl->Next(it); avl->Next(it);
        AttributeValue* offval = avl->GetAttrVal(it);
        int offset = offval->int_val();
        avl->Remove(offval); delete offval;

        avl->First(it); avl->Next(it); avl->Next(it);
        AttributeValue* cntval = avl->GetAttrVal(it);
        int cnt = cntval->int_val();
        avl->Remove(cntval); delete cntval;

        keyval = comterpserv()->run(tokbuf + offset, cnt);
      }

      /* construct singleton attrlist (:key val) */
      AttributeList* al = new AttributeList();
      al->add_attr(key_symid, keyval);
      Resource::ref(al);
      ComValue result(AttributeList::class_symid(), (void*)al);
      nremval->int_ref()--;
      push_stack(result);
      return;
    }

    /* positional element -- (offset, count) pair */
    int offset = firstval->int_val();
    avl->Remove(firstval); delete firstval;

    /* re-navigate fresh for count */
    avl->First(it); avl->Next(it); avl->Next(it);
    AttributeValue* cntval = avl->GetAttrVal(it);
    int cnt = cntval->int_val();
    avl->Remove(cntval); delete cntval;

    /* lazy evaluation -- run exactly cnt tokens from tokbuf+offset */
    ComValue result(comterpserv()->run(tokbuf + offset, cnt));

    /* nil terminates stream early */
    if (result.is_nil()) {
        nremval->int_ref() = 0;
        push_stack(ComValue::nullval());
        return;
    }

    nremval->int_ref()--;
    push_stack(result);
}

/*****************************************************************************/

int InfoFunc::_symid = -1;

InfoFunc::InfoFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void InfoFunc::execute() {
  /* attrlst=info(streamobj [:raw]) -- inspect a stream's internal directory.

     Directory layout for StreamLiteralNextFunc-backed streams (the current
     diminishing-directory design):
       [0]    FuncObj(tokbuf, ntoks)
       [1]    nremaining  (starts at element count, decremented as consumed)
       [2..]  element entries: positional = (offset,count) = 2 slots;
              keyword = KeywordType marker [+ (offset,count) if it has a value]

     :raw returns the raw internal list directly -- layout-agnostic, so it works
     unchanged as the directory evolves and is the probe used by regression
     tests.  Without :raw, a named-field AttributeList is returned describing the
     directory in the layout above.  Non-literal streams report (:mode "external"
     :func `funcname) since their list layouts differ. */

  /* fetch :raw from the post-eval region BEFORE reset_stack() clears it.
     InfoFunc is post_eval, so the keyword lives in the post-eval buffer. */
  static int raw_symid = symbol_add("raw");
  ComValue rawv(stack_key_post_eval(raw_symid));
  boolean rawflag = rawv.is_true();

  ComValue streamv(stack_arg_post_eval(0));
  reset_stack();

  if (!streamv.is_stream()) {
    push_stack(ComValue::nullval());
    return;
  }

  AttributeValueList* avl = streamv.stream_list();

  /* :raw -- return the raw internal list directly */
  if (rawflag) {
    if (avl) {
      ComValue retval(avl);
      push_stack(retval);
    } else {
      push_stack(ComValue::nullval());
    }
    return;
  }

  /* identify a literal-backed stream; others have different list layouts */
  static int slnf_symid = -1;
  if (slnf_symid == -1) slnf_symid = symbol_add("streamliteralnext");
  ComFunc* sfunc = streamv.stream_func() ? (ComFunc*)streamv.stream_func() : nil;
  boolean is_literal = sfunc && sfunc->funcid() == slnf_symid;

  if (!avl || !is_literal) {
    AttributeList* al = new AttributeList();
    static int mode_sym = symbol_add("mode");
    static int func_sym = symbol_add("func");
    int sfunc_symid = sfunc ? sfunc->funcid() : symbol_add("unknown");
    ComValue modeval(is_literal ? "internal" : "external");
    ComValue funcval(sfunc_symid, ComValue::SymbolType);
    funcval.bquote(1);
    al->add_attr(mode_sym, modeval);
    al->add_attr(func_sym, funcval);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  /* field-aware report for the literal directory layout */
  AttributeList* al = new AttributeList();
  static int func_sym2  = symbol_add("func");
  static int ntoks_sym  = symbol_add("ntoks");
  static int nrem_sym   = symbol_add("nremaining");
  static int nelem_sym  = symbol_add("nelem");

  /* func name of the backing NextFunc -- back-quoted symbol */
  int sfunc_symid2 = sfunc ? sfunc->funcid() : symbol_add("unknown");
  ComValue* funcnamev = new ComValue(sfunc_symid2, ComValue::SymbolType);
  funcnamev->bquote(1);
  al->add_attr(func_sym2, funcnamev);

  /* [0] FuncObj -> ntoks */
  FuncObj* fo = avl->Number() > 0
    ? (FuncObj*)((AttributeValue*)avl->Get(0))->obj_val() : nil;
  ComValue ntoksv(fo ? fo->ntoks() : 0);
  al->add_attr(ntoks_sym, ntoksv);

  /* [1] nremaining */
  if (avl->Number() > 1) {
    ComValue nremv(*((AttributeValue*)avl->Get(1)));
    al->add_attr(nrem_sym, nremv);
  }

  /* [2..] element entries */
  int nelem = 0;
  int pos = 2;
  char keybuf[64];
  while (pos < avl->Number()) {
    AttributeValue* entry = (AttributeValue*)avl->Get(pos);
    if (entry->is_type(ComValue::KeywordType)) {
      /* keyword-with-value needs two trailing slots; stop before adding
         anything so a short tail leaves no orphaned key entry */
      if (entry->keynarg_val() > 0 && pos+2 >= avl->Number()) break;
      snprintf(keybuf, sizeof(keybuf), "key%d", nelem);
      ComValue kv(entry->keyid_val(), ComValue::SymbolType);
      al->add_attr(symbol_add(keybuf), kv);
      if (entry->keynarg_val() > 0) {
        ComValue kov(*((AttributeValue*)avl->Get(pos+1)));
        al->add_attr(symbol_add(keybuf), kov);
        snprintf(keybuf, sizeof(keybuf), "key%d_cnt", nelem);
        ComValue kcv(*((AttributeValue*)avl->Get(pos+2)));
        al->add_attr(symbol_add(keybuf), kcv);
        pos += 3;
      } else {
        pos += 1;
      }
    } else {
      /* positional needs one trailing slot (count); stop on a short tail */
      if (pos+1 >= avl->Number()) break;
      snprintf(keybuf, sizeof(keybuf), "elem%d_off", nelem);
      ComValue ov(*entry);
      al->add_attr(symbol_add(keybuf), ov);
      snprintf(keybuf, sizeof(keybuf), "elem%d_cnt", nelem);
      ComValue cv(*((AttributeValue*)avl->Get(pos+1)));
      al->add_attr(symbol_add(keybuf), cv);
      pos += 2;
    }
    nelem++;
  }

  ComValue nelemv(nelem);
  al->add_attr(nelem_sym, nelemv);

  ComValue retval(AttributeList::class_symid(), (void*)al);
  push_stack(retval);
}
