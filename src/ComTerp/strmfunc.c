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
#include <ComTerp/listfunc.h>
#include <ComTerp/postfunc.h>
#include <ComTerp/postfixspan.h>
#include <ComTerp/socket.h>
#include <ComTerp/timefunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Unidraw/iterator.h>
#include <algorithm>
#include <sstream>
#include <vector>

#define TITLE "StrmFunc"

/* forward decl: InfoFunc::execute() identifies a ring stream by comparing
   against this singleton, defined down by FeedFunc/RingNextFunc below */
static RingNextFunc* ring_next_func(ComTerp* comterp);
/* forward decl: InfoFunc::execute() reports a ring's free slot count via
   this, defined down alongside the other ring helpers below */
static int ring_avail(AttributeValueList* avl);
/* forward decl: MoreFunc::execute_impl() peeks a ring via this, defined
   down alongside the other ring helpers below */
static ComValue ring_peek_char(AttributeValueList* avl);

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

boolean StrmFunc::is_delimiter(ComValue& val) {
  static int eos_symid = symbol_add("EOS");
  return val.is_symbol() && val.bquote() && val.symbol_val()==eos_symid;
}

/*****************************************************************************/


StreamFunc::StreamFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void StreamFunc::execute() {

  /* stream literal: (val val ...) -- delegate to execute_literal(); use
     nargstotal() so a bare keyword counts (see doc/POSTFIX-INDEXING.md) */
  if (nargstotal() > 1) {
    execute_literal();
    return;
  }

  static StreamNextFunc* snfunc = nil;
  if (!snfunc) {
    snfunc = new StreamNextFunc(comterp());
    snfunc->funcid(symbol_add("streamnext"));
  }

  if (nargs() == 0) {
    /* empty stream literal: [] -- returns nil on first next() */
    reset_stack();
    AttributeValueList* avl = new AttributeValueList();
    ComValue stream(snfunc, avl);
    stream.stream_mode(STREAM_INTERNAL);
    push_stack(stream);
    return;
  }

  ComValue operand1(stack_arg_post_eval(0));

  reset_stack();

  push_stream_from_value(operand1);
}

/* Build a character cursor over a string (see StringNextFunc for the layout).
   Shared by stream()'s conversion and by feed()'s ingestion so the two cannot
   drift about what streaming a string means. */
static ComValue string_stream_value(ComTerp* comterp, ComValue& strv) {
  static StringNextFunc* strnfunc = nil;
  if (!strnfunc) {
    strnfunc = new StringNextFunc(comterp);
    strnfunc->funcid(symbol_add("stringnext"));
  }
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(strv));
  avl->Append(new AttributeValue(0, AttributeValue::IntType));
  ComValue stream(strnfunc, avl);
  stream.stream_mode(STREAM_INTERNAL);
  return stream;
}

/* is this an argument a string should be taken apart for?  StringType only,
   though is_string() would also admit a symbol: a symbol is a name rather than
   text, and the `EOS delimiters that ride through feeds are symbols whose
   whole purpose is to arrive intact.  Bquote is the per-value escape -- it
   already means "do not take this apart" -- and marks the value rather than
   the call, so one argument can be protected while another is ingested. */
static boolean streams_as_characters(ComValue& v) {
  return v.is_type(ComValue::StringType) && !v.bquote();
}

void StreamFunc::push_stream_from_value(ComValue& operand1) {

  static StreamNextFunc* snfunc = nil;
  if (!snfunc) {
    snfunc = new StreamNextFunc(comterp());
    snfunc->funcid(symbol_add("streamnext"));
  }

  if (operand1.is_stream()) {

    /* stream copy */
    AttributeValueList* old_avl = operand1.stream_list();
    AttributeValueList* new_avl = new AttributeValueList(old_avl);
    ComValue retval(operand1.stream_func(), new_avl);
    retval.stream_mode(operand1.stream_mode());
    push_stack(retval);

  } else {

    /* conversion operator */

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

    else if (streams_as_characters(operand1)) {
      ComValue stream(string_stream_value(comterp(), operand1));
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

int StringNextFunc::_symid = -1;

StringNextFunc::StringNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

/* Cursor over a string's own bytes.  The stream list carries two slots:
     [0]  the string itself, which keeps the bytes reachable
     [1]  the index of the next character
   Nothing is copied to streamify: the read side does not write, so the cursor
   holds the value rather than duplicating its characters, and boxes one
   ComValue per pull instead of the whole string up front the way split() does.
   at() indexes the same bytes the same way (listfunc.c's is_string branch). */
void StringNextFunc::execute() {
  ComValue streamv(stack_arg(0));
  reset_stack();

  AttributeValueList* avl = streamv.stream_list();
  if (!avl) { push_stack(ComValue::nullval()); return; }

  Iterator it;
  avl->First(it);
  AttributeValue* strval = avl->GetAttrVal(it);
  if (!strval || !strval->is_type(AttributeValue::StringType)) {
    push_stack(ComValue::nullval()); return;
  }
  avl->Next(it);
  AttributeValue* posval = avl->GetAttrVal(it);
  if (!posval) { push_stack(ComValue::nullval()); return; }

  const char* str = strval->string_ptr();
  int pos = posval->int_val();
  if (!str || pos < 0 || pos >= (int)strlen(str)) {
    push_stack(ComValue::nullval());
    return;
  }
  posval->int_ref()++;
  ComValue retval(*(str+pos), ComValue::CharType);
  push_stack(retval);
}

/*****************************************************************************/

void StreamFunc::execute_literal() {
  /* Handle (val val ...) literal syntax: scan _pfbuf for per-element
     (offset,count), store in AVL for lazy eval by StreamLiteralNextFunc. */

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

  /* scan to find total tokens and bottom of arg region;
     nargsfixed() counts fixed-format args + keyword values */
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

  /* recording scan: start at saved_offtop, keywords first
     (nkeys() of them), then fixed-format args until offtop */
  int elem_offset = 0;
  int rescan = saved_offtop;
  int nelem = 0;

/* skip keywords to reach positionals */
  int keys_start = rescan;
  for (int ki = 0; ki < nkeys(); ki++) {
    argcnt = 0;
    skip_key_in_expr(rescan, argcnt);
  }

  /* fixed-format args first: skip_arg_in_expr discovers them in reverse,
     so accumulate reverse to get forward offsets; AVL[0] = positional 0 */
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

  /* Scan keywords backward, then append them in source order.
     Advance offsets past each value and its keyword tag. */
  rescan = keys_start;
  int* keysizes = nkeys()>0 ? new int[nkeys()] : nil;
  int* keysymids = nkeys()>0 ? new int[nkeys()] : nil;
  int* keynargs = nkeys()>0 ? new int[nkeys()] : nil;
  for (int ki = 0; ki < nkeys(); ki++) {
    ComValue& keytoken = comterp()->pfcomvals()[comterp()->pfnum()-1+rescan];
    keysymids[ki] = keytoken.keyid_val();
    keynargs[ki] = keytoken.keynarg_val();
    argcnt = 0;
    skip_key_in_expr(rescan, argcnt);
    keysizes[ki] = argcnt;
  }
  for (int ki = nkeys()-1; ki >= 0; ki--) {
    ComValue keymarker;
    keymarker.type(ComValue::KeywordType);
    keymarker.symbol_ref() = keysymids[ki];
    keymarker.keynarg_ref() = keynargs[ki];
    avl->Append(new AttributeValue(keymarker));
    if (keynargs[ki] > 0) {
      avl->Append(new AttributeValue(elem_offset, AttributeValue::IntType));
      avl->Append(new AttributeValue(keysizes[ki], AttributeValue::IntType));
      elem_offset += keysizes[ki];
    }
    elem_offset++;
    nelem++;
  }
  delete [] keysizes;
  delete [] keysymids;
  delete [] keynargs;

  /* set nremaining now that we know total element count */
  ((AttributeValue*)avl->Get(1))->int_ref() = nelem;

  reset_stack();

  ComValue stream(slnfunc, avl);
  stream.stream_mode(STREAM_INTERNAL);
  push_stack(stream);
}

/*****************************************************************************/


SpreadFunc::SpreadFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void SpreadFunc::execute() {
  ComValue operand1(stack_arg_post_eval(0));

  /* Normalize a bare list/attrlist/scalar into an internal stream exactly
     the way $$ (StreamFunc) does, so the drain loop below is uniform */
  if (!operand1.is_stream()) {
    static StreamNextFunc* snfunc = nil;
    if (!snfunc) {
      snfunc = new StreamNextFunc(comterp());
      snfunc->funcid(symbol_add("streamnext"));
    }
    AttributeValueList* avl;
    if (operand1.is_array())
      avl = new AttributeValueList(operand1.array_val());
    else if (operand1.is_attributelist()) {
      /* an attrlist spreads into keywords: copy each Attribute now,
         since the source may be gone by the time a ~~ stream drains */
      avl = new AttributeValueList();
      AttributeList* al = (AttributeList*)operand1.obj_val();
      Iterator i;
      for(al->First(i); !al->Done(i); al->Next(i)) {
	Attribute* attrcopy = new Attribute(*al->GetAttr(i));
	avl->Append(new AttributeValue(Attribute::class_symid(), (void*)attrcopy));
      }
    }
    else {
      avl = new AttributeValueList();
      avl->Append(new AttributeValue(operand1));
    }
    ComValue stream(snfunc, avl);
    stream.stream_mode(STREAM_INTERNAL);
    operand1 = stream;
  }

  reset_stack();

  /* tag for spread and leave exactly one value on the stack;
     eval_expr_internals drains it into the caller's positionals later */
  operand1.stream_mode(operand1.stream_mode() | STREAM_SPREAD);
  push_stack(operand1);
}

/*****************************************************************************/

EchoFunc::EchoFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void EchoFunc::execute() {
  int npos = nargsfixed();

  /* capture the evaluated positional values into a list */
  AttributeValueList* poslist = npos > 0 ? new AttributeValueList() : nil;
  for (int i = 0; i < npos; i++)
    poslist->Append(new AttributeValue(stack_arg(i)));

  /* capture the keywords as an attrlist (each add_attr copies the value) */
  AttributeList* keys = stack_keys();
  boolean has_kw = keys && keys->Number() > 0;

  reset_stack();

  if (npos > 0) {
    /* positionals present -> a list; keywords, if any, become one
       single-attribute attrlist per keyword at the TAIL, preserving order */
    if (has_kw) {
      Iterator it;
      for (keys->First(it); !keys->Done(it); keys->Next(it)) {
	Attribute* a = keys->GetAttr(it);
	AttributeList* singleton = new AttributeList();
	singleton->add_attr(a->SymbolId(), *a->Value());
	poslist->Append(new AttributeValue(AttributeList::class_symid(), (void*)singleton));
      }
    }
    delete keys;  /* copied into the singletons above (or unused); we own it */
    ComValue retval(poslist);
    push_stack(retval);

  } else if (has_kw) {
    /* no positionals -> return the multi-attribute attrlist bare (adopts keys) */
    ComValue retval(AttributeList::class_symid(), keys);
    push_stack(retval);

  } else {
    delete keys;  /* empty and unused */
    push_stack(ComValue::nullval());
  }
}

/*****************************************************************************/


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

    // if FileObj or PipeObj read next newline terminated string and return;
    // retval is base-class AttributeValue, so don't cast it to ComValue*
    if (retval && (retval->is_object(FileObj::class_symid()) ||
		   retval->is_object(PipeObj::class_symid()))) {
      ComValue fpobj(*retval);
      comterp()->push_stack(fpobj);
      GetStringFunc func(comterp());
      func.funcid(symbol_add("getstringnext"));
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
    cnfunc->funcid(symbol_add("concatnext"));
  }
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(operand1));
  avl->Append(new AttributeValue(operand2));
  ComValue stream(cnfunc, avl);
  stream.stream_mode(STREAM_INTERNAL); // for internal use (use by ConcatNextFunc)
  push_stack(stream);
}

/*****************************************************************************/


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
    }

    /* no bail-out for a stream operand: overdrive already unwrapped a level,
       so bailing would unbalance the stack; build the repeat stream instead */

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

ReplayFunc::ReplayFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void ReplayFunc::execute() {
    /* post_eval: take the WHOLE stream operand (like ConcatFunc), not
       a per-element broadcast; %% N builds the internal replay stream */
    ComValue operand1(stack_arg_post_eval(0));
    ComValue operand2(stack_arg_post_eval(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    int n = operand2.int_val();
    if (n <= 0) {           // no passes -> empty stream; don't allocate one (cf. RepeatFunc)
      push_stack(ComValue::nullval());
      return;
    }

    static ReplayNextFunc* rnfunc = nil;
    if (!rnfunc) {
      rnfunc = new ReplayNextFunc(comterp());
      rnfunc->funcid(symbol_add("replaynext"));
    }
    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(operand1));                    // [0] source stream (template, never consumed)
    avl->Append(new AttributeValue(n, AttributeValue::IntType));  // [1] passes remaining
    avl->Append(new AttributeValue(ComValue::nullval()));         // [2] current pass copy (a stream, or nil)
    ComValue stream(rnfunc, avl);
    stream.stream_mode(STREAM_INTERNAL); // driven by ReplayNextFunc
    push_stack(stream);
}

/*****************************************************************************/

ReplayNextFunc::ReplayNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void ReplayNextFunc::execute() {
    ComValue operand1(stack_arg(0));   // our internal replay stream

    /* invoked by the next mechanism */
    reset_stack();
    AttributeValueList* avl = operand1.stream_list();
    if (avl) {
      Iterator i;
      avl->First(i);
      AttributeValue* srcval = avl->GetAttrVal(i);   // [0] source stream (template, never consumed)
      avl->Next(i);
      AttributeValue* cntval = avl->GetAttrVal(i);   // [1] passes remaining
      avl->Next(i);
      AttributeValue* curval = avl->GetAttrVal(i);   // [2] current pass copy (a stream, or nil)

      for (;;) {
	/* start of a pass: need a fresh $$-copy of the source */
	if (!curval->is_stream()) {
	  if (cntval->int_val() <= 0 || !srcval->is_stream()) {
	    push_stack(ComValue::nullval());           // all passes done (or non-stream source)
	    return;
	  }
	  /* $$ copy semantics: new AVL (independent cursor), same stream func,
	     sharing the source's immutable elements -- source untouched */
	  AttributeValueList* newavl = new AttributeValueList(srcval->stream_list());
	  ComValue copyv(srcval->stream_func(), newavl);
	  copyv.stream_mode(srcval->stream_mode());
	  *curval = copyv;
	  cntval->int_ref()--;
	}
	/* pull the next element from the current copy */
	ComValue curcopy(*curval);
	NextFunc::execute_impl(comterp(), curcopy);
	if (comterp()->stack_top().is_unknown()) {
	  comterp()->pop_stack();                      // this pass exhausted
	  *curval = ComValue::nullval();               // force a fresh copy next time
	  continue;                                    // ...and start the next pass
	}
	return;   // the value from execute_impl is already on the stack
      }
    } else
      push_stack(ComValue::nullval());
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
    }

    /* no bail-out for a stream operand --
       same reasoning as RepeatFunc's above */

    ComValue operand2(stack_arg(1));
    reset_stack();

    if (operand1.is_nil() || operand2.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }

    /* a non-stream, non-numeric operand (e.g. ArrayType) must not fall through
       to int_val()/int_ref(): corrupts an ArrayType/ObjectType heap pointer */
    if (!operand1.is_num() || !operand2.is_num()) {
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

/* backing lists of the streams a live chain of execute_impl calls is
   currently unwinding, innermost last.  Nesting can run arbitrarily deep,
   and any two backing lists in the chain can be the same object, so
   membership is checked across the whole chain, not just the immediate
   caller. */
static std::vector<AttributeValueList*> _draining_avls;

/* set to the backing list the recursive-stream guard above matched, so a
   null it produces can be told apart from a nested element's own,
   legitimate exhaustion -- the two look identical on the stack (both a
   pushed nullval()) but call for opposite handling: a legitimate nil means
   forget this nested element and move on, while a guard-tripped nil means
   the whole pull was refused and nothing it touched should be mutated.
   Identifying it by avl, not just a boolean, keeps an unrelated drain's
   trip (some other ring, pulled as a side effect while fulfilling this
   one) from being misread as this drain's own: a trip is only "mine" if
   the avl it names is still a live ancestor of the call reading it --
   which, for the ring a trip actually concerns, is guaranteed for as long
   as that ring's own DrainingAVLGuard is still on the stack, and false
   again once an unrelated trip's guard has already unwound. */
static AttributeValueList* _draining_guard_tripped_avl = 0;

struct DrainingAVLGuard {
  DrainingAVLGuard(AttributeValueList* avl) { _draining_avls.push_back(avl); }
  ~DrainingAVLGuard() { _draining_avls.pop_back(); }
};

NextFunc::NextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void NextFunc::execute() {
    boolean have_var = nargsfixed()>1;
    /* unevaluated (symbol=true), the same way AssignFunc reads its own lhs --
       a plain var name, not whatever value it currently holds. */
    ComValue varname(have_var ? stack_arg(1, true) : ComValue::nullval());

    if (have_var && varname.type() != ComValue::SymbolType) {
      /* not a bare name -- maybe at()/global()/local()/temp(): same lhs
	 detection AssignFunc uses on its own arg 0, walked here for this
	 call's arg 1 instead. Flagging the command token before evaluating
	 it is what makes a streamed at() (e.g. r@0..3) come back as a
	 settable stream rather than a plain read; this must happen, and
	 arg 1 must be fully evaluated, before arg 0 is touched at all --
	 same ordering constraint AssignFunc's own lhs has on its arg 0. */
      int linenum = funcstate()->linenum();
      static int global_symid = symbol_add("global");
      static int local_symid = symbol_add("local");
      static int temp_symid = symbol_add("temp");
      static int at_symid = symbol_add("at");
      static int each_symid = symbol_add("each");
      ComValue argoff(comterp()->stack_top());
      int offtop = argoff.int_val() - comterp()->pfnum();
      int argcnt = 0;
      for (int j=nargsfixed(); j>1; j--) { argcnt = 0; skip_arg_in_expr(offtop, argcnt); }
      /* the walked-to arg's own root token sits at offtop+argcnt, not offtop --
	 offtop is left pointing one-past the far end of its subtree. */
      int startidx = comterp()->pfnum() + offtop + argcnt - 1;
      ComValue& startval = comterp()->pfcomvals()[startidx];
      boolean from_each = false;
      if (startval.is_type(ComValue::CommandType)) {
	ComFunc* func = (ComFunc*)startval.obj_val();
	if (func->funcid() == each_symid) {
	  startval.lhs_assign(1);
	  from_each = true;
	} else if (func->funcid() == global_symid || func->funcid() == local_symid ||
		   func->funcid() == temp_symid || func->funcid() == at_symid)
	  startval.lhs_assign(1);
      }
      varname = stack_arg_post_eval(1, true);
      ComValue streamv(stack_arg_post_eval(0));
      reset_stack();
      return execute_var_dispatch(streamv, varname, linenum, from_each);
    }

    /* a bare symbol var (or none given) -- unchanged from before the lhs
       machinery above existed, including argument evaluation order. */
    ComValue streamv(stack_arg_post_eval(0));
    reset_stack();
    execute_impl(comterp(), streamv);
    if (varname.is_type(ComValue::SymbolType)) {
      ComValue* pulled = new ComValue(comterp()->stack_top());
      comterp()->write_funcscope_symval(varname.symbol_val(), pulled);
    }
}

/* dispatch for a non-symbol, lhs-eligible var -- a streamed at() (zipper), a
   scalar at() pair, or a dot() attribute.  Split out of execute() so the
   plain-symbol path above can keep evaluating its two arguments in the
   original order; this path's own arg 1 is already fully evaluated by the
   time it's called. */
void NextFunc::execute_var_dispatch(ComValue& streamv, ComValue& varname, int linenum,
				     boolean from_each) {
    if (varname.is_stream() && varname.lhs_assign()) {
      /* a streamed at()-destination or an each()-wrapped stream -- zip-
	 drive the source against it (zip_assign_stream) and return the
	 write/pull count, not a single value. */
      ComValue idxstream(varname);
      int count = NextFunc::zip_assign_stream(comterp(), idxstream, &streamv, true, linenum,
					       from_each);
      if (count < 0) {
	push_stack(ComValue::nullval());
	return;
      }
      ComValue retval(count, ComValue::IntType);
      push_stack(retval);
      comterp()->stack_top().wrapper(AttributeValue::BracketWrapper);
      return;
    }

    if (varname.is_array() && varname.lhs_assign()) {
      /* var is a single at()-destination (e.g. r@2) -- pull one value,
	 write it there, and still return the pulled value itself, same
	 contract as the plain-symbol case in execute(). */
      execute_impl(comterp(), streamv);
      ComValue pulled(comterp()->stack_top());
      NextFunc::write_at_pair(comterp(), varname, pulled);
      return;
    }

    if (varname.is_object(Attribute::class_symid())) {
      /* a dot()-destination -- write the pulled value into it, same as
	 AssignFunc's al.field=val, reporting nil instead on a self-
	 insertion refusal rather than the unwritten value. */
      execute_impl(comterp(), streamv);
      ComValue pulled(comterp()->stack_top());
      Attribute* attr = (Attribute*)varname.obj_val();
      AttributeList* owner = attr->Owner();
      if (owner && value_contains_container(pulled, (void*)owner, true)) {
	fprintf(stderr, "WARNING: refusing to insert an attrlist into itself -- line %d\n",
		linenum);
	reset_stack();
	push_stack(ComValue::nullval());
	return;
      }
      attr->Value(new ComValue(pulled));
      return;
    }

    if (varname.is_stream() && !varname.lhs_assign()) {
      /* default "stream builds stream": var is a plain stream, so defer
	 into a lazy wrapper (NextVarNextFunc) instead of draining here. */
      static NextVarNextFunc* nvnfunc = nil;
      if (!nvnfunc) {
	nvnfunc = new NextVarNextFunc(comterp());
	nvnfunc->funcid(symbol_add("nextvarnext"));
      }
      AttributeValueList* avl = new AttributeValueList();
      avl->Append(new AttributeValue(streamv));
      avl->Append(new AttributeValue(varname));
      ComValue stream(nvnfunc, avl);
      stream.stream_mode(STREAM_INTERNAL);
      push_stack(stream);
      return;
    }

    /* flagged but none of the above -- e.g. a sealed attrlist's dot lookup
       (unknown()&&lhs_assign()) -- leaves next(stream) unchanged, same as
       an unrecognized lhs falls through in AssignFunc (assignfunc.c). */
    execute_impl(comterp(), streamv);
}

NextVarNextFunc::NextVarNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void NextVarNextFunc::execute() {
    ComValue operand1(stack_arg(0));

    /* invoked by the next mechanism */
    reset_stack();
    AttributeValueList* avl = operand1.stream_list();
    if (avl) {
      Iterator i;
      avl->First(i);
      AttributeValue* srcval = avl->GetAttrVal(i);  // [0] source stream
      avl->Next(i);
      AttributeValue* varval = avl->GetAttrVal(i);  // [1] var's own stream

      /* var's stream paces this one in lockstep, same pairing as the
	 eager zip_assign_stream loop -- its pulled value is discarded
	 (var was never a writable destination here), but either side
	 running dry ends the lazy sequence. */
      ComValue varcopy(*varval);
      NextFunc::execute_impl(comterp(), varcopy);
      if (comterp()->stack_top().is_unknown()) {
	comterp()->pop_stack();
	push_stack(ComValue::nullval());
	return;
      }
      comterp()->pop_stack();

      ComValue srccopy(*srcval);
      NextFunc::execute_impl(comterp(), srccopy);
      return;   // the value from execute_impl is already on the stack
    } else
      push_stack(ComValue::nullval());
}

void NextFunc::execute_impl(ComTerp* comterp, ComValue& streamv) {

    _next_depth++;

    if (!streamv.is_stream()) {
      _next_depth--;
      return;
    }

    if (_draining_avls.empty()) _draining_guard_tripped_avl = 0;

    AttributeValueList* self_avl = streamv.stream_list();
    if (std::find(_draining_avls.begin(), _draining_avls.end(), self_avl)
        != _draining_avls.end()) {
      fprintf(stderr, "WARNING: recursive stream -- next() returning nil instead of pulling forever\n");
      comterp->push_stack(ComValue::nullval());
      _draining_guard_tripped_avl = self_avl;
      _next_depth--;
      return;
    }
    DrainingAVLGuard draining_guard(self_avl);

    /* handle nested stream -- looped, not recursed, so a long run of
       exhausted elements can't blow the C++ call stack */
    {
      AttributeValueList* avl = streamv.stream_list();
      for (;;) {
	Iterator i;
	avl->First(i);
	if (avl->Done(i)) break;
	AttributeValue* val =  avl->GetAttrVal(i);
	/* stream_mode_raw(), not stream_mode() -- the latter reports 0 once
	   this nested stream's own list is empty (see attrvalue.c) */
	if (!(val->is_stream() && val->stream_mode_raw()&STREAM_NESTED)) break;
	// fprintf(stderr, "NextFunc: Handling nested stream\n");
	ComValue cval(*val);
	NextFunc::execute_impl(comterp, cval);
	/* a trip naming a live ancestor is this call's own refusal, not the
	   nested element running dry -- leave it in place for a later call. */
	if (_draining_guard_tripped_avl &&
	    std::find(_draining_avls.begin(), _draining_avls.end(), _draining_guard_tripped_avl)
	    != _draining_avls.end()) {
	  _next_depth--;
	  return;
	}
	if (!comterp->stack_top().is_null()) {
	  _next_depth--;
	  return;
	}
	avl->Remove(val);
        delete val;
	comterp->pop_stack();
	/* loop back: the new front element may itself be
	   STREAM_NESTED and needs unwrapping too */
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
      /* a ring's avl is head/tail/count bookkeeping, not remaining elements --
	 nil is "empty for now," so clearing it would destroy a feedable ring. */
      /* stream_mode_raw(), not stream_mode(): the latter reads back 0 once
	 this stream's own list is empty (attrvalue.c), masking STREAM_RING. */
      if (comterp->stack_top().is_null() &&
	  comterp->stack_height()>outside_stackh &&
	  !(streamv.stream_mode_raw()&STREAM_RING))
	streamv.stream_list()->clear();
      else if (comterp->stack_height()==outside_stackh)
	comterp->push_stack(ComValue::blankval());

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
	      /* internal use -- routed through execute_impl (not the stream
	         func directly), so a feed() holding a stream yields one value/pull */
	      ComValue cval(*val);
	      NextFunc::execute_impl(comterp, cval);

	    } else {
	      // fprintf(stderr, "NextFunc: handling external mode stream argument\n");

	      /* external use */
	      ComValue cval(*val);
              // fprintf(stderr, "before: strm arg 0x%lx, stack_top %d\n", val, comterp->stack_height());

              // fprintf(stdout, "Stack before NextFunc::execute_impl\n");
              // comterp->print_stack();

	      NextFunc::execute_impl(comterp, cval);
              // fprintf(stderr, "after:  strm arg 0x%lx, stack_top %d\n", val, comterp->stack_height());

	    }
	    
	    if (comterp->stack_top().is_null() && 
		comterp->stack_height()>inside_stackh) {
	      
	      /* sub-stream return null, zero it, and return null for this one --
		 unless it's a ring, whose avl is persistent bookkeeping rather
		 than remaining elements (see the STREAM_INTERNAL branch above) */
	      if (!(val->stream_mode_raw()&STREAM_RING))
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

	if (streamv.stream_mode()&STREAM_FUNCOBJ) {
	  /* the packed callee is a FuncObj, not a registered command --
	     fire the body via fire_funcobj instead of exec'ing */
	  ComValue fobjv(FuncObj::class_symid(), (void*)funcptr);
	  fobjv.narg(narg);
	  fobjv.nkey(nkey);
	  comterp->fire_funcobj(fobjv);
	} else {
	  if (streamv.lhs_assign()) {
	    /* reasserts the lvalue signal ListAtFunc reads off a stale stack
	       slot (see ARCHITECTURE.md, "at()'s lhs flag") -- that slot is
	       long gone by replay time, so push one flagged throwaway value
	       and pop it right back off to leave a fresh one in its place. */
	    ComValue sentinel(ComValue::nullval());
	    sentinel.lhs_assign(1);
	    comterp->push_stack(sentinel);
	    comterp->pop_stack(false);
	  }
	  funcptr->exec(narg, nkey);
	}

	// recurse until not a stream
	while (comterp->stack_top().is_stream()) {
	  ComValue *newstream = new ComValue(comterp->pop_stack());
	  execute_impl(comterp, *newstream);

  	  // insert this stream at the front of the parent stream, to be recognized and dealt with by NextFunc
	  newstream->stream_mode(newstream->stream_mode()|STREAM_NESTED);
          streamv.stream_list()->Prepend(newstream);

	}
      }

      if (comterp->stack_top().is_null() &&
	  comterp->stack_height() > outside_stackh &&
	  !(streamv.stream_mode_raw()&STREAM_RING))
	streamv.stream_list()->clear();
      else if (comterp->stack_height()==outside_stackh)
	comterp->push_stack(ComValue::blankval());

    } else 
      comterp->push_stack(ComValue::nullval());

    _next_depth--;
}

MoreFunc::MoreFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void MoreFunc::execute() {
  /* symbol=true -- suppress the default auto-resolve (ComTerp::pop_stack's
     is_symbol()/is_attribute() handling) so a dot expression (al.field)
     hands back the Attribute DotFunc found (dotfunc.c:622) rather than
     its value, and a bare name stays a SymbolType -- either way, a handle
     to WHERE the stream lives, not just a copy of what it held a moment
     ago.  Needed because a stream's own avl pointer, though shared by an
     ordinary variable read, does NOT survive an attribute's read-then-
     auto-resolve copy (ComTerp::pop_stack, comterp.c:1513-1516) -- so the
     restash below has to be written back to that handle explicitly, not
     left to aliasing. */
  ComValue rawarg(stack_arg_post_eval(0, true));
  reset_stack();

  Attribute* attr = nil;
  ComValue streamv;
  if (rawarg.is_object(Attribute::class_symid())) {
    attr = (Attribute*)rawarg.obj_val();
    streamv = *attr->Value();
  } else if (rawarg.is_type(ComValue::SymbolType)) {
    streamv = comterp()->lookup_symval(rawarg);
  } else {
    streamv = rawarg;
  }

  if (!streamv.is_stream()) {
    push_stack(ComValue::nullval());
    return;
  }

  ComValue peeked(execute_impl(comterp(), streamv));

  /* write the (possibly restashed) stream back to wherever it came from,
     so a second more() before the first next() -- or next() itself --
     reads it fresh from there instead of counting on streamv's own avl
     pointer to still be reachable from that same place. */
  if (attr) {
    attr->Value(new ComValue(streamv));
  } else if (rawarg.is_type(ComValue::SymbolType))
    comterp()->write_funcscope_symval(rawarg.symbol_val(), new ComValue(streamv));

  push_stack(peeked);
}

ComValue MoreFunc::execute_impl(ComTerp* comterp, ComValue& streamv) {
  if (streamv.stream_mode_raw()&STREAM_RING)
    /* random-access over its own buffer -- read without popping, so
       there's nothing to lose and nothing to restash. */
    return ring_peek_char(streamv.stream_list());

  /* no random access -- the only way to see the next value is to pull it
     for real, same state mutation next() itself would cause. */
  NextFunc::execute_impl(comterp, streamv);
  ComValue peeked(comterp->pop_stack());
  if (peeked.is_null() || peeked.is_unknown())
    /* exhausted, or an ongoing stream's not-yet blank tick -- either way
       nothing was produced to hold onto. */
    return peeked;

  /* restash the peeked value at the front of this SAME stream object (its
     avl is a shared pointer, not a copy of whatever variable/expression
     referenced it) -- the identical STREAM_NESTED+Prepend idiom
     execute_impl's own nested-stream handling uses above, so the next
     real pull against this stream, however it's later reached, drains
     this one-shot wrapper first and re-delivers the peeked value. */
  static StreamNextFunc* snfunc = nil;
  if (!snfunc) {
    snfunc = new StreamNextFunc(comterp);
    snfunc->funcid(symbol_add("streamnext"));
  }
  AttributeValueList* peekavl = new AttributeValueList();
  peekavl->Append(new AttributeValue(peeked));
  ComValue* peekstream = new ComValue(snfunc, peekavl);
  peekstream->stream_mode(STREAM_INTERNAL|STREAM_NESTED);
  streamv.stream_list()->Prepend(peekstream);

  return peeked;
}

/* whether at()'s :set would actually write (list target :set) vs. silently
   no-op and return nil (a fixed-capacity string index out of range) --
   nil is also a legitimate written value, so the write's own return can't
   tell the two apart; this mirrors ListAtFunc::execute()'s own bounds
   check (listfunc.c) on the same target/index pair. */
static boolean idxassign_in_range(ComValue& listv, int idx) {
  if (idx < 0) return false;
  if (listv.is_type(ComValue::ArrayType)) return true;
  if (listv.is_only_string()) {
    boolean isslice = listv.sliced();
    int cap = isslice ? listv.slicelen() : symbol_len(listv.string_val());
    int chunksz = listv.blocksz();
    int nchunks = chunksz>0 ? cap/chunksz : cap;
    return idx < nchunks;
  }
  return false;
}

ComValue NextFunc::write_at_pair(ComTerp* comterp, ComValue& pairv, ComValue& writeval) {
  AttributeValueList* pair = pairv.array_val();
  static int set_symid = symbol_add("set");
  comterp->push_stack(*pair->Get(0));
  comterp->push_stack(*pair->Get(1));
  comterp->push_stack(writeval);
  ComValue setkey(set_symid, 1);
  comterp->push_stack(setkey);
  ListAtFunc atfunc(comterp);
  atfunc.funcid(symbol_add("at"));
  /* narg counts non-keyword args including the value after a keyword --
     4 pushes here mean narg=3, nkey=1, not narg=2 */
  atfunc.exec(3, 1);
  return comterp->pop_stack();
}

int NextFunc::zip_assign_stream(ComTerp* comterp, ComValue& idxstream,
				 ComValue* rhsval, boolean rhs_is_stream,
				 int linenum, boolean tolerate_nonpair) {
  int count = 0;
  for (;;) {
    NextFunc::execute_impl(comterp, idxstream);
    ComValue pairv(comterp->pop_stack());
    if (pairv.is_null()) break;
    AttributeValueList* pair = pairv.array_val();
    boolean is_pair = pairv.is_array() && pairv.lhs_assign() && pair && pair->Number()==2;
    if (!is_pair && !tolerate_nonpair) {
      /* the streamed target isn't list/string-shaped (e.g. an attrlist) --
	 at() has nothing writable to hand back for it, same as the
	 non-streamed al@n=val case (test 5, atop.comt): no effect.  -1
	 (not a partial count) matches the non-streamed case's hard nil. */
      fprintf(stderr, "WARNING:  assignment to something other than a symbol or attribute (%s) ignored -- line %d\n",
	      symbol_pntr(pairv.type_symid()), linenum);
      return -1;
    }
    ComValue writeval;
    if (rhs_is_stream) {
      NextFunc::execute_impl(comterp, *rhsval);
      ComValue tick(comterp->pop_stack());
      if (tick.is_null()) break;
      writeval = tick;
    } else
      writeval = *rhsval;
    if (is_pair) {
      ComValue targetv(*pair->Get(0));
      if (!idxassign_in_range(targetv, pair->Get(1)->int_val()))
	/* any nil ends a stream -- an out-of-range index is where at()
	   itself would start returning nil, so the write stream ends
	   here too, the same as the read-side index stream would. */
	break;
      write_at_pair(comterp, pairv, writeval);
    }
    /* !is_pair here only reaches this point when tolerate_nonpair is set --
       an each()-wrapped, never-assignable target (e.g. true**4): nothing
       to write, but the pull still counts. */
    count++;
  }
  return count;
}

AssignAtNextFunc::AssignAtNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void AssignAtNextFunc::execute() {
    ComValue operand1(stack_arg(0));
    reset_stack();
    AttributeValueList* avl = operand1.stream_list();
    if (!avl) {
      push_stack(ComValue::nullval());
      return;
    }
    Iterator i;
    avl->First(i);
    AttributeValue* idxval = avl->GetAttrVal(i);   // [0] write-index stream
    avl->Next(i);
    AttributeValue* rhsval = avl->GetAttrVal(i);   // [1] rhs value or stream

    ComValue idxcopy(*idxval);
    NextFunc::execute_impl(comterp(), idxcopy);
    ComValue pairv(comterp()->pop_stack());
    if (pairv.is_null()) {
      push_stack(ComValue::nullval());
      return;
    }
    AttributeValueList* pair = pairv.array_val();
    boolean is_pair = pairv.is_array() && pairv.lhs_assign() && pair && pair->Number()==2;
    if (!is_pair) {
      fprintf(stderr, "WARNING:  assignment to something other than a symbol or attribute (%s) ignored -- line %d\n",
	      symbol_pntr(pairv.type_symid()), funcstate()->linenum());
      push_stack(ComValue::nullval());
      return;
    }

    ComValue writeval;
    if (rhsval->is_stream()) {
      ComValue rhscopy(*rhsval);
      NextFunc::execute_impl(comterp(), rhscopy);
      ComValue tick(comterp()->pop_stack());
      if (tick.is_null()) {
	push_stack(ComValue::nullval());
	return;
      }
      writeval = tick;
    } else
      writeval = *rhsval;

    ComValue targetv(*pair->Get(0));
    if (!idxassign_in_range(targetv, pair->Get(1)->int_val())) {
      /* any nil ends a stream -- an out-of-range index is where at() itself
	 would start returning nil, so the write stream ends here too. */
      push_stack(ComValue::nullval());
      return;
    }
    ComValue written(NextFunc::write_at_pair(comterp(), pairv, writeval));
    push_stack(written);
}

/*****************************************************************************/

EachFunc::EachFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void EachFunc::execute() {
  /* lhs_assign() on this token means NextFunc's lhs-walk (strmfunc.c)
     flagged it as a **-wrapped 2nd arg wanting a batch-drain handoff,
     not a drained count here -- same stale-command-token convention
     at()/global()/local() already read via stack_top() (ARCHITECTURE.md,
     "at()'s lhs flag"); checked before stack_arg_post_eval touches
     anything, same ordering ListAtFunc's own check uses. */
  boolean for_batch = comterp()->stack_top(nkeys()+1).lhs_assign();
  ComValue strmv(stack_arg_post_eval(0));

  if (for_batch) {
    /* hand back the still-live driving stream itself, flagged, so the
       caller (NextFunc) can pace its own pulls against it instead of
       each() draining it to a count here. */
    reset_stack();
    strmv.lhs_assign(1);
    push_stack(strmv);
    return;
  }

  if (strmv.is_stream()) {
    /* explicit stream argument -- traverse normally */
    reset_stack();
    int cnt = 0;
    boolean done = false;
    while (!done) {
      NextFunc::execute_impl(comterp(), strmv);
      ComValue popval(comterp()->pop_stack());
      if (popval.is_unknown() || StrmFunc::is_delimiter(popval))
	done = true;
      else
	cnt++;
    }
    ComValue retval(cnt, ComValue::IntType);
    push_stack(retval);
    /* stamp the pushed slot, not retval: the wrapper never survives
       a copy (see AttributeValue::operator=) */
    comterp()->stack_top().wrapper(AttributeValue::BracketWrapper);

    /* this drain already reported its count above; a ring's own traffic
       counter would otherwise carry these pops into whatever prints the
       ring next. */
    AttributeValueList* ringavl = strmv.stream_list();
    if ((strmv.stream_mode_raw()&STREAM_RING) && ringavl && ringavl->Number()>5)
      ((AttributeValue*)ringavl->Get(5))->int_ref() = 0;

  } else if (nargs() > 1) {
    /* implicit stream literal -- evaluate remaining fixed-format args;
       first arg (strmv) already evaluated, count it if non-nil */
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
    comterp()->stack_top().wrapper(AttributeValue::BracketWrapper);

  } else {
    /* single non-stream arg -- error */
    fprintf(stderr, "Error: each() requires a stream argument (line %d)\n",
            funcstate()->linenum());
    reset_stack();
    push_stack(ComValue::nullval());
  }
    
}

/*****************************************************************************/


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
    flfunc->funcid(symbol_add("filternext"));
  }
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(streamv));
  avl->Append(new AttributeValue(filterv));
  ComValue stream(flfunc, avl);
  stream.stream_mode(STREAM_INTERNAL); // for internal use (use by FilterNextFunc)
  push_stack(stream);
}

/*****************************************************************************/


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
	  NextFunc::execute_impl(comterp(), strm2filt);
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


StreamLiteralNextFunc::StreamLiteralNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void StreamLiteralNextFunc::execute() {
    /* AVL: [0] FuncObj(tokbuf), [1] nremaining, [2..] (offset,count) positionals
       or (KeywordType[,offset,count]) keywords; re-navigate from AVL start after Remove() */
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

      /* construct singleton attrlist (:key val): ComValue ctor already refs it,
         so skip Resource::ref(al) -- an extra ref pins it forever (see ListFunc::execute) */
      AttributeList* al = new AttributeList();
      al->add_attr(key_symid, keyval);
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


/* is_command_node -- true if a postfix_nest_into()-built node is a call
   whose operator is the given symbol (an "assign" or "seq" node, say) --
   distinct from a leaf symbol reference, which carries no operand list. */
static boolean is_command_node(ComValue& node, int opsymid) {
  if (node.type() != ComValue::ArrayType) return false;
  AttributeValueList* avl = node.array_val();
  if (!avl || avl->Number() == 0) return false;
  ComValue op(*avl->Get(0));
  return op.is_type(ComValue::SymbolType) && op.symbol_val() == opsymid;
}

/* is_seq_node -- true for a well-formed binary seq node: op + exactly 2
   operands, the shape ";" (OPTYPE_BINARY, optable.c) always produces. */
static boolean is_seq_node(ComValue& node, int seq_symid) {
  return is_command_node(node, seq_symid) && node.array_val()->Number() == 3;
}

static void funcobj_unparse(ComValue& node, ostream& out, ComTerp* comterp, int depth);
static void funcobj_unparse_bare(ComValue& node, ostream& out, ComTerp* comterp, int depth);

static const char* INDENT_UNIT = "  ";

static std::string funcobj_indent(int depth) {
  std::string ind;
  for (int i = 0; i < depth; i++) ind += INDENT_UNIT;
  return ind;
}

/* Prints avl's entries from index 'first' on, separated by 'sep', eliding
   past comterp's cutoff() the same way ArrayType printing already does
   (comvalue.c's ArrayType case) -- "{N more}" standing in for the rest.
   'count' caps how many entries from 'first' are considered (e.g. for()'s
   first 3 args, its 4th rendered separately); -1 means the rest of avl.
   'bare', for a for()/while() body list, splices a ';'-chain element the
   same unwrapped way funcobj_unparse_bare does (each body slot accepts
   one independently, same as func()'s own last span). */
static void funcobj_unparse_list(AttributeValueList* avl, int first, const char* sep,
                                  ostream& out, ComTerp* comterp, int depth, int count = -1,
                                  boolean bare = false) {
  int total = avl->Number() - first;
  if (count >= 0 && count < total) total = count;
  int cutoff = comterp ? comterp->cutoff() : 0;
  int shown = (cutoff > 0 && total > cutoff) ? cutoff : total;
  for (int i = 0; i < shown; i++) {
    if (i > 0) out << sep;
    ComValue elt(*avl->Get(first + i));
    if (bare) funcobj_unparse_bare(elt, out, comterp, depth);
    else funcobj_unparse(elt, out, comterp, depth);
  }
  if (shown < total) out << sep << "{" << (total - shown) << " more}";
}

/* funcobj_flatten_seq -- a ';'-chain of N statements nests as N-1 strictly
   binary seq nodes, left-associative (a;b;c;d is {seq,{seq,{seq,a,b},c},d}).
   Flattens that left spine into 'out' in left-to-right statement order, so
   it renders as one flat list instead of mirroring the parse nesting. Only
   the left/first operand is descended into; a right operand that is itself
   a seq node (from an explicit, non-default right-grouping) is kept as one
   statement rather than flattened, preserving its deliberate grouping. */
static void funcobj_flatten_seq(ComValue& node, AttributeValueList* out) {
  static int seq_symid = symbol_add("seq");
  AttributeValueList* avl = node.array_val();
  ComValue left(*avl->Get(1));
  if (is_seq_node(left, seq_symid))
    funcobj_flatten_seq(left, out);
  else
    out->Append(new AttributeValue(left));
  ComValue right(*avl->Get(2));
  out->Append(new AttributeValue(right));
}

/* funcobj_unparse_stmts -- renders a flat statement list one statement per
   line, indented one level past 'depth'; 'depth' is the chain's own
   enclosing indent. Callers flatten a seq node via funcobj_flatten_seq
   first, so a multi-statement ';'-chain renders at one uniform depth
   rather than staircasing deeper per statement. */
static void funcobj_unparse_stmts(AttributeValueList* avl, int first, ostream& out,
                                   ComTerp* comterp, int depth) {
  std::string ind = funcobj_indent(depth + 1);
  int total = avl->Number() - first;
  int cutoff = comterp ? comterp->cutoff() : 0;
  int shown = (cutoff > 0 && total > cutoff) ? cutoff : total;
  for (int i = 0; i < shown; i++) {
    if (i > 0) out << ";\n" << ind;
    ComValue elt(*avl->Get(first + i));
    funcobj_unparse(elt, out, comterp, depth + 1);
  }
  if (shown < total) out << ";\n" << ind << "{" << (total - shown) << " more}";
}

/* funcobj_unparse_bare -- renders a slot that already accepts a bare
   ';'-chain unwrapped (a keyword value, or the sole body of while()/for(),
   same as func()'s own last span) -- parenthesizing it there would reparse
   as a different, larger token stream, so a seq node splices its flattened
   statements directly instead of going through funcobj_unparse's generic,
   always-parenthesizing seq branch. */
static void funcobj_unparse_bare(ComValue& node, ostream& out, ComTerp* comterp, int depth) {
  static int seq_symid = symbol_add("seq");
  if (is_seq_node(node, seq_symid)) {
    /* The caller places the first statement at depth; offset the extra
       level added by funcobj_unparse_stmts for later statements. */
    AttributeValueList* flat = new AttributeValueList();
    funcobj_flatten_seq(node, flat);
    funcobj_unparse_stmts(flat, 0, out, comterp, depth - 1);
    delete flat;
  } else
    funcobj_unparse(node, out, comterp, depth);
}

/* funcobj_unparse -- renders one postfix_nest_into() tree node as source
   for :source below. A leaf streams via ComValue's own brief operator<<;
   a call's own operator symbol is read directly to skip its narg/nkey
   token-count annotation. 'depth' is this node's own indent level, used
   only by a nested ';'-chain to indent one level deeper. */
static void funcobj_unparse(ComValue& node, ostream& out, ComTerp* comterp, int depth) {
  static int assign_symid = symbol_add("assign");
  static int seq_symid = symbol_add("seq");
  static int if_symid = symbol_add("if");
  static int for_symid = symbol_add("for");
  static int while_symid = symbol_add("while");

  if (node.type() == ComValue::ObjectType &&
      node.class_symid() == AttributeList::class_symid()) {
    /* a keyword tuple, folded by postfix_nest_into into a one-entry
       attrlist -- a bare flag's value is an empty list. */
    AttributeList* al = (AttributeList*)node.obj_val();
    ALIterator ai;
    al->First(ai);
    Attribute* attr = al->GetAttr(ai);
    ComValue val(*attr->Value());
    out << ":" << symbol_pntr(attr->SymbolId());
    if (!(val.type() == ComValue::ArrayType && val.array_val()->Number() == 0)) {
      out << " ";
      funcobj_unparse_bare(val, out, comterp, depth);
    }
    return;
  }

  if (is_command_node(node, assign_symid) && node.array_val()->Number() == 3) {
    AttributeValueList* avl = node.array_val();
    ComValue lhs(*avl->Get(1)), rhs(*avl->Get(2));
    funcobj_unparse(lhs, out, comterp, depth);
    out << "=";
    funcobj_unparse(rhs, out, comterp, depth);
    return;
  }
  if (is_seq_node(node, seq_symid)) {
    AttributeValueList* flat = new AttributeValueList();
    funcobj_flatten_seq(node, flat);
    out << "(\n" << funcobj_indent(depth + 1);
    funcobj_unparse_stmts(flat, 0, out, comterp, depth);
    out << "\n" << funcobj_indent(depth) << ")";
    delete flat;
    return;
  }
  /* if(cond :then ...[:else ...]) -- each clause's value starts its own
     line right after the keyword, at the same depth a nested value's
     own closing paren uses. */
  if (is_command_node(node, if_symid) && node.array_val()->Number() >= 3) {
    AttributeValueList* avl = node.array_val();
    int nkw = avl->Number() - 2;
    out << "if(";
    ComValue cond(*avl->Get(1));
    funcobj_unparse(cond, out, comterp, depth);
    for (int i = 0; i < nkw; i++) {
      if (i == 0) out << " ";
      else out << "\n" << funcobj_indent(depth);
      ComValue kwnode(*avl->Get(2 + i));
      if (kwnode.type() != ComValue::ObjectType ||
          kwnode.class_symid() != AttributeList::class_symid()) {
        /* a positional arg in if()'s keyword slots (e.g. the literal
           call if(1 2)) -- not valid if() usage, but still renderable. */
        funcobj_unparse(kwnode, out, comterp, depth);
        continue;
      }
      AttributeList* al = (AttributeList*)kwnode.obj_val();
      ALIterator ai;
      al->First(ai);
      Attribute* attr = al->GetAttr(ai);
      ComValue val(*attr->Value());
      out << ":" << symbol_pntr(attr->SymbolId());
      if (!(val.type() == ComValue::ArrayType && val.array_val()->Number() == 0)) {
        out << "\n" << funcobj_indent(depth + 1);
        funcobj_unparse_bare(val, out, comterp, depth + 1);
      }
    }
    out << "\n" << funcobj_indent(depth) << ")";
    return;
  }
  /* for(init test step body) -- the body (its 4th argument) starts its
     own indented line rather than trailing init/test/step. */
  if (is_command_node(node, for_symid) && node.array_val()->Number() == 5) {
    AttributeValueList* avl = node.array_val();
    out << "for(";
    funcobj_unparse_list(avl, 1, " ", out, comterp, depth, 3);
    out << "\n" << funcobj_indent(depth + 1);
    ComValue body(*avl->Get(4));
    funcobj_unparse_bare(body, out, comterp, depth + 1);
    out << ")";
    return;
  }
  /* while(test body...) -- the body (one or more space-joined bodies,
     per func()/for()'s own convention) starts its own indented line
     rather than trailing the test expression. */
  if (is_command_node(node, while_symid) && node.array_val()->Number() >= 3) {
    AttributeValueList* avl = node.array_val();
    out << "while(";
    ComValue test(*avl->Get(1));
    funcobj_unparse(test, out, comterp, depth);
    out << "\n" << funcobj_indent(depth + 1);
    funcobj_unparse_list(avl, 2, " ", out, comterp, depth + 1, -1, true);
    out << ")";
    return;
  }
  if (node.type() == ComValue::ArrayType) {
    AttributeValueList* avl = node.array_val();
    out << symbol_pntr(ComValue(*avl->Get(0)).symbol_val()) << "(";
    funcobj_unparse_list(avl, 1, " ", out, comterp, depth);
    out << ")";
    return;
  }

  node.comterp(comterp);
  out << node;
}

/* funcobj_span_trees -- each FuncObj span's own postfix_nest_into tree, in
   declaration order, built directly from the FuncObj's compiled tokens --
   no text involved. Shared by funcobj_source (stringifies them for
   info(f).source) and funcobj_tree (returns them as the :tree value
   directly). Caller deletes the returned list. */
static AttributeValueList* funcobj_span_trees(FuncObj* fo) {
  AttributeValueList* spans = new AttributeValueList();
  int offset = 0;
  for (int i = 0; i < fo->nspans(); i++) {
    int len = fo->spanlen(i);
    postfix_nest_into(fo->toks() + offset, len, spans);
    offset += len;
  }
  return spans;
}

/* funcobj_tree -- the same {func,span...} shape postfix(expr :tree) builds
   for a func() literal, assembled from the FuncObj's own span trees rather
   than reparsing printed source text. */
static ComValue funcobj_tree(FuncObj* fo) {
  AttributeValueList* spans = funcobj_span_trees(fo);
  static int func_symid = symbol_add("func");
  AttributeValueList* node = new AttributeValueList();
  ComValue headval(func_symid, AttributeValue::SymbolType);
  node->Append(new AttributeValue(headval));
  Iterator it;
  for (spans->First(it); !spans->Done(it); spans->Next(it))
    node->Append(new AttributeValue(*spans->GetAttrVal(it)));
  delete spans;
  return ComValue(node);
}

/* funcobj_source -- info(func)'s :source field: one space-separated
   positional per FuncObj span, final span's ';'-chain spliced bare
   (not parenthesized -- the paren-after-space trap, AGENTS.md). */
static std::string funcobj_source(FuncObj* fo, ComTerp* comterp) {
  boolean oldbrief = comterp ? comterp->brief() : false;
  if (comterp) comterp->brief(true);

  AttributeValueList* spans = funcobj_span_trees(fo);

  static int seq_symid = symbol_add("seq");
  int nspans = spans->Number();
  int cutoff = comterp ? comterp->cutoff() : 0;
  int shown = (cutoff > 0 && nspans > cutoff) ? cutoff : nspans;

  /* a newline is plain whitespace to the parser, so a multi-statement
     func reads better one statement per indented line; a single
     statement stays on one line. */
  AttributeValueList* lastflat = NULL;
  int nstmts = shown;
  if (shown == nspans && shown > 0) {
    ComValue last(*spans->Get(shown - 1));
    if (is_seq_node(last, seq_symid)) {
      lastflat = new AttributeValueList();
      funcobj_flatten_seq(last, lastflat);
      nstmts += lastflat->Number() - 1;
    }
  }
  boolean multiline = nstmts > 1;
  std::string ind = funcobj_indent(1);
  const char* sep = multiline ? "\n" : " ";

  std::ostringstream out;
  out << "func(" << (multiline ? "\n" + ind : "");
  for (int i = 0; i < shown; i++) {
    if (i > 0) out << sep << (multiline ? ind : "");
    if (i == shown - 1 && i == nspans - 1 && lastflat)
      funcobj_unparse_stmts(lastflat, 0, out, comterp, 0);
    else {
      ComValue span(*spans->Get(i));
      funcobj_unparse(span, out, comterp, 1);
    }
  }
  if (shown < nspans) out << sep << (multiline ? ind : "") << "{" << (nspans - shown) << " more}";
  out << (multiline ? "\n" : "") << ")";

  delete lastflat;
  delete spans;
  if (comterp) comterp->brief(oldbrief);
  return out.str();
}

InfoFunc::InfoFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void InfoFunc::execute() {
  /* attrlst=info(strm|attrlst|funcname|fileobj|pipeobj|sockobj|str [:raw] [:tree]) --
     inspect an opaque value's internal facts; :raw returns a stream's raw
     list as-is, :tree (func only) returns its postfix(:tree)-shaped tree
     directly, else a type-specific named AttributeList */

  /* fetch :raw/:tree from the post-eval region before
     reset_stack() clears them */
  static int raw_symid = symbol_add("raw");
  ComValue rawv(stack_key_post_eval(raw_symid));
  boolean rawflag = rawv.is_true();

  static int tree_symid = symbol_add("tree");
  ComValue treev(stack_key_post_eval(tree_symid));
  boolean treeflag = treev.is_true();

  /* a bare symbol naming a func is peeked via lookup_symval() (same as
     help(), helpfunc.c) rather than stack_arg_post_eval(), which would
     fire it and hand back its return value instead of the FuncObj. */
  ComValue peekval(stack_arg(0, true));
  FuncObj* peeked_fo = nil;
  if (peekval.is_type(AttributeValue::SymbolType)) {
    ComValue resolved(comterp()->lookup_symval(peekval));
    if (resolved.is_object(FuncObj::class_symid()))
      peeked_fo = (FuncObj*) resolved.obj_val();
  }

  /* info(a.f) -- peek-fire a pending "dot" call, same as help(a.f)
     (helpfunc.c): a FuncObj field is identified without being called;
     any other field value is unwrapped for the dispatch below. */
  static int dot_symid = symbol_add("dot");
  boolean dot_fired = false;
  ComValue dotval;
  if (!peeked_fo && peekval.is_type(AttributeValue::CommandType) &&
      peekval.command_symid()==dot_symid && peekval.narg()==2) {
    dotval = stack_arg_post_eval(0, true);
    dot_fired = true;
    if (dotval.class_symid()==Attribute::class_symid()) {
      Attribute* attr = (Attribute*) dotval.obj_val();
      if (attr->Value()->is_object(FuncObj::class_symid()))
	peeked_fo = (FuncObj*) attr->Value()->obj_val();
      else
	dotval = ComValue(*attr->Value());
    }
  }

  ComValue streamv(peeked_fo ? ComValue::nullval() :
		    dot_fired ? dotval : stack_arg_post_eval(0));
  reset_stack();

  /* :tree only has a meaning for a func -- nil for anything else, rather
     than falling through to that other type's own dispatch below, so a
     caller can tell a real tree apart from some other value's unrelated
     attrlist without inspecting its shape first. */
  if (treeflag && !peeked_fo) {
    push_stack(ComValue::nullval());
    return;
  }

  if (peeked_fo) {
    if (treeflag) {
      ComValue retval(funcobj_tree(peeked_fo));
      push_stack(retval);
      return;
    }
    AttributeList* al = new AttributeList();
    static int ntoks_sym = symbol_add("ntoks");
    static int nspans_sym = symbol_add("nspans");
    static int posteval_sym = symbol_add("posteval");
    static int source_sym = symbol_add("source");
    static int bare_sym = symbol_add("bare");
    ComValue ntoksv(peeked_fo->ntoks());
    ComValue nspansv(peeked_fo->nspans());
    ComValue postevalv(peeked_fo->posteval() ? ComValue::trueval() : ComValue::falseval());
    ComValue sourcev(funcobj_source(peeked_fo, comterp()).c_str());
    /* bare -- the attrlist a bare call through this instance resolves
       against, or nil if never claimed. Permanent once set. */
    ComValue barev(peeked_fo->home_attrs().is_object(AttributeList::class_symid())
		   ? peeked_fo->home_attrs() : ComValue::nullval());
    al->add_attr(ntoks_sym, ntoksv);
    al->add_attr(nspans_sym, nspansv);
    al->add_attr(posteval_sym, postevalv);
    al->add_attr(source_sym, sourcev);
    al->add_attr(bare_sym, barev);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  if (streamv.is_object(DateObj::class_symid())) {
    ComValue retval(dateobj_to_colonlist((DateObj*)streamv.obj_val()));
    push_stack(retval);
    return;
  }

  if (streamv.is_object(TimeObj::class_symid())) {
    ComValue retval(timeobj_to_colonlist((TimeObj*)streamv.obj_val()));
    push_stack(retval);
    return;
  }

  if (streamv.is_object(AttributeList::class_symid())) {
    AttributeList* target = (AttributeList*)streamv.obj_val();
    AttributeList* al = new AttributeList();
    static int sealed_sym = symbol_add("sealed");
    static int count_sym = symbol_add("count");
    static int binsz_sym = symbol_add("binsz");
    ComValue sealedv(target->sealed() ? ComValue::trueval() : ComValue::falseval());
    ComValue countv(target->Number());
    ComValue binszv(target->index_size());
    al->add_attr(sealed_sym, sealedv);
    al->add_attr(count_sym, countv);
    al->add_attr(binsz_sym, binszv);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  if (streamv.is_object(FileObj::class_symid())) {
    FileObj* fileobj = (FileObj*)streamv.obj_val();
    AttributeList* al = new AttributeList();
    static int filename_sym = symbol_add("filename");
    static int mode_sym2 = symbol_add("mode");
    static int open_sym = symbol_add("open");
    ComValue filenamev(fileobj->filename() ? fileobj->filename() : "");
    ComValue modev(fileobj->mode() ? fileobj->mode() : "");
    ComValue openv(fileobj->fptr() != nil ? ComValue::trueval() : ComValue::falseval());
    al->add_attr(filename_sym, filenamev);
    al->add_attr(mode_sym2, modev);
    al->add_attr(open_sym, openv);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  if (streamv.is_object(PipeObj::class_symid())) {
    PipeObj* pipeobj = (PipeObj*)streamv.obj_val();
    AttributeList* al = new AttributeList();
    static int command_sym = symbol_add("command");
    static int pid_sym = symbol_add("pid");
    ComValue commandv(pipeobj->command() ? pipeobj->command() : "");
    ComValue pidv((int)pipeobj->pid());
    al->add_attr(command_sym, commandv);
    al->add_attr(pid_sym, pidv);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  if (streamv.is_object(SocketObj::class_symid())) {
    SocketObj* sockobj = (SocketObj*)streamv.obj_val();
    AttributeList* al = new AttributeList();
    static int host_sym = symbol_add("host");
    static int port_sym = symbol_add("port");
    ComValue hostv(sockobj->host() ? sockobj->host() : "");
    ComValue portv((int)sockobj->port());
    al->add_attr(host_sym, hostv);
    al->add_attr(port_sym, portv);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  if (streamv.is_only_string()) {
    AttributeList* al = new AttributeList();
    static int symid_sym = symbol_add("symid");
    static int sliced_sym = symbol_add("sliced");
    static int sliceoff_sym = symbol_add("sliceoff");
    static int slicelen_sym = symbol_add("slicelen");
    static int slicecap_sym = symbol_add("slicecap");
    static int blocksz_sym = symbol_add("blocksz");
    static int blocktype_sym = symbol_add("blocktype");
    /* same id symid() reports for this string -- its symbol table slot, shared
       with other strings of equal content only when that slot is interned
       rather than a private, writable copy made by symbol_new() */
    ComValue symidv((int)streamv.string_val());
    al->add_attr(symid_sym, symidv);
    ComValue slicedv(streamv.sliced() ? ComValue::trueval() : ComValue::falseval());
    al->add_attr(sliced_sym, slicedv);
    if (streamv.sliced()) {
      ComValue sliceoffv(streamv.sliceoff());
      ComValue slicelenv(streamv.slicelen());
      al->add_attr(sliceoff_sym, sliceoffv);
      al->add_attr(slicelen_sym, slicelenv);
      if (streamv.slicecapset()) {
        ComValue slicecapv(streamv.slicecap());
        al->add_attr(slicecap_sym, slicecapv);
      }
    }
    ComValue blockszv(streamv.blocksz());
    al->add_attr(blocksz_sym, blockszv);
    /* the ValueType each @ chunk decodes/encodes as -- UnknownType (nil'd
       to a bquoted `UnknownType) for an ordinary, non-chunked string */
    ComValue blocktypev(AttributeValue::type_symid(streamv.blocktype()), ComValue::SymbolType);
    blocktypev.bquote(1);
    al->add_attr(blocktype_sym, blocktypev);
    if (streamv.blocksz()>0) {
      static int nblocks_sym = symbol_add("nblocks");
      int cap = streamv.sliced() ? streamv.slicelen() : symbol_len(streamv.string_val());
      ComValue nblocksv(cap/streamv.blocksz());
      al->add_attr(nblocks_sym, nblocksv);
    }
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  if (streamv.is_type(ComValue::ArrayType)) {
    AttributeValueList* avl = streamv.array_val();
    AttributeList* al = new AttributeList();
    static int count_sym3 = symbol_add("count");
    static int coloned_sym = symbol_add("coloned");
    static int nested_sym = symbol_add("nested");
    static int cutoff_sym = symbol_add("cutoff");
    ComValue countv(avl ? avl->Number() : 0);
    ComValue colonedv(streamv.coloned() ? ComValue::trueval() : ComValue::falseval());
    ComValue nestedv(avl && avl->nested_insert() ? ComValue::trueval() : ComValue::falseval());
    ComValue cutoffv(avl ? avl->max_out() : -1);
    al->add_attr(count_sym3, countv);
    al->add_attr(coloned_sym, colonedv);
    al->add_attr(nested_sym, nestedv);
    al->add_attr(cutoff_sym, cutoffv);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

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

  /* STREAM_FUNCOBJ packs a bare FuncObj* in the func slot (comterp.c's
     try_stream_funcobj), not a ComFunc* -- funcid() on it is undefined
     behavior, so report it on its own rather than falling into the
     ComFunc-shaped paths below. */
  if (streamv.stream_mode()&STREAM_FUNCOBJ) {
    FuncObj* fo = (FuncObj*) streamv.stream_func();
    AttributeList* al = new AttributeList();
    static int mode_sym_fo = symbol_add("mode");
    static int ntoks_sym_fo = symbol_add("ntoks");
    static int nargs_sym_fo = symbol_add("nargs");
    ComValue modeval("funcobj");
    ComValue ntoksv(fo ? fo->ntoks() : 0);
    ComValue nargsv(avl ? avl->Number() : 0);
    al->add_attr(mode_sym_fo, modeval);
    al->add_attr(ntoks_sym_fo, ntoksv);
    al->add_attr(nargs_sym_fo, nargsv);
    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

  /* identify a literal-backed stream; others have different list layouts */
  static int slnf_symid = -1;
  if (slnf_symid == -1) slnf_symid = symbol_add("streamliteralnext");
  ComFunc* sfunc = streamv.stream_func() ? (ComFunc*)streamv.stream_func() : nil;
  boolean is_literal = sfunc && sfunc->funcid() == slnf_symid;

  /* ring FIFO: report head/tail/count/cap plus the live region as a slice
     (or two, when it straddles the end) of the backing string -- the
     string's own slice fields are how a run of its bytes is represented
     everywhere else, so the ring's live data is shown the same way rather
     than as raw offsets */
  if (avl && sfunc == ring_next_func(comterp()) && avl->Number()>=5) {
    AttributeList* al = new AttributeList();
    static int mode_sym3 = symbol_add("mode");
    static int base_sym = symbol_add("base");
    static int head_sym = symbol_add("head");
    static int tail_sym = symbol_add("tail");
    static int count_sym = symbol_add("count");
    static int cap_sym = symbol_add("cap");
    static int wrap_sym = symbol_add("wrap");
    static int free_sym = symbol_add("free");
    static int traffic_sym = symbol_add("traffic");
    static int buf_sym = symbol_add("buf");
    ComValue bufv(*((AttributeValue*)avl->Get(0)));
    int head = ((AttributeValue*)avl->Get(1))->int_val();
    int tail = ((AttributeValue*)avl->Get(2))->int_val();
    int count = ((AttributeValue*)avl->Get(3))->int_val();
    int wrap = ((AttributeValue*)avl->Get(4))->int_val();
    /* the ring's buffer may itself be a slice (feed("abcd"@1:3)) -- head/tail
       are offsets within that window, so a display slice needs winoff added
       to land on the same bytes the ring itself reads and writes */
    int winoff = bufv.sliced() ? bufv.sliceoff() : 0;
    int bytecap = bufv.sliced() ? bufv.slicelen() : symbol_len(bufv.string_val());
    /* head/tail/count are slot counts, elemsz bytes each (1 for a plain
       char-granular string) -- cap and the buf slice below convert
       through elemsz to land on the bytes feed()/next() actually use. */
    int elemsz = bufv.blocksz()>0 ? bufv.blocksz() : 1;
    int cap = bytecap/elemsz;

    ComValue modeval("ring");
    /* the live buffer's address, as a hex-printed ULongType -- UIntType
       would truncate a 64-bit pointer */
    ComValue basev((unsigned long)(bufv.string_ptr()+winoff));
    basev.state(AttributeValue::HexState);
    ComValue headv(head);
    ComValue tailv(tail);
    ComValue countv2(count);
    ComValue capv(cap);
    ComValue wrapv(wrap ? ComValue::trueval() : ComValue::falseval());
    /* same "how many more pushes fit" reckoning the push path itself
       uses, exposed directly so a caller doesn't have to reconstruct it
       as cap-count (which is only right in :wrap mode -- see ring_avail()) */
    ComValue freev(ring_avail(avl));
    /* traffic (elements pushed or popped since this ring was last
       printed) is a 6th avl slot older rings may lack -- report 0
       rather than reading past the end. */
    ComValue trafficv(avl->Number()>5 ? ((AttributeValue*)avl->Get(5))->int_val() : 0);
    al->add_attr(mode_sym3, modeval);
    al->add_attr(base_sym, basev);
    al->add_attr(head_sym, headv);
    al->add_attr(tail_sym, tailv);
    al->add_attr(count_sym, countv2);
    al->add_attr(cap_sym, capv);
    al->add_attr(wrap_sym, wrapv);
    al->add_attr(free_sym, freev);
    al->add_attr(traffic_sym, trafficv);

    if (count>0) {
      /* one contiguous run when it doesn't straddle the end, two when it
	 does -- same slice ctor pattern ListAtFunc uses for str@lo:hi */
      int firstlen = head+count<=cap ? count : cap-head;
      ComValue first(bufv.string_val(), ComValue::StringType);
      first.ref_as_needed();
      first.sliceoff(winoff+head*elemsz);
      first.slicelen(firstlen*elemsz);
      first.sliced(1);
      first.blocktype(bufv.blocktype());
      if (firstlen==count) {
	al->add_attr(buf_sym, first);
      } else {
	ComValue second(bufv.string_val(), ComValue::StringType);
	second.ref_as_needed();
	second.sliceoff(winoff);
	second.slicelen((count-firstlen)*elemsz);
	second.sliced(1);
	second.blocktype(bufv.blocktype());
	AttributeValueList* parts = new AttributeValueList();
	parts->Append(new AttributeValue(first));
	parts->Append(new AttributeValue(second));
	ComValue partsv(parts);
	al->add_attr(buf_sym, partsv);
      }
    }

    ComValue retval(AttributeList::class_symid(), (void*)al);
    push_stack(retval);
    return;
  }

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
      /* keyword-with-value needs two trailing slots; stop early
         to avoid an orphaned key entry */
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

/*****************************************************************************/


/* singleton RingNextFunc, shared by FeedFunc (construction/push) and
   InfoFunc (identifying a ring stream to report on) */
static RingNextFunc* ring_next_func(ComTerp* comterp) {
  static RingNextFunc* rnfunc = nil;
  if (!rnfunc) {
    rnfunc = new RingNextFunc(comterp);
    rnfunc->funcid(symbol_add("ringnext"));
  }
  return rnfunc;
}

/* the ring's buffer element may itself be a slice (feed("abcd"@1:3) confines
   the ring to that window) -- wrap it as a ComValue to reach sliced()/
   sliceoff()/slicelen(), not exposed on the raw AttributeValue* element. */
static int ring_buf_bytecap(AttributeValue* bufav) {
  ComValue bufv(*bufav);
  return bufv.sliced() ? bufv.slicelen() : symbol_len(bufv.string_val());
}

static char* ring_buf_base(AttributeValue* bufav) {
  ComValue bufv(*bufav);
  return (char*)bufv.string_ptr() + (bufv.sliced() ? bufv.sliceoff() : 0);
}

/* width in bytes of one ring slot -- blocksz() for a typed (string(n
   type)) buffer, one byte for an ordinary char-granular string. */
static int ring_elemsz(AttributeValue* bufav) {
  ComValue bufv(*bufav);
  int bsz = bufv.blocksz();
  return bsz>0 ? bsz : 1;
}

static AttributeValue::ValueType ring_buf_blocktype(AttributeValue* bufav) {
  ComValue bufv(*bufav);
  return bufv.blocktype();
}

/* ring capacity in slots, not bytes -- one slot per element of the
   buffer's own declared type, matching how at()/size() already count it. */
static int ring_buf_cap(AttributeValue* bufav) {
  return ring_buf_bytecap(bufav) / ring_elemsz(bufav);
}

/* how many more ring_push_char() calls would succeed right now -- wrap
   mode is bounded by count (tail always wraps below cap), :noring by
   how far tail has advanced (it never wraps back, so count can trail
   it once elements are popped). */
static int ring_avail(AttributeValueList* avl) {
  if (!avl || avl->Number()<5) return 0;
  AttributeValue* bufav = (AttributeValue*)avl->Get(0);
  AttributeValue* tailav = (AttributeValue*)avl->Get(2);
  AttributeValue* countav = (AttributeValue*)avl->Get(3);
  AttributeValue* wrapav = (AttributeValue*)avl->Get(4);
  int cap = ring_buf_cap(bufav);
  int tail = tailav->int_val();
  if (cap<=0 || tail>=cap) return 0;
  if (!wrapav->int_val()) return cap-tail;
  int count = countav->int_val();
  return count<cap ? cap-count : 0;
}

/* build a fresh ring FIFO over buf's own bytes (or its sliced window).  avl
   layout: [0]=buf [1]=head [2]=tail [3]=count [4]=wrap(0|1) [5]=traffic --
   wrap=0 (:noring) never reclaims space freed from the head, wrap=1 is the
   circular default.  traffic counts elements pushed or popped since this
   ring was last printed (ComValue::StreamType's print case, comvalue.c).

   buf's own content up to its first NUL (bounded by its capacity) seeds
   the ring as already-queued data, immediately poppable -- string(cap) is
   all-NUL so this is 0 for a freshly allocated buffer, but feed("hello")
   starts with all five characters queued, matching what feeding a string
   into a FIFO meant before it had a fixed-capacity form.  A typed buffer
   (string(n type)) has no such text terminator -- its zero value is
   ordinary data, not an end marker -- so it always starts empty rather
   than guessing how much of it counts as already queued. */
static ComValue ring_stream_value(ComTerp* comterp, ComValue& buf, boolean wrap) {
  int bytecap = buf.sliced() ? buf.slicelen() : symbol_len(buf.string_val());
  const char* base = buf.string_ptr() + (buf.sliced() ? buf.sliceoff() : 0);
  int elemsz = buf.blocksz()>0 ? buf.blocksz() : 1;
  int cap = bytecap/elemsz;
  int initial;
  if (elemsz==1) {
    const void* nulp = memchr(base, '\0', bytecap);
    initial = nulp ? (const char*)nulp-base : cap;
    if (initial>cap) initial = cap;
  } else {
    initial = 0;
  }
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(buf));
  avl->Append(new AttributeValue(0, AttributeValue::IntType));  // head
  int tail0 = initial;
  if (tail0>=cap) tail0 = wrap ? 0 : cap;
  avl->Append(new AttributeValue(tail0, AttributeValue::IntType));  // tail
  avl->Append(new AttributeValue(initial, AttributeValue::IntType));  // count
  avl->Append(new AttributeValue(wrap ? 1 : 0, AttributeValue::IntType));  // wrap
  avl->Append(new AttributeValue(0, AttributeValue::IntType));  // [5] traffic since last print
  ComValue stream(ring_next_func(comterp), avl);
  stream.stream_mode(STREAM_INTERNAL | STREAM_RING);
  return stream;
}

/* push one element into a ring FIFO's avl; false (refused) when full --
   capacity is ring_buf_cap(), not strlen(), so an embedded zero byte
   written earlier in the ring never strands the rest of the buffer.
   A typed (blocktype()!=UnknownType) ring encodes v at that type's own
   width via comval_encode() -- the same promotion/demotion at(s N :set
   v) already does -- instead of coercing through char_val(); an
   ordinary byte ring stores v.char_val() directly. */
static boolean ring_push_elt(AttributeValueList* avl, ComValue& v) {
  if (!avl || avl->Number()<5) return false;
  AttributeValue* bufav = (AttributeValue*)avl->Get(0);
  AttributeValue* tailav = (AttributeValue*)avl->Get(2);
  AttributeValue* countav = (AttributeValue*)avl->Get(3);
  AttributeValue* wrapav = (AttributeValue*)avl->Get(4);
  int cap = ring_buf_cap(bufav);
  int count = countav->int_val();
  int tail = tailav->int_val();
  /* full is count==cap in wrap mode, but a :noring tail sits AT cap once it
     stops advancing (never reclaiming drained space), so count alone can
     understate fullness there -- tail==cap is the real bound on where an
     element may land, checked either way since it's always true when count
     alone would already refuse */
  if (cap<=0 || count>=cap || tail>=cap) return false;
  AttributeValue::ValueType bt = ring_buf_blocktype(bufav);
  char* dst = ring_buf_base(bufav) + tail*ring_elemsz(bufav);
  if (bt == AttributeValue::UnknownType) *dst = v.char_val();
  else ComValue::comval_encode(dst, v, bt);
  int newtail = tail+1;
  if (newtail>=cap) newtail = wrapav->int_val() ? 0 : cap;
  tailav->int_ref() = newtail;
  countav->int_ref() = count+1;
  if (avl->Number()>5) ((AttributeValue*)avl->Get(5))->int_ref()++;
  return true;
}

static boolean ring_push_char(AttributeValueList* avl, char ch) {
  ComValue cv(ch);
  return ring_push_elt(avl, cv);
}

/* push a value into a ring FIFO's avl.

   A typed (blocktype()!=UnknownType) ring's slot is one whole element of
   that type, so a pushed value is promoted/demoted to it via
   ring_push_elt() as a single unit -- never split into characters, since
   there's no meaningful character view of a UIntType (or other typed)
   element.

   An ordinary byte ring's slot is one literal byte.  An unprotected
   StringType (not bquoted, not :raw) pushes its characters in order --
   the only way a multi-character string can go in at all, whatever
   rawflag says.  Indexes the string's own bytes directly (not cstr(),
   which truncates at the first embedded NUL) so a slice containing one
   still pushes its full slicelen() bytes.  Snapshots those bytes before
   writing any of them, since the source can be the ring's own backing
   string (e.g. feeding a ring a slice of itself) -- writing in place
   while still reading would let an earlier write clobber a byte a later
   iteration hasn't read yet.

   Any other StringType (bquoted, or :raw-protected) can only be honored
   when it's exactly one byte long -- a ring has nowhere to put a whole
   multi-character string as a single unsplit unit, so it's refused
   rather than silently truncated through char_val().  A non-string value
   pushes via char_val() as before.

   Snapshots only as many bytes as ring_avail() says have room, not the
   whole string, so pushing a long string at a full or nearly-full ring
   copies at most what could actually land.

   Stops at the first refusal (buffer full or an un-splittable string),
   leaving whatever already landed in place, and reports that refusal to
   the caller. */
static boolean ring_push_value(AttributeValueList* avl, ComValue& v, boolean rawflag) {
  if (!avl || avl->Number()<5) return false;
  if (ring_buf_blocktype((AttributeValue*)avl->Get(0)) != AttributeValue::UnknownType)
    return ring_push_elt(avl, v);
  if (!rawflag && streams_as_characters(v)) {
    const char* base = v.string_ptr() + (v.sliced() ? v.sliceoff() : 0);
    int len = v.sliced() ? v.slicelen() : symbol_len(v.string_val());
    if (len==0) return true;
    int avail = ring_avail(avl);
    if (avail<=0) return false;
    int tocopy = len<avail ? len : avail;
    std::string snapshot(base, tocopy);
    for (int k=0; k<len; k++)
      if (!ring_push_char(avl, k<tocopy ? snapshot[k] : 0)) return false;
    return true;
  }
  if (v.is_type(ComValue::StringType)) {
    const char* base = v.string_ptr() + (v.sliced() ? v.sliceoff() : 0);
    int len = v.sliced() ? v.slicelen() : symbol_len(v.string_val());
    if (len!=1) return false;
    return ring_push_char(avl, base[0]);
  }
  return ring_push_char(avl, v.char_val());
}

/* pop one element from a ring FIFO's avl; ComValue::nullval() when empty.
   An ordinary byte ring returns a CharType element; a typed ring decodes
   the slot back via comval_decode(), the reverse of ring_push_elt()'s
   comval_encode(). */
static ComValue ring_pop_char(AttributeValueList* avl) {
  if (!avl || avl->Number()<5) return ComValue::nullval();
  AttributeValue* bufav = (AttributeValue*)avl->Get(0);
  AttributeValue* headav = (AttributeValue*)avl->Get(1);
  AttributeValue* countav = (AttributeValue*)avl->Get(3);
  int count = countav->int_val();
  if (count<=0) return ComValue::nullval();
  int cap = ring_buf_cap(bufav);
  int head = headav->int_val();
  AttributeValue::ValueType bt = ring_buf_blocktype(bufav);
  char* src = ring_buf_base(bufav) + head*ring_elemsz(bufav);
  ComValue result = bt == AttributeValue::UnknownType
    ? ComValue(*src) : ComValue::comval_decode(src, bt);
  headav->int_ref() = cap>0 ? (head+1)%cap : 0;
  countav->int_ref() = count-1;
  if (avl->Number()>5) ((AttributeValue*)avl->Get(5))->int_ref()++;
  return result;
}

/* read the element at a ring's head without popping it -- same decode as
   ring_pop_char(), minus the head/count/traffic updates -- so a ring's
   more() is a true, zero-cost peek rather than a pull-and-restash. */
static ComValue ring_peek_char(AttributeValueList* avl) {
  if (!avl || avl->Number()<5) return ComValue::nullval();
  AttributeValue* bufav = (AttributeValue*)avl->Get(0);
  AttributeValue* headav = (AttributeValue*)avl->Get(1);
  AttributeValue* countav = (AttributeValue*)avl->Get(3);
  if (countav->int_val()<=0) return ComValue::nullval();
  int head = headav->int_val();
  AttributeValue::ValueType bt = ring_buf_blocktype(bufav);
  char* src = ring_buf_base(bufav) + head*ring_elemsz(bufav);
  return bt == AttributeValue::UnknownType
    ? ComValue(*src) : ComValue::comval_decode(src, bt);
}

/* push one already-pulled value onto a ring, refusing by type rather than
   pushing something the ring can't represent: AnyType boxes a value whole
   (comval_encode's AnyType branch), so a string there costs exactly one
   slot like any other value -- only a numeric blocktype's lossy conversion
   needs refusing a string outright; an untyped (byte) ring refuses a
   multi-byte string that wouldn't fit the room actually left. */
static boolean ring_push_one(AttributeValueList* avl, ComValue& popval, boolean rawflag) {
  AttributeValue::ValueType bt = ring_buf_blocktype((AttributeValue*)avl->Get(0));
  boolean numeric_ring = bt!=AttributeValue::UnknownType && bt!=AttributeValue::AnyType;
  if (numeric_ring && popval.is_type(ComValue::StringType)) return false;
  if (bt==AttributeValue::UnknownType && streams_as_characters(popval)) {
    int len = popval.sliced() ? popval.slicelen() : symbol_len(popval.string_val());
    if (len>ring_avail(avl)) return false;
  }
  return ring_push_value(avl, popval, rawflag);
}

/* push one feed() argument onto a ring: a stream is run, not stored --
   pulled one value at a time and each pushed in turn, stopping (without
   consuming the value that wouldn't fit) once the ring has no room left.
   A value this can't pull without risking loss -- because capacity is
   already exhausted -- is never pulled, so a stream with more left after
   exactly filling the ring is refused the same way a short one is, rather
   than guessed at by pulling anyway.  The ring is refused outright as its
   own source, directly or wrapped (e.g. nested inside a FIFO fed back into
   it): registering the destination in NextFunc's own recursive-stream guard
   makes a pull that bottoms out on it return nil instead of completing the
   cycle, the same way next() already refuses a stream draining itself.
   A non-stream argument still goes straight to ring_push_value().  When
   count is given, it is incremented once per element actually pushed, for
   an each()-forced eager drain to report how many landed. */
static boolean ring_push_arg(ComTerp* comterp, AttributeValueList* avl, ComValue& v,
			      boolean rawflag, int* count = nil) {
  if (rawflag || !v.is_stream()) {
    boolean ok = ring_push_value(avl, v, rawflag);
    if (ok && count) (*count)++;
    return ok;
  }
  if (v.stream_list()==avl) return false;
  if (_draining_guard_tripped_avl==avl) _draining_guard_tripped_avl = 0;
  DrainingAVLGuard dest_guard(avl);
  ComValue streamv(v);
  for (;;) {
    if (ring_avail(avl)<=0) {
      /* out of room -- "genuinely refused" (source still has a value,
	 lost otherwise) and "nothing left to refuse" (source is also
	 spent) both reach this check looking identical from avail alone,
	 so peek the source via more() rather than assume the worst; a
	 real peeked value is already restashed onto streamv by more()
	 itself, so a later feed() call on the same source resumes
	 without loss. */
      ComValue peeked(MoreFunc::execute_impl(comterp, streamv));
      if (_draining_guard_tripped_avl==avl) {
	_draining_guard_tripped_avl = 0;
	return false;
      }
      return peeked.is_null();
    }
    NextFunc::execute_impl(comterp, streamv);
    ComValue popval(comterp->pop_stack());
    /* a trip naming this ring is our own refusal; a trip naming some other
       ring isn't ours to act on, so popval is still a legitimate pull. */
    if (_draining_guard_tripped_avl==avl) {
      _draining_guard_tripped_avl = 0;
      return false;
    }
    if (popval.is_unknown() || StrmFunc::is_delimiter(popval)) return true;
    if (!ring_push_one(avl, popval, rawflag)) return false;
    if (count) (*count)++;
  }
}

FeedFunc::FeedFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void FeedFunc::execute() {
  static FeedNextFunc* fnfunc = nil;
  if (!fnfunc) {
    fnfunc = new FeedNextFunc(comterp());
    fnfunc->funcid(symbol_add("feednext"));
  }

  int n = nargs();
  ComValue* argv = n>0 ? new ComValue[n] : nil;

  /* each()'s batch-drain flag only means anything to a ring target -- a
     FIFO has its own lazy default (STREAM_NESTED below) and ignores it. */
  boolean arg1_from_each = false;
  if (n>0) {
    /* symbol=true -- suppress stack_arg_post_eval's default symbol lookup
       so a bquoted symbol (e.g. `EOS) survives into storage intact */
    argv[0] = stack_arg_post_eval(0, true);
    boolean arg0_ring_target = (argv[0].is_stream() &&
				 argv[0].stream_func()==(void*)ring_next_func(comterp())) ||
      streams_as_characters(argv[0]);
    if (nargsfixed()==2 && arg0_ring_target) {
      /* flags an each()-wrapped 2nd arg before it evaluates, the same
	 stale-command-token convention as NextFunc's own lhs-walk above
	 (see doc/POSTFIX-INDEXING.md for the walk itself). */
      /* a bare symbol can't be an each() call, so it skips the walk below. */
      ComValue peek1(stack_arg(1, true));
      if (peek1.type() != ComValue::SymbolType) {
	static int each_symid = symbol_add("each");
	ComValue argoff(comterp()->stack_top());
	int offtop = argoff.int_val() - comterp()->pfnum();
	int argcnt = 0;
	/* skip any trailing keywords (:raw, :noring) before walking args,
	   same order stack_arg_post_eval() itself uses -- a keyword token
	   found where an arg is expected is what "unexpected keyword" means. */
	for (int k=0; k<nkeys(); k++) { argcnt = 0; skip_key_in_expr(offtop, argcnt); }
	skip_arg_in_expr(offtop, argcnt);
	int startidx = comterp()->pfnum() + offtop + argcnt - 1;
	ComValue& startval = comterp()->pfcomvals()[startidx];
	if (startval.is_type(ComValue::CommandType)) {
	  ComFunc* func = (ComFunc*)startval.obj_val();
	  if (func->funcid() == each_symid) {
	    startval.lhs_assign(1);
	    arg1_from_each = true;
	  }
	}
      }
    }
  }
  for (int i=1; i<n; i++) argv[i] = stack_arg_post_eval(i, true);
  /* :raw -- store a stream arg as an opaque, undrained element (skip STREAM_NESTED
     tagging), so a FIFO can hold streams and rotate them, not flatten them going in */
  static int raw_symid = symbol_add("raw");
  ComValue rawv(stack_key_post_eval(raw_symid));
  boolean rawflag = rawv.is_true();
  /* :noring -- a new string-backed FIFO refuses a push once full instead
     of wrapping to reclaim drained space; no effect once the FIFO exists */
  static int noring_symid = symbol_add("noring");
  ComValue noringv(stack_key_post_eval(noring_symid));
  boolean noringflag = noringv.is_true();
  reset_stack();

  boolean arg0_is_fifo = n>0 && argv[0].is_stream() &&
    argv[0].stream_func() == (void*)fnfunc;
  boolean arg0_is_ring = n>0 && argv[0].is_stream() &&
    argv[0].stream_func() == (void*)ring_next_func(comterp());

  /* a single bare stream argument into a ring defers the push into a lazy
     wrapper, the same "do it later" default a growable FIFO's own
     STREAM_NESTED tagging already gives it below. */
  /* each() (arg1_from_each) asks for "do it now" instead, which the
     fall-through eager-drain code a few lines down still provides. */
  if ((arg0_is_ring || (n>0 && !arg0_is_fifo && !rawflag && streams_as_characters(argv[0]))) &&
      nargsfixed()==2 && !rawflag && argv[1].is_stream() && !arg1_from_each) {
    ComValue ringv(arg0_is_ring ? argv[0] : ring_stream_value(comterp(), argv[0], !noringflag));
    AttributeValueList* avl = ringv.stream_list();
    if (argv[1].stream_list()==avl) {
      /* the ring refused as its own source, same as the eager path below */
      delete [] argv;
      push_stack(ComValue::nullval());
      return;
    }
    static FeedRingNextFunc* frnfunc = nil;
    if (!frnfunc) {
      frnfunc = new FeedRingNextFunc(comterp());
      frnfunc->funcid(symbol_add("feedringnext"));
    }
    AttributeValueList* wavl = new AttributeValueList();
    wavl->Append(new AttributeValue(ringv));
    wavl->Append(new AttributeValue(argv[1]));
    ComValue wrapper(frnfunc, wavl);
    /* STREAM_RING tells NextFunc::execute_impl a nil here means refused
       for now, not exhausted, so it keeps this wrapper's [ring,source]
       state intact instead of clearing it. */
    wrapper.stream_mode(STREAM_INTERNAL | STREAM_RING);
    delete [] argv;
    push_stack(wrapper);
    return;
  }

  if (arg0_is_ring) {
    /* push the remaining args onto the ring's tail; a stream argument is
       run (pulled and pushed one value at a time), and any refusal fails
       the whole call with nil, same as next()'s empty-ring refusal. */
    /* an each()-forced drain (arg1_from_each) reports its pushed count,
       bracketed like next()'s own batch-drain; every other case still
       just returns the ring. */
    AttributeValueList* avl = argv[0].stream_list();
    boolean ok = true;
    int pushcount = 0;
    for (int i=1; ok && i<n; i++) ok = ring_push_arg(comterp(), avl, argv[i], rawflag, &pushcount);
    if (ok && arg1_from_each) {
      ComValue retval(pushcount, ComValue::IntType);
      delete [] argv;
      push_stack(retval);
      comterp()->stack_top().wrapper(AttributeValue::BracketWrapper);
      return;
    }
    ComValue retval(ok ? argv[0] : ComValue::nullval());
    delete [] argv;
    push_stack(retval);
    return;
  }

  if (n>0 && !arg0_is_fifo && !rawflag && streams_as_characters(argv[0])) {
    /* a bare (unprotected) string first argument becomes a fixed-capacity
       ring over its own bytes, not a growable copy of its characters --
       a bquoted or :raw-protected string still falls through to the
       growable FIFO below, stored whole */
    ComValue stream(ring_stream_value(comterp(), argv[0], !noringflag));
    AttributeValueList* avl = stream.stream_list();
    boolean ok = true;
    int pushcount = 0;
    for (int i=1; ok && i<n; i++) ok = ring_push_arg(comterp(), avl, argv[i], rawflag, &pushcount);
    if (ok && arg1_from_each) {
      ComValue retval(pushcount, ComValue::IntType);
      delete [] argv;
      push_stack(retval);
      comterp()->stack_top().wrapper(AttributeValue::BracketWrapper);
      return;
    }
    ComValue retval(ok ? stream : ComValue::nullval());
    delete [] argv;
    push_stack(retval);
    return;
  }

  if (arg0_is_fifo) {
    /* append the remaining args to the existing FIFO's back end; a stream-valued
       arg is tagged STREAM_NESTED so NextFunc::execute_impl drains it lazily, one value per next() */
    AttributeValueList* avl = argv[0].stream_list();
    for (int i=1; i<n; i++) {
      /* a string is ingested as its characters via the same cursor stream()
         would make, then the lazy nested-stream path drains it one value per next() */
      if (!rawflag && streams_as_characters(argv[i]))
	argv[i] = string_stream_value(comterp(), argv[i]);
      boolean tag_nested = argv[i].is_stream() && !rawflag;
      AttributeValue* elt;
      if (argv[i].is_stream() && argv[i].stream_list() == avl) {
        /* feed(f f): the fed-in stream's own backing list *is* this FIFO's list,
           so storing it directly self-references; snapshot contents instead, as $$/stream() does */
        AttributeValueList* snapshot = new AttributeValueList(avl);
        elt = new AttributeValue(argv[i].stream_func(), snapshot);
        elt->stream_mode(rawflag ? argv[i].stream_mode_raw()
                                 : (argv[i].stream_mode_raw()|STREAM_NESTED));
      } else {
        int mode = tag_nested ? (argv[i].stream_mode_raw()|STREAM_NESTED) : 0;
        elt = new AttributeValue(argv[i]);
        if (tag_nested) elt->stream_mode(mode);
      }
      avl->Append(elt);
    }
    ComValue retval(argv[0]);
    delete [] argv;
    push_stack(retval);
    return;
  }

  /* build a brand-new FIFO from all given args (zero args -> empty
     FIFO), same STREAM_NESTED tagging as the append case above */
  AttributeValueList* avl = new AttributeValueList();
  for (int i=0; i<n; i++) {
    if (!rawflag && streams_as_characters(argv[i]))
      argv[i] = string_stream_value(comterp(), argv[i]);
    boolean tag_nested = argv[i].is_stream() && !rawflag;
    int mode = tag_nested ? (argv[i].stream_mode_raw()|STREAM_NESTED) : 0;
    AttributeValue* elt = new AttributeValue(argv[i]);
    if (tag_nested) elt->stream_mode(mode);
    avl->Append(elt);
  }
  delete [] argv;
  ComValue stream(fnfunc, avl);
  stream.stream_mode(STREAM_INTERNAL);
  push_stack(stream);
}

/*****************************************************************************/

/* the reverse of type_symid(ValueType): a linear scan over the closed enum,
   mirrors symbolfunc.c's own private copy for string(cap typesym). */
static AttributeValue::ValueType ring_valuetype_for_symid(int symid) {
  for (int t=AttributeValue::UnknownType; t<=AttributeValue::AnyType; t++)
    if (AttributeValue::type_symid((AttributeValue::ValueType)t) == symid)
      return (AttributeValue::ValueType)t;
  return AttributeValue::UnknownType;
}

RingFunc::RingFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void RingFunc::execute() {
  ComValue capv(stack_arg(0));
  /* symbol=true: a bare type name (AnyType, not `AnyType) reads as the
     symbol itself, same as string(cap typesym)'s own typesym argument */
  ComValue typev(stack_arg(1, true));
  boolean typeflag = typev.type()==ComValue::SymbolType;
  reset_stack();

  AttributeValue::ValueType blocktype = AttributeValue::UnknownType;
  int chunksz = 0;
  if (typeflag) {
    blocktype = ring_valuetype_for_symid(typev.symbol_val());
    chunksz = AttributeValue::type_size(blocktype);
  }
  int cap = typeflag ? capv.int_val()*chunksz : capv.int_val();
  int newid = cap>=0 ? symbol_new((unsigned)cap, false) : -1;
  if (newid<0) {
    push_stack(ComValue::nullval());
    return;
  }
  ComValue bufv((unsigned int)newid, ComValue::StringType);
  if (typeflag) bufv.blocktype(blocktype);

  ComValue stream(ring_stream_value(comterp(), bufv, true));
  push_stack(stream);
}

/*****************************************************************************/

ChunkFunc::ChunkFunc(ComTerp* comterp) : ComFunc(comterp) {
}

/* chunk(strm n) -- re-grain a stream.  The cost a script pays per element is
   not producing it (a computed range and a fully materialized list hand one
   over at the same speed) but crossing into the interpreter to ask: the *
   dispatch, the ComValue, the assignment, the loop test.  A buffer behind a
   per-element * cannot touch any of that.  Handing back n elements at a time
   can: the script loop then runs total/n times, and the per-element work
   inside a block happens wherever the block is consumed.

   The block is an ordinary indexed list, deliberately -- at()/@/size()/xpose()
   already work on one, so chunk needs no companion commands to be useful, and
   the two access operators stay honest about the two grains: * for the next
   block, @ within it. */
void ChunkFunc::execute() {
  static ChunkNextFunc* cnfunc = nil;
  if (!cnfunc) {
    cnfunc = new ChunkNextFunc(comterp());
    cnfunc->funcid(symbol_add("chunknext"));
  }

  ComValue srcv(stack_arg_post_eval(0));
  ComValue nv(stack_arg_post_eval(1));
  reset_stack();

  if (!srcv.is_stream()) {
    push_stack(ComValue::nullval());
    return;
  }
  int n = nv.is_known() ? nv.int_val() : 1;
  if (n < 1) n = 1;

  /* the state this stream carries (source, block size) lives in the
     stream's own backing list, same slot every internal-mode stream uses */
  AttributeValueList* avl = new AttributeValueList();
  avl->Append(new AttributeValue(srcv));
  avl->Append(new AttributeValue(n, AttributeValue::IntType));

  ComValue stream(cnfunc, avl);
  stream.stream_mode(STREAM_INTERNAL);
  push_stack(stream);
}

/*****************************************************************************/


ChunkNextFunc::ChunkNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void ChunkNextFunc::execute() {
  ComValue streamv(stack_arg(0));
  reset_stack();

  AttributeValueList* state = streamv.stream_list();
  if (!state) {
    push_stack(ComValue::nullval());
    return;
  }
  Iterator i;
  state->First(i);
  if (state->Done(i)) {
    push_stack(ComValue::nullval());
    return;
  }
  /* the source ComValue lives in the state list, so advancing through
     this copy advances it too -- successive next()s resume where the last block stopped */
  ComValue srcv(*state->GetAttrVal(i));
  state->Next(i);
  int n = state->Done(i) ? 1 : state->GetAttrVal(i)->int_val();

  AttributeValueList* block = new AttributeValueList();
  for (int k=0; k<n; k++) {
    int before = comterp()->stack_height();
    NextFunc::execute_impl(comterp(), srcv);
    if (comterp()->stack_height() <= before) break;
    ComValue elt(comterp()->pop_stack());
    if (elt.is_null() || elt.is_unknown() || elt.is_nil()) break;
    block->Append(new AttributeValue(elt));
  }

  if (block->Number()==0) {
    /* no elements this pull -- report exhaustion, same as every stream
       consumer's nil handling; ambiguous for a growable feed() FIFO, but chunk can't tell better */
    delete block;
    push_stack(ComValue::nullval());
    return;
  }
  ComValue retval(block);
  push_stack(retval);
}

/*****************************************************************************/


FeedNextFunc::FeedNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void FeedNextFunc::execute() {
  ComValue operand1(stack_arg(0));
  reset_stack();

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
      push_stack(ComValue::nullval());
    }
  } else
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

RingNextFunc::RingNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void RingNextFunc::execute() {
  ComValue operand1(stack_arg(0));
  reset_stack();

  ComValue retval(ring_pop_char(operand1.stream_list()));
  push_stack(retval);
}

/*****************************************************************************/

FeedRingNextFunc::FeedRingNextFunc(ComTerp* comterp) : StrmFunc(comterp) {
}

void FeedRingNextFunc::execute() {
  ComValue operand1(stack_arg(0));
  reset_stack();

  AttributeValueList* state = operand1.stream_list();
  if (!state || state->Number()<2) {
    push_stack(ComValue::nullval());
    return;
  }
  Iterator i;
  state->First(i);
  AttributeValue* ringav = state->GetAttrVal(i);  // [0] ring
  state->Next(i);
  AttributeValue* srcav = state->GetAttrVal(i);    // [1] source stream

  ComValue ringv(*ringav);
  AttributeValueList* avl = ringv.stream_list();
  if (!avl || ring_avail(avl)<=0) {
    push_stack(ComValue::nullval());
    return;
  }

  /* the ring is refused outright as its own source, directly or wrapped
     (e.g. nested inside a FIFO fed back into it), same guard ring_push_arg
     registers for its own eager drain: flagging this ring's avl as a
     draining destination for the span of this one pull is what makes a
     pull that bottoms out on it (through NextFunc::execute_impl's own
     recursive-stream check) return nil instead of cycling forever. */
  if (_draining_guard_tripped_avl==avl) _draining_guard_tripped_avl = 0;
  DrainingAVLGuard dest_guard(avl);

  /* srcav's own backing stream state lives behind this copy, same
     resume-across-calls idiom NextVarNextFunc uses for its paired streams --
     pulling through srccopy advances the shared source, so a later pull
     against this same wrapper resumes where this one left off. */
  ComValue srccopy(*srcav);
  NextFunc::execute_impl(comterp(), srccopy);
  ComValue popval(comterp()->pop_stack());
  if (_draining_guard_tripped_avl==avl) {
    _draining_guard_tripped_avl = 0;
    push_stack(ComValue::nullval());
    return;
  }
  if (popval.is_unknown() || StrmFunc::is_delimiter(popval)) {
    push_stack(ComValue::nullval());
    return;
  }
  if (!ring_push_one(avl, popval, false)) {
    push_stack(ComValue::nullval());
    return;
  }
  push_stack(popval);
}

/*****************************************************************************/


ArrowFunc::ArrowFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ArrowFunc::execute() {
  if (nargsfixed() < 2) {
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }

  ComValue lhsv(stack_arg_post_eval(0));
  boolean lhs_is_ring = lhsv.is_stream() &&
    lhsv.stream_func() == (void*)ring_next_func(comterp());

  /* flagging rhs's at() root token before it evaluates (same walk
     NextFunc::execute() uses for its own var argument) makes box@0 or
     box@lo:hi come back as a write target instead of the stored value. */
  /* gated on lhs_is_ring so a non-ring lhs (e.g. 'a'->rings@0) still
     reads rings@0's actual stored ring, not an unwritten [list,idx]. */
  static int at_symid = symbol_add("at");
  if (lhs_is_ring) {
    ComValue argoff(comterp()->stack_top());
    int offtop = argoff.int_val() - comterp()->pfnum();
    int argcnt = 0;
    skip_arg_in_expr(offtop, argcnt);
    int startidx = comterp()->pfnum() + offtop + argcnt - 1;
    ComValue& startval = comterp()->pfcomvals()[startidx];
    if (startval.is_type(ComValue::CommandType) &&
	((ComFunc*)startval.obj_val())->funcid() == at_symid)
      startval.lhs_assign(1);
  }

  /* a post_eval stack_arg() never resolves, so peeking rhs raw here costs
     nothing and keeps the bare name write_funcscope_symval needs below. */
  ComValue rhs_peek(stack_arg(1, true));

  /* rhs-ring-ness wins over the bare-variable case below, so a variable
     already holding a ring is fed into rather than overwritten -- what
     makes A->B->C chain. */
  ComValue rhsv(stack_arg_post_eval(1));
  reset_stack();

  boolean rhs_is_ring = rhsv.is_stream() &&
    rhsv.stream_func() == (void*)ring_next_func(comterp());

  if (rhs_is_ring) {
    AttributeValueList* avl = rhsv.stream_list();
    boolean ok = ring_push_arg(comterp(), avl, lhsv, false);
    push_stack(ok ? rhsv : ComValue::nullval());
    return;
  }

  if (lhs_is_ring) {
    int linenum = funcstate()->linenum();

    if (rhsv.is_stream() && rhsv.lhs_assign()) {
      /* a streamed at()-destination (box@lo:hi) -- zip-drive the ring
	 against it, same as next(ring box@lo:hi). */
      ComValue idxstream(rhsv);
      int count = NextFunc::zip_assign_stream(comterp(), idxstream, &lhsv, true, linenum);
      if (count < 0) {
	push_stack(ComValue::nullval());
	return;
      }
      ComValue retval(count, ComValue::IntType);
      push_stack(retval);
      comterp()->stack_top().wrapper(AttributeValue::BracketWrapper);
      return;
    }

    if (rhsv.is_array() && rhsv.lhs_assign()) {
      /* a scalar at()-destination (box@0) -- pull one value and write it
	 there, same as next(ring box@0). */
      ComValue streamcopy(lhsv);
      NextFunc::execute_impl(comterp(), streamcopy);
      ComValue pulled(comterp()->stack_top());
      NextFunc::write_at_pair(comterp(), rhsv, pulled);
      return;
    }

    if (rhs_peek.type() == ComValue::SymbolType) {
      ComValue streamcopy(lhsv);
      NextFunc::execute_impl(comterp(), streamcopy);
      ComValue* pulled = new ComValue(comterp()->stack_top());
      comterp()->write_funcscope_symval(rhs_peek.symbol_val(), pulled);
      return;
    }
  }

  push_stack(ComValue::nullval());
}
