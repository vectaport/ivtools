/*
 * Copyright (c) 2001-2009 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1994-1998 Vectaport Inc.
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

#include <cstdarg>
#include <cstdio>
#include <ctype.h>
#include <iostream.h>
#include <string.h>
#include <strstream>
#include <streambuf>
#include <new>
#include <unistd.h>

#include <fstream.h>

#include <ComTerp/comhandler.h>

#include <ComTerp/_comterp.h>
#include <ComTerp/_comutil.h>
#include <ComTerp/assignfunc.h>
#include <ComTerp/bitfunc.h>
#include <ComTerp/boolfunc.h>
#include <ComTerp/bquotefunc.h>
#include <ComTerp/charfunc.h>
#include <ComTerp/comfunc.h>
#include <ComTerp/comterp.h>
#include <ComUtil/comutil.h>
// #include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/condfunc.h>
#include <ComTerp/ctrlfunc.h>
#include <ComTerp/debugfunc.h>
#include <ComTerp/dotfunc.h>
#include <ComTerp/helpfunc.h>
#include <ComTerp/iofunc.h>
#include <ComTerp/listfunc.h>
#include <ComTerp/mathfunc.h>
#include <ComTerp/numfunc.h>
#include <ComTerp/parsefunc.h>
#include <ComTerp/postfunc.h>
#include <ComTerp/funcobjscan.h>
#include <ComTerp/randfunc.h>
#include <ComTerp/soundfunc.h>
#include <ComTerp/statfunc.h>
#include <ComTerp/strmfunc.h>
#include <ComTerp/symbolfunc.h>
#include <ComTerp/timefunc.h>
#include <ComTerp/typefunc.h>
#include <ComTerp/xformfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <OS/math.h>

#include <ctype.h>
#include <errno.h>
#include <iostream.h>
#include <string.h>
#include <strstream>
#include <unistd.h>
#include <fstream.h>

#ifdef LEAKCHECK
#include <leakchecker.h>
#include <vector>
#endif

#define TITLE "ComTerp"

/* a symbol that carried an arglist -- "SYMBOL (args)", a call attempt on
   something that wasn't a registered command when the token was converted */
#define PENDING_CALL(v) \
  ((v).is_type(ComValue::SymbolType) && ((v).narg() || (v).nkey()))

extern int _detail_matched_delims;
extern int _no_bracesplus;

using std::cerr;
using std::cout;

implementTable(ComValueTable,int,void*)

ComTerp* ComTerp::_instance = nil;
ComValueTable* ComTerp::_globaltable = nil;

/*****************************************************************************/

ComTerp::ComTerp() : Parser() {
    init();
}

ComTerp::ComTerp(const char* path) : Parser(path) {
    init();
}


ComTerp::ComTerp(void* inptr, char*(*infunc)(char*,int,void*), 
	       int(*eoffunc)(void*), int(*errfunc)(void*)) 
: Parser(inptr, infunc, eoffunc, errfunc) {
    init();
}

void ComTerp::init() {

    /* Save pointer to this instance */
    if (!_instance) 
	_instance = this;

    /* Allocate stack to initial size */
    _stack_top = -1;
    _stack_siz = 1024;
    if(dmm_calloc((void**)&_stack, _stack_siz, sizeof(ComValue)) != 0) 
	KANRET("error in call to dmm_calloc");

    /* Allocate funcstate stack to initial size */
    _fsstack_top = -1;
    _fsstack_siz = 256;
    if(dmm_calloc((void**)&_fsstack, _fsstack_siz, sizeof(ComFuncState)) != 0) 
	KANRET("error in call to dmm_calloc");

    /* Allocate servstate stack to initial size */
    _ctsstack_top = -1;
    _ctsstack_siz = 256;
    if(dmm_calloc((void**)&_ctsstack, _ctsstack_siz, sizeof(ComTerpState)) != 0) 
	KANRET("error in call to dmm_calloc");

    _pfoff = 0;
    _pfnum = 0;
    _quitflag = false;
    _returnflag = false;

    _pfcomvals = nil;

    /* Create ComValue symbol table */
    _localtable = new ComValueTable(100);
#if 0  /* deferred until first use */
    if (!_globaltable) {
      _globaltable = new ComValueTable(100);
    }
#endif

    _errbuf = new char[BUFSIZ];
    _errbuf2 = new char[BUFSIZ];
    _errbuf2[0] = '\0';

    _alist = nil;
    _brief = true;
    _just_reset = false;
    _defaults_added = false;
    _handler = nil;
    _val_for_next_func = nil;
    _func_for_next_expr = nil;
    _trace_mode = 0;
    _npause = 0;
    _stepflag = 0;
    _echo_postfix = 0;
    _delim_func = 0;
    _ignore_commands = 0;
    _autostream = 0;
    _running = 0;
    _muted = 0;
    _force_nested = 0;  /* read on every ComterpHandler::handle_input; left
                           uninitialized it intermittently nested/reset the stack
                           and added a stray pop_stack -> reactor-reentrancy crash */
    _fd = -1;
    _arg_strs = nil;
    _narg_strs = 0;
    _funcobj_argvals = nil;
    _funcobj_nargs = 0;
    _funcobj_active = false;
    _peek_scratch = new ComValue();
    _fire_scratch_pool = new AttributeValueList();
    _top_commands = NULL;
}


ComTerp::~ComTerp() {
    delete _peek_scratch;
    delete _fire_scratch_pool;
    /* Free stacks */
    if(dmm_free((void**)&_stack) != 0) 
	KANRET ("error in call to dmm_free");
    if(dmm_free((void**)&_fsstack) != 0) 
	KANRET ("error in call to dmm_free");
    if(dmm_free((void**)&_ctsstack) != 0) 
	KANRET ("error in call to dmm_free");

    delete _errbuf;
    delete _arg_strs;
    delete _top_commands;
}

const ComValue* ComTerp::stack(unsigned int &top) const {
    top = _stack_top;
    return _stack;
}

boolean ComTerp::read_expr() {
    check_parser_client();
    int status = parser (_inptr, _infunc, _eoffunc, _errfunc, (FILE*)_outptr, _outfunc,
			 _buffer, _bufsiz, &_bufptr, _token, _toksiz, &_linenum,
			 &_pfbuf, &_pfsiz, &_pfnum);

    _pfoff = 0;
    save_parser_client();    
    postfix_echo();

    return status==0 
      && (_pfnum==0 || _pfbuf[_pfnum-1].type != TOK_EOF) 
      && _buffer[0] != '\0';
}

void ComTerp::increment_linenum() {
    check_parser_client();
    _linenum++;
    save_parser_client();    
}

boolean ComTerp::eof() {

    return _pfnum ? _pfbuf[_pfnum-1].type == TOK_EOF : false;
}

boolean ComTerp::brief() const {
  return _brief;
}

int ComTerp::eval_expr(boolean nested) {
  if(_pfnum==0) return FUNCBAD;
  _pfoff = 0;
  delete [] _pfcomvals;
  _pfcomvals = nil;

  if (!nested) {
    _stack_top = -1;
    /* a genuinely new top-level statement -- nothing from here on can
       legitimately alias a fire_if_funcobj() result from a PRIOR
       statement, so it's safe to reclaim the pool now.  Reclaiming only
       ever happens across this boundary, never mid-statement
       (nested==true keeps appending to it instead), so two fires within
       one statement's evaluation -- however deeply nested -- always
       land in distinct, individually-allocated entries.  This is also
       the reset that bounds the pool's growth to one top-level
       statement rather than the process lifetime, even across a whole
       runfile() session -- see _fire_scratch_pool's own comment in
       comterp.h for how the next top-level entry (ComterpHandler::
       handle_input) always reaches this branch, with pause()'s
       force_nested(1) as the one deliberate exception. */
    _fire_scratch_pool->clear();
  }
  while (_pfoff < _pfnum) {
    load_sub_expr();
    eval_expr_internals();
    if (returnflag()) {
      break;
    }
  }
  return FUNCOK;
}

boolean ComTerp::top_expr() { return _pfoff >= _pfnum && NextFunc::next_depth()<=1; }

int ComTerp::eval_expr(ComValue* pfvals, int npfvals) {
  push_servstate();

  _pfoff = 0;
  _pfnum = npfvals;
  _pfcomvals = pfvals;

  while (_pfoff < _pfnum) {
    load_sub_expr();
    eval_expr_internals();
    if (returnflag()) break;
  }

  pop_servstate();

  return FUNCOK;
}

/* Bounds-safe snprintf accumulation into a fixed buffer -- plain
   'pos += snprintf(buf+pos, sizeof(buf)-pos, ...)' is unsafe to repeat:
   once the buffer is full, snprintf returns the length it WOULD have
   written (not what it actually wrote), so pos can end up past the
   buffer's end.  The next call then computes buf+pos as an out-of-bounds
   pointer and sizeof(buf)-pos as a size_t underflow (huge, since
   sizeof() is unsigned) -- snprintf believes it has nearly unlimited
   room and writes past the real allocation (Greptile, PR #337).  This
   clamps pos to stay valid after every call, so a signature long enough
   to fill the buffer truncates safely instead of overflowing it. */
static void append_bounded(char* buf, size_t bufsize, int& pos, const char* fmt, ...) {
  if (pos < 0 || (size_t)pos >= bufsize - 1) return;  /* already full/invalid -- skip */
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf + pos, bufsize - (size_t)pos, fmt, ap);
  va_end(ap);
  if (n < 0) return;  /* encoding error -- leave pos alone */
  pos += n;
  if ((size_t)pos > bufsize - 1) pos = (int)(bufsize - 1);  /* clamp for the next call */
}

/* renders a ComValue as text into a fixed buffer, for embedding into
   append_bounded's printf-style calls above (which have no %v of their
   own) -- same std::strstreambuf/ostream pattern already used elsewhere
   in this file (e.g. the stack_top() print path). */
static void render_comvalue(ComValue& v, char* out, size_t outsize) {
  std::strstreambuf sbuf;
  ostream os(&sbuf);
  os << v;
  os << '\0';
  strncpy(out, sbuf.str(), outsize - 1);
  out[outsize - 1] = '\0';
}

/* #334 (staged from #170 phase 1): the bare IO-contract signature for
   help(f) where f is a bare, unfired FuncObj -- see the fuller comment on
   ComTerp::describe_funcobj in comterp.h.  Positionals render as
   arg0/arg1/... (the arg(n) indices themselves, since a positional has no
   other name) or "..." when the count can't be pinned down statically (a
   computed index, or narg() usage -- see FuncObjVarScan::scan_positionals).
   Keywords are read-only union read-before-write only -- per #170's own
   framing, write-before-read is local scratch a caller's keyword would
   just be clobbering, not a real input, so it's omitted; escaping
   (local()/global()) vars are reported in a trailing annotation instead
   of the parens themselves, since they're not part of the func's own
   frame at all. */
ComValue ComTerp::describe_funcobj(FuncObj* fo) {
  boolean* is_plain_var = FuncObjVarScan::build_is_plain_var(this, fo->toks(), fo->ntoks());
  AttributeList* classification = FuncObjVarScan::classify(fo->toks(), fo->ntoks(), is_plain_var);
  ComValue classification_owner(AttributeList::class_symid(), (void*)classification);
  /* #336 (staged from #170's "Future" section): recognizes only the
     canonical if(x==nil :then DEFAULT :else x) idiom -- needs
     is_plain_var too, so computed before it's freed below. */
  AttributeList* defaults = FuncObjVarScan::scan_defaults(this, fo->toks(), fo->ntoks(), is_plain_var);
  ComValue defaults_owner(AttributeList::class_symid(), (void*)defaults);
  delete [] is_plain_var;

  FuncObjVarScan::PositionalInfo posinfo = FuncObjVarScan::scan_positionals(fo->toks(), fo->ntoks());

  char buf[2048];
  int pos = 0;
  append_bounded(buf, sizeof(buf), pos, "(");
  boolean first = true;

  if (posinfo.count < 0) {
    append_bounded(buf, sizeof(buf), pos, "...");
    first = false;
  } else {
    /* Reserve room for a " ... argMAX)" tail (worst case ~19 bytes for a
       10-digit index) so a huge literal index like arg(2000000000) -- the
       same repro as the DoS this loop's bound guards against, Greptile,
       PR #337 -- renders as many arg%d entries as fit and then names the
       true final index instead of just stopping mid-list with no
       indication anything was cut off. */
    const int tail_reserve = 32;
    int i = 0;
    for (; i < posinfo.count && pos < (int)sizeof(buf) - 1 - tail_reserve; i++) {
      append_bounded(buf, sizeof(buf), pos, first ? "arg%d" : " arg%d", i);
      first = false;
    }
    if (i < posinfo.count) {
      append_bounded(buf, sizeof(buf), pos, first ? "... arg%ld" : " ... arg%ld", posinfo.count - 1);
      first = false;
    }
  }

  ALIterator cit;
  for (classification->First(cit); !classification->Done(cit); classification->Next(cit)) {
    Attribute* attr = classification->GetAttr(cit);
    int kind = attr->Value()->int_val();
    if (kind == FuncObjVarScan::ReadOnly || kind == FuncObjVarScan::ReadBeforeWrite) {
      AttributeValue* defval = defaults->find(attr->SymbolId());
      /* #310's declaration-time capture can make the coded default
	 above unreachable: if this name already had a real value in
	 scope when func() ran, ReadOnly capture grabbed THAT value, not
	 nil -- the x==nil check inside the body will never be true for
	 as long as that capture stands, so the :then literal is
	 currently dead code.  Surface this rather than let :help claim
	 a default that the func will actually never produce. */
      AttributeValue* capval = nil;
      if (fo->captures().is_object(AttributeList::class_symid())) {
        capval = ((AttributeList*)fo->captures().obj_val())->find(attr->SymbolId());
      }
      boolean cap_shadows = capval && !ComValue(*capval).is_unknown();
      if (defval) {
        /* #336: render the detected default inline, :name [value] --
           comterp contracts are textually communicated wherever
           possible (postfix(help)'s trailing * for post-eval commands,
           help()'s own docstring/dockeys rendering); this is the same
           idea applied to a func's own IO contract. */
        char defbuf[256];
        ComValue defv(*defval);
        render_comvalue(defv, defbuf, sizeof(defbuf));
        if (cap_shadows) {
          char capbuf[256];
          ComValue capv(*capval);
          render_comvalue(capv, capbuf, sizeof(capbuf));
          append_bounded(buf, sizeof(buf), pos, first ? ":%s [%s, %s]" : " :%s [%s, %s]",
                          symbol_pntr(attr->SymbolId()), defbuf, capbuf);
        } else {
          append_bounded(buf, sizeof(buf), pos, first ? ":%s [%s]" : " :%s [%s]",
                          symbol_pntr(attr->SymbolId()), defbuf);
        }
      } else if (cap_shadows) {
        /* no coded default idiom at all, but this read-only keyword was
           still captured as a real value at declaration time -- worth
           showing too, same reasoning as above, just without a coded
           literal to contrast it against. */
        char capbuf[256];
        ComValue capv(*capval);
        render_comvalue(capv, capbuf, sizeof(capbuf));
        append_bounded(buf, sizeof(buf), pos, first ? ":%s [%s]" : " :%s [%s]",
                        symbol_pntr(attr->SymbolId()), capbuf);
      } else {
        append_bounded(buf, sizeof(buf), pos, first ? ":%s" : " :%s",
                        symbol_pntr(attr->SymbolId()));
      }
      first = false;
    }
  }
  append_bounded(buf, sizeof(buf), pos, ")");

  boolean any_escape = false;
  for (classification->First(cit); !classification->Done(cit); classification->Next(cit)) {
    Attribute* attr = classification->GetAttr(cit);
    int kind = attr->Value()->int_val();
    if (kind == FuncObjVarScan::EscapingLocal || kind == FuncObjVarScan::EscapingGlobal) {
      append_bounded(buf, sizeof(buf), pos, any_escape ? ", %s->%s" : "  -- escapes: %s->%s",
                      symbol_pntr(attr->SymbolId()),
                      kind == FuncObjVarScan::EscapingGlobal ? "global" : "local");
      any_escape = true;
    }
  }

  return ComValue(buf);
}

void ComTerp::fire_funcobj(ComValue& val, AttributeList* extra_keys, ComValue* lazy_posvals) {
  EvalFunc ef(this);
  /* keywords still build the body's locals (the _alist); the fixed
     positionals become the func's eager actual args, captured here so
     arg(n)/narg() can serve them inside the body (see funcobj_arg).
     The keywords sit above the positionals on the stack (no positionals
     after keywords), so pop them first.  narg() counts non-keyword args
     *including* values that follow keywords, and each keyword carries its
     own keynarg (0 for a bare flag), so the fixed-positional count is narg
     minus the keyword values actually consumed -- not narg-nkey.  (When
     extra_keys is supplied, narg() is already positional-only -- the
     caller's keywords never touched the shared stack -- so no post-
     keyword deduction applies there; see below.  When lazy_posvals is
     supplied, val.narg() is exactly its length -- nothing to deduct,
     nothing was pushed for the args at all.) */
  int npos = val.narg();
  AttributeList* al = new AttributeList();
  /* #310: seed al from this funcobj's own declaration-time captures
     (read-only/read-before-write free variables, funcobjscan.h)
     before keyword args land on top -- add_attr's replace-by-symid
     below then makes an explicit :x val keyword override a capture
     for free, no special-case needed. */
  FuncObj* callee_fo = (FuncObj*)val.obj_val();
  if (callee_fo->captures().is_object(AttributeList::class_symid())) {
    AttributeList* caps = (AttributeList*)callee_fo->captures().obj_val();
    ALIterator capit;
    for (caps->First(capit); !caps->Done(capit); caps->Next(capit)) {
      Attribute* capattr = caps->GetAttr(capit);
      al->add_attr(capattr->SymbolId(), *capattr->Value());
    }
  }
  if (extra_keys) {
    /* caller already built the call's keyword AttributeList some other way
       instead of leaving marker+value pairs on the shared stack --
       NilFunc's dynamic re-check (ComFunc::stack_keys_post_eval) for an
       eager target, or (a :posteval target) FuncObjPendingArg markers
       instead of real values.  Either way fire_funcobj just copies the
       entries into al -- it doesn't care whether they're real or pending,
       only funcobj_arg()/the bare-read fallthrough do.  Same add_attr call
       as the ordinary loop below, still lands after captures, so an
       explicit :x val keyword still overrides a capture for free. */
    ALIterator ekit;
    for (extra_keys->First(ekit); !extra_keys->Done(ekit); extra_keys->Next(ekit)) {
      Attribute* ekattr = extra_keys->GetAttr(ekit);
      al->add_attr(ekattr->SymbolId(), *ekattr->Value());
    }
  } else if (!lazy_posvals) {
    for(int i=0; i<val.nkey(); i++) {
      ComValue keyv(pop_stack());
      int knarg = keyv.keynarg_val();
      if (knarg==0) {
	al->add_attr(keyv.keyid_val(), ComValue::trueval());  /* :flag => flag true */
      } else {
	/* knarg is 0 or 1 by construction: the parser emits every keyword
	   token with narg 0 (bare flag) or 1 (keyword+value) -- the
	   TOK_KEYWORD PFOUT sites in ComUtil/_parser.c -- and keynarg is set
	   from token->narg (comterp.c:927).  So knarg>1 is unreachable; this
	   loop is written generally only.  Even if it ran, add_attr dedups by
	   symid (replaces, never appends), binding a single value, and every
	   value is popped so the positional count (npos) stays correct. */
	for(int j=0; j<knarg; j++) {
	  ComValue valv(pop_stack());
	  al->add_attr(keyv.keyid_val(), valv);
	  npos--;   /* a post-keyword value, not a fixed positional */
	}
      }
    }
  }
  if (npos<0) npos = 0;
  ComValue* posvals;
  if (lazy_posvals) {
    /* nothing to pop -- lazy_posvals' entries are ordinarily
       FuncObjPendingArg markers, pulled and memoized in place by
       funcobj_arg() the first time (if ever) arg(n) actually reads one. */
    posvals = lazy_posvals;
  } else {
    posvals = npos>0 ? new ComValue[npos] : nil;
    for(int i=npos-1; i>=0; i--) posvals[i] = pop_stack();
  }
  ComValue* saved_argvals = _funcobj_argvals;
  int saved_nargs = _funcobj_nargs;
  boolean saved_active = _funcobj_active;
  _funcobj_argvals = posvals;
  _funcobj_nargs = npos;
  _funcobj_active = true;
  push_stack(val);
  ComValue alv(AttributeList::class_symid(), al);
  push_stack(alv);
  static int alist_symid = symbol_add("alist");
  ComValue alkeyv(alist_symid, 1);
  push_stack(alkeyv);
  ef.exec(2, 1);
  _funcobj_argvals = saved_argvals;
  _funcobj_nargs = saved_nargs;
  _funcobj_active = saved_active;
  /* free any FuncObjPendingArg markers still standing at invocation end --
     AttributeValue::unref_as_needed() (Attribute/attrvalue.c) only knows
     how to clean up ArrayType/StreamType/StringType and (for ObjectType)
     AttributeList/Attribute specifically, nothing generic for an arbitrary
     ObjectType payload, so a marker nobody explicitly deletes just leaks.
     A positional's marker is usually still here regardless of whether
     arg(n) ever pulled it -- UNLESS the pulled result was a stream, in
     which case funcobj_arg() already replaced the slot with the real
     stream object and deleted the marker itself (pinning, not re-firing
     -- see its own comment); is_object() below correctly skips those,
     since they're no longer markers at all by the time we get here.  A
     keyword's marker only survives to here if it was never read at all
     -- one that was gets replaced by pull_alist_pending()'s (or, for a
     stream result, peek_alist_pending()'s) own add_attr call, which
     deletes the old marker there instead, right as it's overwritten. */
  for (int i=0; i<npos; i++) {
    if (posvals[i].is_object(FuncObjPendingArg::class_symid()))
      delete (FuncObjPendingArg*)posvals[i].obj_val();
  }
  ALIterator alit;
  for (al->First(alit); !al->Done(alit); al->Next(alit)) {
    AttributeValue* attrval = al->GetAttr(alit)->Value();
    if (attrval->is_object(FuncObjPendingArg::class_symid()))
      delete (FuncObjPendingArg*)attrval->obj_val();
  }
  delete [] posvals;
}

void ComTerp::eval_expr_internals(int pedepth) {
  static int step_symid = symbol_add("step");
  ComValue sv = pop_stack(false);

  /* ~~ spread expansion, upstream of the command/funcobj dispatch: if any of
     this call's args on the stack is a STREAM_SPREAD-tagged stream (left there
     by SpreadFunc), drain it in place so its elements become separate
     positionals for whatever consumes them -- an eager command via stack_arg,
     or a funcobj via its captured actuals.  Runs before the CommandType
     overdrive scan, so a tagged stream spreads rather than overdrives.  Skipped
     for post_eval commands, whose args are token-spans, not stack values. */
  if ((sv.type() == ComValue::CommandType &&
       !((ComFunc*)sv.obj_val())->post_eval()) ||
      sv.type() == ComValue::SymbolType) {
    int nall = sv.narg() + sv.nkey();
    boolean has_spread = false;
    for (int i = 0; i < nall && !has_spread; i++)
      has_spread = stack_top(-i).is_stream() &&
                   (stack_top(-i).stream_mode() & STREAM_SPREAD);
    if (has_spread) {
      /* pop the whole arg run off (top-down) preserving order, then rebuild it,
         draining each tagged stream in place -- mirrors overdrive's pop/rebuild.
         A tagged stream's N elements replace its single slot (added += N-1),
         so sv.narg() grows to the real positional count for both dispatch
         branches (CommandType reads sv.narg(); the funcobj branch reads it via
         lookup_symval carrying narg onto the looked-up value). */
      ComValue* saved = new ComValue[nall];
      for (int i = 0; i < nall; i++)
        saved[nall-1-i] = pop_stack(false);   /* saved[0] = bottom-most arg */
      int addpos = 0, addkey = 0;
      for (int j = 0; j < nall; j++) {
        ComValue v(saved[j]);
        if (v.is_stream() && (v.stream_mode() & STREAM_SPREAD)) {
          int npos = 0, nkey = 0;
          boolean done = false;
          while (!done) {
            NextFunc::execute_impl(this, v);
            if (stack_top().is_unknown()) { pop_stack(); done = true; }
            else if (stack_top().is_object(Attribute::class_symid())) {
              /* an Attribute element (from an attrlist) becomes a real
                 ":key value" keyword: push the value, then the keyword on top --
                 the order stack_key / the funcobj decode expect.  pop_stack(false)
                 is required: the default (lookupsym=true) tries to resolve the
                 object as a symbol and crashes.  SpreadFunc hands us owned
                 Attribute copies, so attr is valid across the deferred drain. */
              ComValue av(pop_stack(false));
              Attribute* attr = (Attribute*)av.obj_val();
              ComValue valv(*attr->Value());
              push_stack(valv);
              ComValue keyv((unsigned int)attr->SymbolId(), 1, ComValue::KeywordType);
              push_stack(keyv);
              nkey++;
            }
            else if (stack_top().is_attributelist()) {
              /* an attrlist element -- e.g. a tail singleton from echo's
                 (positionals..., attrlist-singletons) form -- spreads ALL its
                 attributes as keywords (same emit as a single Attribute,
                 iterated).  This inverts echo's mixed representation; a
                 singleton yields one keyword, a multi-attribute attrlist yields
                 several. */
              ComValue alv(pop_stack(false));
              AttributeList* al = (AttributeList*)alv.obj_val();
              Iterator it;
              for (al->First(it); !al->Done(it); al->Next(it)) {
                Attribute* attr = al->GetAttr(it);
                ComValue valv(*attr->Value());
                push_stack(valv);
                ComValue keyv((unsigned int)attr->SymbolId(), 1, ComValue::KeywordType);
                push_stack(keyv);
                nkey++;
              }
            }
            else
              npos++;                    /* a plain positional stays on the stack */
          }
          /* narg counts non-keyword args INCLUDING the values that follow
             keywords, so each emitted keyword's value counts too; the tagged
             ~~ slot was itself 1 narg, hence the -1. */
          addpos += (npos + nkey - 1);
          addkey += nkey;
        } else
          push_stack(v);
      }
      delete [] saved;
      sv.narg(sv.narg() + addpos);
      sv.nkey(sv.nkey() + addkey);
    }
  }

  /* a funcobj call with a stream arg and no :posteval overdrives like a
     command call -- the stream drives the INVOCATION, firing the body once per
     element with arg(n) bound to a scalar, so control flow inside the body is
     ordinary scalar code.  :posteval is excluded: its contract is the opposite,
     internal one, where the arguments stay unevaluated and the body itself
     drains the pinned stream with *arg(n).  Only symbol-bound funcobjs reach
     here; that is every named call site. */
  if (sv.type() == ComValue::SymbolType && (sv.narg() || sv.nkey())) {
    AttributeValue* funcval = lookup_symval(&sv, false);
    if (funcval && funcval->is_object(FuncObj::class_symid()) &&
	!((FuncObj*)funcval->obj_val())->posteval()) {
      boolean has_streams = false;
      for(int i=0; i<sv.narg()+sv.nkey(); i++) {
	if (!stack_top(-i).is_symbol() && !stack_top(-i).is_attribute())
	  has_streams = stack_top(-i).is_stream();
	else if (stack_top(-i).is_symbol() &&
		 is_posteval_pending(stack_top(-i).symbol_val()))
	  has_streams = false;   /* same rule as the CommandType scan below */
	else {
	  AttributeValue* testval = lookup_symval(&stack_top(-i), false);
	  has_streams = testval ? testval->is_stream() : false;
	}
	if (has_streams) break;
      }
      if (has_streams) {
	AttributeValueList* avl = new AttributeValueList();
	for(int i=0; i<sv.narg()+sv.nkey(); i++) {
	  /* resolve every stream-valued arg, so a stream held in a variable
	     zips per-element like a stream literal instead of arriving as an
	     unresolved symbol; scalars stay unresolved for per-element
	     broadcast -- identical to the CommandType pack below. */
	  boolean argstream;
	  if (!stack_top().is_symbol() && !stack_top().is_attribute())
	    argstream = stack_top().is_stream();
	  else {
	    AttributeValue* tv = lookup_symval(&stack_top(), false);
	    argstream = tv ? tv->is_stream() : false;
	  }
	  ComValue topval(pop_stack(argstream));
	  avl->Prepend(new AttributeValue(topval));
	}
	/* the FuncObj rides in the same void* slot a ComFunc* normally uses;
	   STREAM_FUNCOBJ tells NextFunc to fire it rather than exec() it. */
	ComValue strmval((void*)funcval->obj_val(), avl);
	strmval.stream_mode(STREAM_EXTERNAL|STREAM_FUNCOBJ);
	push_stack(strmval);
	return;
      }
    }
  }

  if (sv.type() == ComValue::CommandType) {

    /* if func has StreamType ComValue's for arguments */
    /* create another StreamType ComValue to hold all its */
    /* arguments, along with a pointer to the func. */
    boolean has_streams = false;
    if (!((ComFunc*)sv.obj_val())->post_eval())
      for(int i=0; i<sv.narg()+sv.nkey(); i++) {
	if (!stack_top(-i).is_symbol() && !stack_top(-i).is_attribute())
	  has_streams = stack_top(-i).is_stream();
	else if (stack_top(-i).is_symbol() &&
		 is_posteval_pending(stack_top(-i).symbol_val())) {
	  /* a still-pending :posteval operand can't answer "am I a stream"
	     without being pulled, and overdrive is an upfront, whole-
	     expression decision made once here -- not something a later,
	     single-value resolution (stack_arg/stack_key) could retrofit
	     per operand the way a bare-funcobj fire can (see is_funcobj's
	     own comment).  So it never overdrives: leave it unpulled,
	     treat as not-a-stream, and let it resolve to whatever it
	     resolves to -- stream or not -- as an ordinary scalar operand
	     when it's actually consumed. */
	  has_streams = false;
	}
	else {
	  AttributeValue* testval =
	    lookup_symval(&stack_top(-i), false);
	  has_streams = testval ? testval->is_stream() : false;
	}
	if (has_streams)
	  break;   // any stream arg triggers the overdrive; the pack loop
		   // below re-detects stream-ness per arg (no streamid needed)
      }
    if (has_streams) {
      AttributeValueList* avl = new AttributeValueList();
      
      /* if delims associated with symbol, put that first in stream list */
      if (_delim_func && sv.nids()!=1) {
	ComValue nameval(sv.command_symid(), ComValue::SymbolType);
	avl->Prepend(new AttributeValue(nameval));
      }

      for(int i=0; i<sv.narg()+sv.nkey(); i++) {
	/* Resolve EVERY stream-valued arg, not just the first one found.
	   A stream held in a variable arrives here as a symbol; left unresolved
	   (as the old pop_stack(i==streamid) did for all but the first) it is not is_stream()
	   in the AVL, so the per-element zip treats it as a whole non-stream
	   argument -- which is why stream-var*stream-var (and var*literal)
	   returned only the left operand's elements.  Resolving it makes it a
	   stream value that zips per-element like a stream literal.  Scalar args
	   stay unresolved (symbols) for per-element re-evaluation (broadcast). */
	boolean argstream;
	boolean alist_bound = false;
	if (!stack_top().is_symbol() && !stack_top().is_attribute())
	  argstream = stack_top().is_stream();
	else {
	  /* a symbol bound through _alist (a func-local keyword or #310
	     capture) is fixed for the life of this call -- nothing
	     legitimately mutates it mid-broadcast, so there's no "re-read
	     fresh each iteration" benefit to leaving it as a deferred
	     symbol the way a true outer-scope global's comment above
	     intends.  Worse, leaving it deferred is actively wrong: the
	     packed value returned here can propagate out past this call
	     (e.g. as the func's own return value, driven forward later by
	     whatever pulls it -- list(), an outer print(), etc.), and by
	     then _alist no longer points at this call's AttributeList at
	     all, so the deferred read silently falls through to global
	     scope instead (#343).  Resolve now, same as a genuinely
	     stream-valued operand already does, whenever the symbol
	     resolves through the CURRENT _alist specifically; a true
	     global stays deferred, unchanged. */
	  if (!stack_top().global_flag() && _alist &&
	      _alist->find(stack_top().symbol_val()))
	    alist_bound = true;
	  AttributeValue* tv = lookup_symval(&stack_top(), false);
	  argstream = tv ? tv->is_stream() : false;
	}
	ComValue topval(pop_stack(argstream || alist_bound));
	avl->Prepend(new AttributeValue(topval));
      }

      ComValue val((ComFunc*)sv.obj_val(), avl);
      // fprintf(stderr, "comterp::eval_expr_internals:  packed up stream for %s\n", symbol_pntr(((ComFunc*)sv.obj_val())->funcid()));
      val.stream_mode(STREAM_EXTERNAL); // for external use
      push_stack(val);
      return;
    }

    ComFunc* func = nil;
    int nargs = sv.narg();
    int nkeys = sv.nkey();
    int func_for_next_expr_post_eval = 0;
    if (_func_for_next_expr) {
      func = _func_for_next_expr;
      _func_for_next_expr = nil;
      push_stack(sv);
      func->push_funcstate(1, 0, pedepth, func->funcid());
    } else {   
      func = (ComFunc*)sv.obj_val();
      if (_delim_func && sv.nids()!=1) {
	ComValue nameval(sv.command_symid(), ComValue::SymbolType);
	push_stack(nameval);  // this assumes it will be immediately popped off the stack
	if (!func->post_eval()) 
	  nargs++;
	else
	  func_for_next_expr_post_eval = 1;
      }
      /* sv.command_symid(), not func->funcid(): for the ordinary case
	 they're the same symbol (sv resolved to exactly the command it
	 named), but token_to_comvalue's nil-substitution fallback dispatches
	 an unresolved call-shaped symbol to the single shared NilFunc
	 instance while still recording the ORIGINAL requested name in
	 sv.command_symid() -- func->funcid() would always read "nil" there,
	 losing the name NilFunc needs to dynamically re-check (issue #328). */
      func->push_funcstate(nargs, nkeys, pedepth, sv.command_symid(), sv.linenum());
    }

    /* output execution trace */
    if (this->trace_mode()) {
      int ln = func->funcstate()->linenum();
      if(ln<100) cout << " ";
      if(ln<10) cout << " ";
      cout << func->funcstate()->linenum() << ":  ";
      for(int i=0; i<pedepth; i++) cout << "    ";
      cout << symbol_pntr(sv.command_symid());
      if (func->post_eval()) 
	cout << ": nargs=" << nargs << " nkeys=" << nkeys << "\n";
      else {
	int ntotal = func->nargs() + func->nkeys();
	for(int i=0; i<ntotal; i++) {
	  if (i) 
	    cout << " ";
	  else 
	    cout << "(";
	  cout << stack_top(i-ntotal+1);
	}
	cout << ")\n";
      }
    }

    if (stepflag()) {
      int fd = 	handler() ? handler()->wrfd() : 1;
      FILE* fp = fdopen(dup(fd), "w");
      FILEBUF(fbufout, fp, ios_base::out);
      ostream out(&fbufout);
      out << ">>> " << *func << "(" << *func->funcstate() << ")\n";
      static int pause_symid = symbol_add("pause");
      ComValue pausekey(pause_symid, 0, ComValue::KeywordType);
      push_stack(pausekey);
      ComterpStepFunc stepfunc(this);
      stepfunc.push_funcstate(0,1, pedepth, step_symid);
      stepfunc.execute();
      stepfunc.pop_funcstate();
      pop_stack();
    }

    int stack_base = _stack_top;
    if (!func->post_eval()) 
      stack_base -= nargs+nkeys;
    else {
      stack_base -= 1;
      stack_base -= func_for_next_expr_post_eval;
    }

    func->execute();
    int linenum = func->funcstate()->linenum();
    func->pop_funcstate();

    if (_just_reset && !_func_for_next_expr) {
      push_stack(ComValue::blankval());
      _just_reset = false;
    }

    if (stack_base+1 < _stack_top) {
      fprintf(stderr, "func \"%s\" pushed more than a single value on stack (line %d)\n", symbol_pntr(func->funcid()), linenum);
      fprintf(stderr, "stack_base %d, stack_top %d\n", stack_base, _stack_top);
      for(int i=stack_base+1; i<=_stack_top; i++)
          std::cerr << i << ":  " << _stack[i] << "\n";
      /* Trim the stray extra value(s) rather than leaving them behind: an
         under-consumed post_eval command here (observed from deeply nested
         stream/next() recursion) otherwise leaves residue on the shared
         value stack that outlives this statement -- since eval_expr() only
         resets _stack_top between top-level statements when !nested, a
         run("file") session (nested=true, see runfile()'s eval_expr(true))
         carries the stray value into every later statement, where an
         unrelated post_eval command's stack_top()-based argoff anchor lookup
         misreads it as its own bookmark (comfunc.c's stack_arg_post family),
         cascading into "offlimit hit by ComTerp::skip_arg" and similar
         postfix-bookkeeping errors far from the true cause. */
      decr_stack(_stack_top - (stack_base+1));
    }
    else if (stack_base+1 > _stack_top) {
      fprintf(stderr, "func \"%s\" failed to push a single value on stack\n", symbol_pntr(func->funcid()));
      /* secondary backstop, not the primary handler: a command whose last
         stack-affecting act was reset_stack() and nothing else is already
         caught above by the _just_reset check (comfunc.c's reset_stack()
         sets it, any push_stack() clears it) and backfilled with blankval()
         there -- that's the common case, and by the time we get here it has
         already restored stack_base+1.  This branch only still fires for a
         command that shorts the stack some other way, bypassing
         reset_stack() entirely.  Use the same blankval() sentinel as that
         mechanism, not nullval()/nil -- they're not interchangeable
         elsewhere (is_blank() vs is_nil() are checked separately, and a
         stray nil reads differently than "no value here" placeholder). */
      push_stack(ComValue::blankval());
    }

    return;
    
  }

  if (sv.type() == ComValue::SymbolType) {

    if (_func_for_next_expr) {
      ComFunc* func = _func_for_next_expr;
      _func_for_next_expr = nil;

      push_stack(sv);
      func->push_funcstate(1, 0, pedepth, func->funcid());
      func->execute();
      func->pop_funcstate();
      if (_just_reset && val_for_next_func().is_null()) {
	push_stack(ComValue::blankval());
	_just_reset = false;
      }

    } else {
      
      if (_alist) {
	// cerr << "looking up " << sv.symbol_ptr() << " (" << _alist << ")\n";
	int id = sv.symbol_val();
	/* a :posteval keyword arg that hasn't been read yet sits here as a
	   FuncObjPendingArg marker (postfunc.h) instead of a real value --
	   peek_alist_pending() pulls it fresh on every read, never memoizing
	   (see its own comment in comterp.h): an unwritten :posteval keyword
	   behaves like a live tap, re-evaluated on each access, same as
	   arg(n) already does.  A keyword the body only ever writes, or
	   never reads at all, never reaches here with a write first
	   (AssignFunc writes directly, it doesn't read through this path)
	   -- true laziness, not just "resolved late". */
	AttributeValue* val = peek_alist_pending(_alist, id, _alist->find(id));
	/* a func-local FuncObj falls through to the same fire-check below as
	   every other symbol reference, instead of returning early -- a
	   standalone variable is a niladic call site regardless of whether
	   it's read from a func's own local frame or from local/global scope
	   (comterp.c:566's lookup_symval(sv) rechecks _alist first anyway, so
	   this isn't a second, different lookup -- just the same one, minus
	   the early exit that previously skipped the FuncObj check). */
	if (val && !val->is_object(FuncObj::class_symid())) {
	  ComValue newval(*val);
	  /* a func-local plain value drains a pending arglist the same way
	     the local/global case below does -- otherwise the args stay
	     stranded under the result */
	  decr_stack(sv.narg() + sv.nkey());
	  push_stack(newval);
	  return;
	}
      }

      // cerr << "looking up " << sv.symbol_ptr() << "\n";
      const char* funcname = sv.symbol_ptr();
      ComValue val = lookup_symval(sv);
      if(val.is_object(FuncObj::class_symid())) {
	fire_funcobj(val);
      } else {
	/* sv carried a pending arglist (same-line adjacency to a following
	   paren group always means "this is a call attempt"), but val --
	   what the symbol actually resolves to -- isn't a FuncObj.  Its
	   narg()+nkey() worth of args were already eagerly evaluated (the
	   same way they would be for a real command) and are sitting on
	   the stack below where val is about to go; discard them here the
	   same way a real command's own reset_stack() would, rather than
	   stacking val on top of them.  This generalizes true()/false()/
	   pi()'s existing "wrap any expression, override its result" idiom
	   to any plain value: the wrapped expression still runs for its
	   side effects, but the wrapper's own value always wins. */
	decr_stack(sv.narg() + sv.nkey());
	push_stack(val);
      }
    }

    return;

  }

  if (sv.is_object(Attribute::class_symid())) {

    push_stack(*((Attribute*)sv.obj_val())->Value());
    return;
    
  }

  if (sv.type() == ComValue::BlankType) {

    if (!stack_empty()) eval_expr_internals(pedepth);
    return;

  }

  /* everything else*/
  push_stack(sv);
  return;

}

void ComTerp::load_sub_expr() {

  /* initialize arrays of ComValue's wrapped around ComFunc's */
  /* and counters that indicate depth of post-eval operators  */
  if (!_pfcomvals) {
    _pfcomvals = new ComValue[_pfnum];
    for (int i=_pfnum-1; i>=0; i--) {
      ComValue* sv = _pfcomvals + i;
      token_to_comvalue(_pfbuf+i, sv);
    }
    int offset = 0;
    for (int j=_pfnum-1; j>=0; j--) {
      ComValue* sv = _pfcomvals + j;
      if (sv->is_type(ComValue::CommandType)) {
	ComFunc* func = (ComFunc*)sv->obj_val();
	if (func && func->post_eval()) {
	  int newoffset = offset;
	  skip_func(_pfcomvals+_pfnum-1, newoffset, -_pfnum);
	  int start = j-1;
	  int stop = _pfnum+newoffset;
	  for (int k=start; k>=stop; k--) 
	    _pfcomvals[k].pedepth()++;
	}
      }
      offset--;
    }
  }

  /* skip pushing values on stack until _postevaldepth is 0 */
  /* push all the zero-depth things until you get a CommandType */
  while (_pfoff < _pfnum ) {
    if (_pfcomvals[_pfoff].pedepth()) {
      _pfoff++;
      continue;
    }
    if (_pfcomvals[_pfoff].is_type(ComValue::CommandType)) {
      ComFunc* func = (ComFunc*)_pfcomvals[_pfoff].obj_val();
      if (func && func->post_eval()) {
	ComValue argoffval(_pfoff);
	push_stack(argoffval);
      }
    }
    if (!_pfcomvals[_pfoff].is_blank()) {
      push_stack(_pfcomvals[_pfoff]);
    } else {
      /* to handle a list as the 1st operand of the tuple operator */
      if (stack_top(0).is_array()) {
	stack_top(0).array_val()->nested_insert(true);
      } else if (stack_top(0).is_symbol()) {
        AttributeValue* av = lookup_symval(&stack_top(0), false);
	if (av && av->is_array()) av->array_val()->nested_insert(true);
      }
    }
    _pfoff++;
    /* A bare funcobj that is the RHS of a dot is an attribute name, not a
       call: don't fire it -- leave it on the stack as a symbol for the dot
       command (the next token) to read.  The RHS of a dot attribute should
       not look up a zero-arg funcobj. */
    boolean funcobj_top = stack_top().is_funcobj(this);
    /* Same break for a symbol carrying a pending arglist: "SYMBOL (args)"
       is always a call attempt (LANGUAGE.md), so it has to be dispatched
       here like a command -- eval_expr_internals is what fires a FuncObj
       or drains a plain value's arglist.  Without the break the symbol
       sits on the stack with its already-evaluated args stranded under
       it, so the next operator reads those as its operands and one entry
       leaks per evaluation. */
    boolean pending_call_top = PENDING_CALL(stack_top());
    if ((funcobj_top || pending_call_top) && _pfoff < _pfnum &&
	_pfcomvals[_pfoff].is_type(ComValue::CommandType)) {
      static int dot_symid = symbol_add("dot");
      if (_pfcomvals[_pfoff].command_symid() == dot_symid)
	funcobj_top = pending_call_top = false;
    }
    if ((stack_top().type() == ComValue::CommandType || funcobj_top ||
	 pending_call_top) && !_pfcomvals[_pfoff-1].pedepth()) break;
  }
  
#if 0

    /* find the index of the last (or outermost) */
    /* post_eval command in the postfix buffer */
    int top_post_eval = -1;
    int pfptr = _pfnum-1;
    while (pfptr > _pfoff ) {
      
        void *vptr = nil;

	/* look up ComFunc and check post_eval flag */
        if (_pfbuf[pfptr].type==TOK_COMMAND)
	  localtable()->find(vptr, _pfbuf[pfptr].v.dfintval);
        ComValue* comptr = (ComValue*)vptr;

        if (comptr && comptr->is_type(AttributeValue::CommandType)) {
	    ComFunc* comfunc = (ComFunc*)comptr->obj_val();
	    if (comfunc && comfunc->post_eval()) {
	        top_post_eval = pfptr;
		break;
	    }
	}
        pfptr--;
    }

    /* push tokens onto the stack until the last post_eval command is pushed */
    /* or if none, the first !post_eval command is pushed */
    while (_pfoff < _pfnum ) {
        push_stack(_pfbuf + _pfoff);
        _pfoff++;
	if (stack_top().type() == ComValue::CommandType && 
	(top_post_eval<0 || top_post_eval == _pfoff-1) ) break;
    }

    /* count down on stack to determine the number of */
    /* args associated with keywords for this command */
    if (stack_top().type() == ComValue::CommandType && top_post_eval<0) {
      int nargs_after_key = 0;
      for (int i=0; i<_pfbuf[_pfoff-1].narg+_pfbuf[_pfoff-1].nkey; i++) {
	ComValue& val = stack_top(-i-1);
	if (val.is_type(ComValue::KeywordType))
	  nargs_after_key += val.keynarg_val();
      }
      return nargs_after_key;
    } else
      return 0;
#endif    
}


int ComTerp::post_eval_expr(int tokcnt, int offtop, int pedepth 
#ifdef POSTEVAL_EXPERIMENT
			    , int nolookup 
#endif
			    ) {
#ifdef POSTEVAL_EXPERIMENT
  int numtok = tokcnt;
#endif
  if (tokcnt) {
    int offset = _pfnum+offtop;
    while (tokcnt>0) {
      while (tokcnt>0) {
	if (_pfcomvals[offset].pedepth()==pedepth) {
	  if (_pfcomvals[offset].is_type(ComValue::CommandType)) {
	    ComFunc* func = (ComFunc*)_pfcomvals[offset].obj_val();
	    if (func && func->post_eval()) {
	      ComValue argoffval(offset);
	      push_stack(argoffval);
	    }
	  }
	  if (!_pfcomvals[offset].is_blank()) {
	    push_stack(_pfcomvals[offset]);
	  } else {
	    /* to handle a list as the 1st operand of the tuple operator */
	    if (stack_top(0).is_array()) {
	      stack_top(0).array_val()->nested_insert(true);
	    } else if (stack_top(0).is_symbol()) {
	      AttributeValue* av = lookup_symval(&stack_top(0), false);
	      if (av->is_array()) av->array_val()->nested_insert(true);
	    }
	  }
	}
	tokcnt--;
	offset++;
	if (_pfcomvals[offset-1].pedepth()!=pedepth)
	  continue;
	/* same dot-RHS funcobj suppression as the main push loop: a bare funcobj
	   that is the RHS of a dot is an attribute name, not a call (here in the
	   post-eval path, e.g. inside && / if). */
	boolean pe_funcobj_top = stack_top().is_funcobj(this);
	/* and the same pending-arglist break as the main push loop */
	boolean pe_pending_call_top = PENDING_CALL(stack_top());
	if ((pe_funcobj_top || pe_pending_call_top) && offset < _pfnum &&
	    _pfcomvals[offset].is_type(ComValue::CommandType)) {
	  static int dot_symid = symbol_add("dot");
	  if (_pfcomvals[offset].command_symid() == dot_symid)
	    pe_funcobj_top = pe_pending_call_top = false;
	}
	if ((stack_top().is_type(ComValue::CommandType) || pe_funcobj_top ||
	     pe_pending_call_top) && stack_top().pedepth() == pedepth) break;
      }
#ifdef POSTEVAL_EXPERIMENT 
      if (!(stack_top().is_symbol()&&numtok==1&&nolookup))
#endif
      eval_expr_internals(pedepth);
      
    }
  }
  return FUNCOK;
}

void ComTerp::print_post_eval_expr(int tokcnt, int offtop, int pedepth ) {
  ComValue topval;
  if (tokcnt) {
    int offset = _pfnum+offtop;
    while (tokcnt>0) {
      while (tokcnt>0) {
	if (_pfcomvals[offset].pedepth()==pedepth) {
	  if (_pfcomvals[offset].is_type(ComValue::CommandType)) {
	    ComFunc* func = (ComFunc*)_pfcomvals[offset].obj_val();
	    if (func && func->post_eval()) {
	      ComValue argoffval(offset);
	      cout << argoffval << " ";
              topval = argoffval;
	    }
	  }
	  if (!_pfcomvals[offset].is_blank()) {
 	    cout << _pfcomvals[offset] << " ";
            topval = _pfcomvals[offset];
          }
	}
	tokcnt--;
	offset++;
	if (_pfcomvals[offset-1].pedepth()!=pedepth)
	  continue;
	if (topval.is_type(ComValue::CommandType) 
	    && topval.pedepth() == pedepth) break;
      }

      cout << "| ";
      
    }
  }
  cout << "\n";
}

postfix_token* ComTerp::copy_post_eval_expr(int tokcnt, int offtop) {
  postfix_token* tokbuf = new postfix_token[tokcnt];
  int offset = _pfnum+offtop;
  for(int i=0; i<tokcnt; i++) {
    tokbuf[i] = _pfbuf[i+offset];
    if (tokbuf[i].type==TOK_STRING)
      symbol_reference(tokbuf[i].v.symbolid);
  }
  return tokbuf;
}

boolean ComTerp::skip_func(ComValue* topval, int& offset, int offlimit) {
  ComValue* sv = topval + offset;
  int nargs = sv->narg();
  int nkeys = sv->nkey();
  if (offlimit == offset) {
    cerr << "offlimit hit by ComTerp::skip_func\n";
    return false;
  }
  offset--;
  while(nargs>0 || nkeys>0) {
    ComValue* nv = topval + offset;
    int tokcnt;
    if (nv->is_type(ComValue::KeywordType)) {
      skip_key(topval, offset, offlimit, tokcnt);
      nkeys--;
      nargs -= tokcnt ? 1 : 0;
    } else {
      skip_arg(topval, offset, offlimit, tokcnt);
      nargs--;
    }
  }
  return true;
}

boolean ComTerp::skip_key(ComValue* topval, int& offset, int offlimit, int& tokcnt) {
  ComValue& curr = *(topval+offset);
  tokcnt = 0;
  if (curr.is_type(ComValue::KeywordType)) {
    if (offlimit == offset) {
      cerr << "offlimit hit by ComTerp::skip_key\n";
      return false;
    }
    offset--;
    if (curr.keynarg_val()) {
      int subtokcnt;
      skip_arg(topval, offset, offlimit, subtokcnt);
      tokcnt += subtokcnt;
    }

    return true;
  }
  return false;
}

boolean ComTerp::skip_arg(ComValue* topval, int& offset, int offlimit, int& tokcnt) {
  tokcnt = 0;
  ComValue& curr = *(topval+offset);
  // fprintf(stderr, "offset is %d, topval is at 0x%lx\n", offset, topval);
  if (curr.is_type(ComValue::KeywordType)) {
    cerr << "unexpected keyword found by ComTerp::skip_arg\n";
    return false;
#if 0
  } else if (curr.is_type(ComValue::UnknownType)) {
    cerr << "unexpected nil found by ComTerp::skip_arg\n";
    return false;
#endif
  } else if (curr.is_type(ComValue::BlankType)) {
    if (offlimit == offset) {
      cerr << "offlimit hit by ComTerp::skip_arg\n";
      return false;
    }
    offset--;
    boolean val = skip_arg(topval, offset, offlimit, tokcnt);
    tokcnt++;
    return val;
  } else {
    if (offlimit == offset) {
      cerr << "offlimit hit by ComTerp::skip_arg\n";
      return false;
    }
    offset--;
    tokcnt++;

    if (curr.narg() || curr.nkey()) {
      int count = 0;
      while (count<(curr.narg() + curr.nkey())) {
	ComValue& next = *(topval+offset);
	int subtokcnt = 0;
	if (next.is_type(ComValue::KeywordType)) {
	  skip_key(topval, offset, offlimit, subtokcnt);
	  tokcnt += subtokcnt + 1;
	  if (subtokcnt) count++;
	} else if (next.is_type(ComValue::CommandType) ||
		   next.is_type(ComValue::SymbolType)) {
	  skip_arg(topval, offset, offlimit, subtokcnt);
	  tokcnt += subtokcnt;
	} else if (next.is_type(ComValue::BlankType)) {
	  if (offlimit == offset) {
	    cerr << "offlimit hit by ComTerp::skip_arg\n";
	    return false;
	  }
	  offset--;
	  skip_arg(topval, offset, offlimit, subtokcnt);
	  tokcnt += subtokcnt+1;
	} else {
	  if (offlimit == offset) {
	    cerr << "offlimit hit by ComTerp::skip_arg\n";
	    return false;
	  }
	  offset--;
	  tokcnt++;
	}
	count++;
      }
    }
    return true;
  }
}

ComValue& ComTerp::expr_top(int n) {
  /* the slot actually read is _pfcomvals[_pfnum-1+n], so that index must be >=0:
     reject _pfnum+n < 1, not < 0.  the old < 0 let _pfnum+n==0 through and read
     _pfcomvals[-1] (unmapped) -- the SIGBUS seen when a corrupt/zero stack anchor
     drives offtop to -_pfnum.  n>0 still rejects reads above the post-eval top. */
  if (((int)_pfnum)+n < 1 || n>0) {
    return ComValue::unkval();
  }
  else
    return _pfcomvals[_pfnum-1+n];
}


int ComTerp::print_stack() const {
    print_stack(cout);
    return true;
}

int ComTerp::print_stack(std::ostream& out) const {
    for (int i = _stack_top; i >= 0; i--) {
	out << _stack[i] << "\n";
    }
    out.flush();
    return true;
}

int ComTerp::print_stack_top() const {
    if (_stack_top < 0) return true;
    ComValue::comterp(this);
    ComValue cv(_stack[_stack_top]);
    fprintf(stdout, "%s\n", cv.String());
    return true;
}

int ComTerp::print_stack_top(ostream& out) const {
    if (_stack_top < 0) return true;
    ComValue::comterp(this);
    out << _stack[_stack_top];
    return true;
}

void ComTerp::push_stack(postfix_token* token) {
    if (_stack_top+1 == _stack_siz) {
	int old_siz = _stack_siz;
	_stack_siz *= 2;
	dmm_realloc_size(sizeof(ComValue));
	if(dmm_realloc((void**)&_stack, (unsigned long)_stack_siz) != 0) {
	    KANRET("error in call to dmm_realloc");
	    return;
	}
	/* dmm_realloc leaves the grown region raw, unlike the initial dmm_calloc.
	   the slots are assigned via operator=, whose assignval calls
	   unref_as_needed() on the *destination* -- on garbage that reads back as a
	   ref-counted _type that path does Resource::unref(garbage_ptr) and crashes.
	   default-construct each grown slot to a clean UnknownType (vtable intact,
	   _v zeroed) so that unref is a no-op -- matching the calloc'd initial region
	   and the destructor's type(UnknownType). */
	for (int k = old_siz; k < _stack_siz; k++)
	    new (_stack + k) ComValue();
    }
    _stack_top++;
    token_to_comvalue(token, _stack + _stack_top);

    _just_reset = false;
}

void ComTerp::token_to_comvalue(postfix_token* token, ComValue* sv) {
  *sv = ComValue(token);
  
  /* See if this really is a command with a ComFunc */
  if (sv->type() == ComValue::SymbolType) {
    void* vptr = nil;
    unsigned int command_symid = sv->int_val();
    if(!ignore_commands()) 
      localtable()->find(vptr, command_symid);
    else if (strncmp(sv->symbol_ptr(), "__", 2)==0) {
      int bufsiz = strlen(sv->symbol_ptr());
      std::vector<char> buf(bufsiz);
      strcpy(&buf[0], sv->symbol_ptr()+2);
      command_symid = symbol_add(&buf[0]);
      localtable()->find(vptr, command_symid);
    }

    /* handle case where symbol has matched parens, and things are set up to invoke a delim-specific func. */
    if (/*!vptr && */ _delim_func && sv->nids() != 1) {
      if (sv->nids() == TOK_RPAREN) {
	static int parens_symid =  symbol_add("()");
	localtable()->find(vptr, parens_symid);
      }
      if (sv->nids() == TOK_RBRACKET) {
	static int brackets_symid =  symbol_add("[]");
	localtable()->find(vptr, brackets_symid);
      }
      else if (sv->nids() == TOK_RBRACE) {
	static int braces_symid =  symbol_add("{}");
	localtable()->find(vptr, braces_symid);
      }
      else if (sv->nids() == TOK_RANGBRACK) {
	static int anglebrackets_symid =  symbol_add("<>");
	localtable()->find(vptr, anglebrackets_symid);
      }
      else if (sv->nids() == TOK_RANGBRACK2) {
	static int dblanglebrackets_symid =  symbol_add("<<>>");
	localtable()->find(vptr, dblanglebrackets_symid);
      }
      command_symid = sv->symbol_val();
    }

    /* handle case where symbol has arguments/keywords, but is not defined --
       or (posteval FuncObj) already resolves to one, at this point, whose
       :posteval flag is set.  Either way, route through the same NilFunc
       substitution: NilFunc is post_eval, so the pre-pass below marks this
       call's whole operand span pedepth'd and the forward push loop never
       eagerly evaluates it -- exactly the "sit in the postfix buffer until
       pulled" contract a posteval func's args need.  A symbol that's simply
       undefined still resolves to NilFunc's real "not found" behavior at
       dispatch time (comterp.c's dynamic gate, #328); one that resolves to a
       posteval FuncObj here is re-checked there too, so a later reassignment
       within the same statement chain is never trusted from this static
       snapshot -- only used to decide whether to defer at all. */
    else if ((sv->narg() || sv->nkey()) &&
	     (!vptr ||
	      (((ComValue*)vptr)->is_object(FuncObj::class_symid()) &&
	       ((FuncObj*)((ComValue*)vptr)->obj_val())->posteval()))) {
      static int nil_symid = symbol_add("nil");
      localtable()->find(vptr, nil_symid);
    }

    // convert to command if it has parens
    if (vptr && ((ComValue*)vptr)->type() == ComValue::CommandType && sv->nids() >= 0) {
      sv->obj_ref() = ((ComValue*)vptr)->obj_ref();
      sv->type(ComValue::CommandType);
      sv->command_symid(command_symid);
    } 
  } else if (sv->type() == ComValue::KeywordType) {
    sv->keynarg_ref() = token->narg;
  }
}

void ComTerp::push_stack(ComValue& value) {
    if (_stack_top+1 == _stack_siz) {
	int old_siz = _stack_siz;
	_stack_siz *= 2;
	dmm_realloc_size(sizeof(ComValue));
	if(dmm_realloc((void**)&_stack, (unsigned long)_stack_siz) != 0) {
	    KANRET("error in call to dmm_realloc");
	    return;
	}
	/* default-construct the grown region to clean UnknownType slots: dmm_realloc
	   leaves it raw, and the first *sv = ComValue(value) runs
	   assignval->unref_as_needed() on the destination, which crashes on garbage
	   that looks like a ref-counted _type.  see the note in
	   push_stack(postfix_token*). */
	for (int k = old_siz; k < _stack_siz; k++)
	    new (_stack + k) ComValue();
    }
    _stack_top++;

    if (_stack_top<0) {
      fprintf(stderr, "warning: comterp stack still empty after push\n");
      return;
    }

    ComValue* sv = _stack + _stack_top;
    *sv = ComValue(value);
    if (sv->type() == ComValue::KeywordType)
      sv->keynarg_ref() = value.keynarg_val();
    _just_reset = false;
}

void ComTerp::push_stack(AttributeValue& value) {
  ComValue comval(value);
  push_stack(comval);
}

void ComTerp::incr_stack() {
    _stack_top++;

    ComValue& sv = stack_top();

    /* See if this really is a command with a ComFunc */
    if (sv.type() == ComValue::SymbolType && sv.nids() >= 0) {
	void* vptr = nil;
	localtable()->find(vptr, sv.int_val());
	if (vptr && ((ComValue*)vptr)->type() == ComValue::CommandType) {
	    sv.obj_ref() = ((ComValue*)vptr)->obj_ref();
	    sv.type(ComValue::CommandType);
	}
    }

}

void ComTerp::incr_stack(int n) {
    for (int i=0; i<n; i++) 
        incr_stack();
}

void ComTerp::decr_stack(int n) {
    for (int i=0; i<n && _stack_top>=0; i++) {
        ComValue& stacktop = _stack[_stack_top--];
	stacktop.AttributeValue::~AttributeValue();
        #ifdef LEAKCHECK // destructor called where constructor never called
	AttributeValue::_leakchecker->create();
        #endif
    }
}

ComValue ComTerp::pop_stack(boolean lookupsym) {
  if (!stack_empty()) {
    ComValue& stacktop = _stack[_stack_top--];
    ComValue topval(stacktop);
    stacktop.AttributeValue::~AttributeValue();
    #ifdef LEAKCHECK  // destructor called where constructor never called
    AttributeValue::_leakchecker->create();
    #endif
    if (lookupsym && topval.is_symbol()) {
      return lookup_symval(topval);
    } else if (lookupsym && topval.is_attribute()) {
      ComValue attrval = *((Attribute*)topval.obj_val())->Value();
      topval.assignval(attrval);
      return topval;
    }else 
      return topval;

  } else {
    cerr << "stack empty, blank returned\n";
    return ComValue::blankval();
  }
}

boolean ComTerp::is_posteval_pending(int id) {
  if (!_alist) return false;
  AttributeValue* found = _alist->find(id);
  return found && found->is_object(FuncObjPendingArg::class_symid());
}

ComValue& ComTerp::fire_if_funcobj(ComValue& val) {
  if (!val.is_object(FuncObj::class_symid()))
    return val;
  ComValue funcval(val);  /* copy out before firing -- 'val' may be a
                              _stack[] reference, invalidated by any
                              dmm_realloc a push during firing triggers */
  fire_funcobj(funcval);
  /* heap-allocate this fire's own entry and Append() it -- unlike a
     contiguous array, this never relocates an entry already returned to
     an earlier caller in the same statement (see the _fire_scratch_pool
     comment in comterp.h). */
  ComValue* slot = new ComValue(pop_stack(false));
  _fire_scratch_pool->Append(slot);
  return *slot;
}

/* sliceoff()/slicelen() (comvalue.h) write straight through to _narg/_nkey
   -- fine on a StringType, which never needs them for anything else, but
   catastrophic on any other type, where those same fields carry a command
   call's own argument/keyword counts.  Restricted to StringType so a
   lookup_symval() carry-over (below) can't clobber that bookkeeping for
   every other symbol resolution in the interpreter. */
static void carry_slice(ComValue& dst, ComValue& src) {
  if (src.type() != ComValue::StringType) return;
  dst.sliced(src.sliced());
  dst.sliceoff(src.sliceoff());
  dst.slicelen(src.slicelen());
  dst.blocksz(src.blocksz());
}

ComValue& ComTerp::lookup_symval(ComValue& comval) {
    if (comval.bquote()) {
        return comval;
    }

    if (comval.type() == ComValue::SymbolType) {
        void* vptr = nil;

	if (_alist) {
	  int id = comval.symbol_val();
	  AttributeValue* aval = peek_alist_pending(_alist, id, _alist->find(id));
	  if (aval) {
	    /* coloned() can't be recovered here at all, and must not be
	       guessed at: _alist (a func's keyword-bound locals, fire_funcobj()
	       comterp.c) is populated via AttributeList::add_attr(int,
	       AttributeValue&) (attrlist.c), same as attrlist() itself, which
	       constructs a plain `new AttributeValue(value)` -- a strictly
	       smaller type with no _flags field, not a ComValue.  aval is
	       therefore genuinely only ever an AttributeValue* here, never a
	       ComValue* despite how it looks; ((ComValue*)aval)->coloned()
	       would read _flags out of memory past the real object's own
	       allocation -- undefined behavior, not a recovered flag (#438
	       tracks the actual fix: AttributeList would need to store
	       ComValue, not AttributeValue). */
	    ComValue newval(*aval);
	    *&comval = newval;
	    return comval;
	  }
	}

	/* assignval() takes an AttributeValue&, so it only ever copies the
	   base class's fields -- ComValue's own coloned() (comvalue.h)
	   isn't one of them, and comval keeps whatever it already had (the
	   identifier token's own, not the stored value's) unless carried
	   over explicitly here. */
	if (!comval.global_flag() && localtable()->find(vptr, comval.symbol_val()) ) {
	  comval.assignval(*(ComValue*)vptr);
	  comval.coloned(((ComValue*)vptr)->coloned());
	  carry_slice(comval, *(ComValue*)vptr);
	  return comval;
	} else if (globaltable()->find(vptr, comval.symbol_val())) {
	  comval.assignval(*(ComValue*)vptr);
	  comval.coloned(((ComValue*)vptr)->coloned());
	  carry_slice(comval, *(ComValue*)vptr);
	  return comval;
	} else
	  return ComValue::nullval();

    } else if (comval.is_object(Attribute::class_symid())) {
      /* coloned() can't be recovered here, and reading it via a ComValue*
	 cast on attrvalp would be undefined behavior, not a recovered flag --
	 same reasoning as the _alist branch above.  Attribute::Value()
	 returns whatever AttributeList::add_attr() (attrlist.c) stored, which
	 is always a plain `new AttributeValue(value)`, never a ComValue --
	 there's no _flags field to read past the end of.  #438 tracks the
	 actual fix (AttributeList would need to store ComValue, not
	 AttributeValue). */
      ComValue attrval = *((Attribute*)comval.obj_val())->Value();
      comval.assignval(attrval);
    }
    return comval;
}

AttributeValue* ComTerp::lookup_symval(ComValue* comval, boolean freeze) {
    if (comval->bquote()) return nil;

    if (comval->type() == ComValue::SymbolType) {
        void* vptr = nil;

	/* search order: func scope (_alist) -> local -> global.
	   _alist must be checked first so that a func-local variable
	   shadows a same-named variable in localtable() (outer scope).
	   Without this, ++ inside a func body finds and mutates the outer
	   variable instead of the func-local one, causing infinite loops.
	   (local()/global() lvalue symbols never reach these branches:
	   their bquote flag returns nil above, and AssignFunc routes them
	   by their scope flags directly.) */
	if (!comval->global_flag() && _alist) {
	  int id = comval->symbol_val();
	  AttributeValue* found = _alist->find(id);
	  AttributeValue* aval = freeze
	    ? pull_alist_pending(_alist, id, found)
	    : peek_alist_pending(_alist, id, found);
	  if (aval) return aval;
	}
	if (!comval->global_flag() && localtable()->find(vptr, comval->symbol_val())) {
	  return (AttributeValue*)vptr;
	} else if (globaltable()->find(vptr, comval->symbol_val())) {
	  return (AttributeValue*)vptr;
	} else
	  return nil;

    } else if (comval->is_object(Attribute::class_symid())) {

      return ((Attribute*)comval->obj_val())->Value();

    }       
    return nil;
}

ComValue& ComTerp::lookup_symval(int symid) {
  void* vptr = nil;
  if (localtable()->find(vptr, symid)) {
    ComValue* valptr = (ComValue*)vptr;
    return *valptr;
  } else 
    return ComValue::nullval();
}

ComValue& ComTerp::stack_top(int n) {
  if (_stack_top+n < 0 || _stack_top+n >= _stack_siz) {
    return ComValue::blankval();    
  }
  else
    return _stack[_stack_top+n];
}

ComValue& ComTerp::pop_symbol() {
    ComValue& stacktop = _stack[_stack_top--];
    if (stacktop.type() == ComValue::SymbolType)
        return stacktop;
    else
        return ComValue::nullval();
}

void ComTerp::clear_top_commands() {
    delete _top_commands;
    _top_commands = nil;
}

int ComTerp::add_command(const char* name, ComFunc* func, const char* alias, const char* docstring2,
                          boolean hidden) {
    int symid = symbol_add((char *)name);

    if(docstring2) func->docstring2(docstring2);
    if (hidden) func->hidden(true);

    if (!hidden) {
      if (!_top_commands)
          _top_commands = new AttributeValueList();
      _top_commands->Append(new AttributeValue(symid, AttributeValue::SymbolType));
    }

    func->funcid(symid);
    ComValue* comval = new ComValue();
    comval->type(ComValue::CommandType);
    comval->obj_ref() = (void*)func;
    comval->command_symid(symid);
    localtable()->insert(symid, comval);
    if (alias) {
      int alias_symid = symbol_add((char *)alias);
      ComValue* aliasval = new ComValue();
      aliasval->type(ComValue::CommandType);
      aliasval->obj_ref() = (void*)func;
      aliasval->command_symid(alias_symid, true /* alias */);
      localtable()->insert(symid, aliasval);
    }
    return symid;
}

ComTerp& ComTerp::instance() {
    if (!_instance) 
	ComTerp* comterp = new ComTerp();
    return *_instance;
}

void ComTerp::quit(boolean quitflag) {
    _quitflag = quitflag;
}

void ComTerp::exit(int status) {
  /* Use _exit(), not exit(): exiting from inside a reactor callback must not run
     atexit handlers / C++ static destructors over a still-live interpreter (the
     same use-after-free class fixed elsewhere in this layer).  But _exit() also
     skips the stdio flush, so a final print() before exit could be lost from a
     block-buffered stdout (e.g. when piped) -- flush it (and stderr) first.
     Deliberately NOT fflush(NULL): a server interpreter can hold FILE* streams
     wrapping live client sockets, and flushing one whose peer has stalled could
     block the exit -- stdout/stderr are all a final print() needs.

     tty_echo_restore() (ComUtil/ttyecho.c, issue #76) is registered via
     atexit() when stdin echo was disabled, but atexit handlers are exactly
     what _exit() skips -- call it explicitly here too, so a scripted
     exit()/quit() doesn't leave the user's terminal echo off after this
     process is gone.  Safe unlike arbitrary atexit handlers: it only
     touches tty state, never the interpreter itself. */
  fflush(stdout);
  fflush(stderr);
  tty_echo_restore();
  _exit( status );
}

boolean ComTerp::quitflag() {
    return _quitflag;
}

void ComTerp::quitflag(boolean flag) {
    _quitflag = flag;
}

ComValue ComTerp::orphan_stream_count(ComValue& streamv) {
  ComValue sv(streamv);
  int cnt = 0;
  boolean done = false;
  while (!done) {
    NextFunc::execute_impl(this, sv);
    ComValue popval(pop_stack());
    if (popval.is_unknown() || StrmFunc::is_delimiter(popval))
      done = true;
    else
      cnt++;
  }
  return ComValue(cnt, ComValue::IntType);
}

int ComTerp::run(boolean one_expr, boolean nested) {
  int old_runflag = running();
  running(true);

  int status = 1;
  _errbuf[0] = '\0';
  char errbuf_save[BUFSIZ];
  errbuf_save[0] = '\0';

#ifdef USE_FDSTREAMS  
  FILEBUF(fbuf, handler() && handler()->wrfptr() ? handler()->wrfptr() : (_fd>0 ? fdopen(_fd, "w") : stdout), ios_base::out);
  ostream out(&fbuf);
#else
  FILE *fp = handler() && handler()->wrfptr() ? handler()->wrfptr() : (_fd>0 ? fdopen(_fd, "w") : stdout);
#endif
  boolean eolflag = false;
  boolean errorflag = false;

  while (!eof() && !quitflag() && !eolflag) {
    
    if (read_expr()) {
      status = 0;
      int top_before = _stack_top;
      eval_expr(nested);

      if (returnflag()) {
        returnflag(false);  // return() at prompt: clear and ignore
        this->err_str(_errbuf, BUFSIZ, "comterp");  // clear any error state
        _errbuf[0] = '\0';
        if (one_expr) break;
        continue;
      }

      if (top_before == _stack_top)
	status = 2;
      this->err_str( _errbuf, BUFSIZ, "comterp" );
      errno = 0;
      if (strlen(_errbuf)==0) {
	if (quitflag()) {
	  status = -1;
	  break;
	} else if (!func_for_next_expr() && val_for_next_func().is_null() && muted()!=1) {
	  if (stack_top().is_stream() && autostream()) {
	    ComValue streamv(stack_top());
	    do {
	      pop_stack();
	      NextFunc::execute_impl(this, streamv);
	      if (stack_top().is_known()) {
		#ifdef USE_FDSTREAMS
		print_stack_top(out);
		out << "\n";
		out.flush();
		#else
		std::streambuf* strmbuf = new std::strstreambuf();
		ostream out(strmbuf);
		print_stack_top(out);
		out << "\n";
		out << '\0';
		const char *str = ((std::strstreambuf*)strmbuf)->str();
		fprintf(fp, "%s", str);
		#endif
	      }
	    } while (stack_top().is_known());
	  } else if (stack_top().is_stream() && stack_top().stream_list() &&
		     stack_top().stream_list()->refcount_==1) {
	    /* An orphaned stream -- the final result of a stand-alone
	       expression, never assigned to anything, never streamed
	       further -- would otherwise print as an uninformative "[]"
	       (whatever's left of it once it falls out of scope
	       unconsumed).  Drain it instead (orphan_stream_count(), same
	       traversal EachFunc uses) and show the count: strictly more
	       informative, and the stream was headed for the same fate
	       either way.  Gated on refcount_==1 (nothing else holds a
	       reference to the underlying AttributeValueList*, confirmed
	       live: an orphaned `$$(1,2,3)` sits at 1, `x=$$(1,2,3)` sits
	       at 2 -- one for the stack, one for x's binding) so this can
	       never drain a stream a variable still needs: x=$$(1,2,3)
	       at an interactive prompt must leave x fully intact for a
	       later next(x), not silently exhaust it while "just printing
	       the result". */
	    ComValue streamv(stack_top());
	    pop_stack();
	    ComValue countv(orphan_stream_count(streamv));
	    push_stack(countv);
	    /* echo as [n]: a count of what went by, not a value.  Stamped on
	       the pushed slot because the wrapper never survives a copy. */
	    stack_top().wrapper(AttributeValue::BracketWrapper);
	    #ifdef USE_FDSTREAMS
	    print_stack_top(out);
	    out << "\n";
	    out.flush();
	    #else
	    std::streambuf* strmbuf = new std::strstreambuf();
	    ostream out(strmbuf);
	    print_stack_top(out);
	    out << "\n";
	    out << '\0';
	    const char *str = ((std::strstreambuf*)strmbuf)->str();
	    fprintf(fp, "%s", str);
	    fflush(fp);
	    #endif
	  } else {
	    #ifdef USE_FDSTREAMS
	    print_stack_top(out);
	    out << "\n";
	    out.flush();
	    #else
	    std::streambuf* strmbuf = new std::strstreambuf();
	    ostream out(strmbuf);
	    print_stack_top(out);
	    out << "\n";
	    out << '\0';
	    const char *str = ((std::strstreambuf*)strmbuf)->str();
	    fprintf(fp, "%s", str);
	    fflush(fp);
	    #endif
	  }
	}
      } else {
	#ifdef USE_FDSTREAMS
	out << _errbuf << "\n";
	out.flush();
	#else
	std::streambuf* strmbuf = new std::strstreambuf();
	ostream out(strmbuf);
	out << _errbuf << "\n";
	out << '\0';
	const char *str = ((std::strstreambuf*)strmbuf)->str();
	fprintf(fp, "%s", str);
        fflush(fp);
	#endif
	strcpy(errbuf_save, _errbuf);
	_errbuf[0] = '\0';
      }
    } else {
      this->err_str( _errbuf, BUFSIZ, "comterp" );
      if (strlen(_errbuf)>0) {
	errorflag = true;
	#ifdef USE_FDSTREAMS
	out << _errbuf << "\n";
	out.flush();
	#else
	std::streambuf* strmbuf = new std::strstreambuf();
	ostream out(strmbuf);
	out << _errbuf << "\n";
	out << '\0';
	const char *str = ((std::strstreambuf*)strmbuf)->str();
	fprintf(fp, "%s", str);
        fflush(fp);
	#endif
	strcpy(errbuf_save, _errbuf);
	_errbuf[0] = '\0';
      } else {
	eolflag = true;
        if (errbuf_save[0]) strcpy(_errbuf, errbuf_save);
      }
    }
    if (!nested)
      decr_stack(_stack_top+1);
    if (one_expr) break;
  }
  if (status==1 && _pfnum==0) status=2;
  if (status==1 && !errorflag) status=3;
  #if 0 // has to be dealt with a different way
  if (nested && status!=2) pop_stack();
  #endif
  if (errno == EPIPE) {
    status = -1;
    fprintf(stderr, "broken pipe detected: comterp quit\n");
  }
  running(old_runflag);
  return status;
}

void ComTerp::add_defaults() {
  set_command_prompt("(comt) ");
  if (!_defaults_added) {
    _defaults_added = true;

    add_command("nil", new NilFunc(this));
    add_command("blank", new BlankFunc(this));
    add_command("char", new CharFunc(this));
    add_command("short", new ShortFunc(this));
    add_command("int", new IntFunc(this));
    add_command("long", new LongFunc(this));
    add_command("float", new FloatFunc(this));
    add_command("double", new DoubleFunc(this));

    add_command("add", new AddFunc(this));
    add_command("sub", new SubFunc(this));
    add_command("minus", new MinusFunc(this));
    add_command("mpy", new MpyFunc(this));
    add_command("div", new DivFunc(this));
    add_command("mod", new ModFunc(this), NULL, "mod (%) is the mod operator");
    add_command("min", new MinFunc(this));
    add_command("max", new MaxFunc(this));
    add_command("abs", new AbsFunc(this));

    add_command("assign", new AssignFunc(this));
    add_command("mod_assign", new ModAssignFunc(this), NULL, "mod_assign (%=) is the mod assign operator");
    
    add_command("mpy_assign", new MpyAssignFunc(this));
    add_command("add_assign", new AddAssignFunc(this));
    add_command("sub_assign", new SubAssignFunc(this));
    add_command("div_assign", new DivAssignFunc(this));
    add_command("incr", new IncrFunc(this));
    add_command("incr_after", new IncrAfterFunc(this));
    add_command("decr", new DecrFunc(this));
    add_command("decr_after", new DecrAfterFunc(this));

    add_command("bit_and", new BitAndFunc(this));
    add_command("bit_xor", new BitXorFunc(this));
    add_command("bit_or", new BitOrFunc(this));
    add_command("bit_not", new BitNotFunc(this));
    add_command("nand", new BitNandFunc(this));
    add_command("xnor", new BitXnorFunc(this));
    add_command("nor", new BitNorFunc(this));
    add_command("lshift", new LeftShiftFunc(this));
    add_command("rshift", new RightShiftFunc(this));
    add_command("and", new AndFunc(this));
    add_command("or", new OrFunc(this));
    add_command("negate", new NegFunc(this));
    add_command("eq", new EqualFunc(this));
    add_command("not_eq", new NotEqualFunc(this));
    add_command("gt", new GreaterThanFunc(this));
    add_command("gt_or_eq", new GreaterThanOrEqualFunc(this));
    add_command("lt", new LessThanFunc(this));
    add_command("lt_or_eq", new LessThanOrEqualFunc(this));
    add_command("true", new TrueFunc(this));
    add_command("false", new FalseFunc(this));

    add_command("stream", new StreamFunc(this));
    add_command("spread", new SpreadFunc(this));
    add_command("echo", new EchoFunc(this));
    add_command("concat", new ConcatFunc(this));
    add_command("repeat", new RepeatFunc(this));
    add_command("replay", new ReplayFunc(this));
    add_command("iterate", new IterateFunc(this));
    add_command("next", new NextFunc(this));
    add_command("info", new InfoFunc(this));
    add_command("each", new EachFunc(this));
    add_command("filter", new FilterFunc(this));
    add_command("feed", new FeedFunc(this));
    add_command("chunk", new ChunkFunc(this));

    add_command("dot", new DotFunc(this));
    add_command("attrname", new DotNameFunc(this));
    add_command("attrval", new DotValFunc(this));

    add_command("list", new ListFunc(this));
    add_command("attrlist", new AttrListFunc(this));
    add_command("at", new ListAtFunc(this));
    add_command("size", new ListSizeFunc(this));
    add_command("tuple", new TupleFunc(this));
    add_command("colonlist", new ColonListFunc(this));
    add_command("index", new ListIndexFunc(this));

    add_command("sum", new SumFunc(this));
    add_command("mean", new MeanFunc(this));
    add_command("var", new VarFunc(this));
    add_command("stddev", new StdDevFunc(this));

    add_command("rand", new RandFunc(this));
    add_command("srand", new SRandFunc(this));

    add_command("exp", new ExpFunc(this));
    add_command("log", new LogFunc(this));
    add_command("log10", new Log10Func(this));
    add_command("log2", new Log2Func(this));
    add_command("pow", new PowFunc(this));

    add_command("acos", new ACosFunc(this));
    add_command("asin", new ASinFunc(this));
    add_command("atan", new ATanFunc(this));
    add_command("atan2", new ATan2Func(this));
    add_command("cos", new CosFunc(this));
    add_command("sin", new SinFunc(this));
    add_command("tan", new TanFunc(this));
    add_command("sqrt", new SqrtFunc(this));
    add_command("pi", new PiFunc(this));
    add_command("radtodeg", new RadToDegFunc(this));
    add_command("degtorad", new DegToRadFunc(this));

    add_command("floor", new FloorFunc(this));
    add_command("ceil", new CeilFunc(this));
    add_command("round", new RoundFunc(this));

    add_command("xform", new XformFunc(this));
    add_command("invert", new InvertXformFunc(this));
    add_command("xpose", new XposeFunc(this));

    add_command("cond", new CondFunc(this));
    add_command("seq", new SeqFunc(this));
    add_command("run", new RunFunc(this));

    add_command("help", new HelpFunc(this));
    add_command("optable", new OptableFunc(this));
    add_command("trace", new ComterpTraceFunc(this));
    add_command("errmsg", new ErrMsgFunc(this));
    add_command("pause", new ComterpPauseFunc(this));
    add_command("step", new ComterpStepFunc(this));
    add_command("stackheight", new ComterpStackHeightFunc(this));
    // add_command("mallinfo", new ComterpMallInfoFunc(this));
    add_command("symid", new SymIdFunc(this));
    add_command("symval", new SymValFunc(this));
    add_command("symbol", new SymbolFunc(this));
    add_command("symadd", new SymAddFunc(this));
    add_command("symvar", new SymVarFunc(this));
    add_command("symstr", new SymStrFunc(this));
    add_command("strref", new StrRefFunc(this));
    add_command("string", new StringFunc(this));
    add_command("strcap", new StrCapFunc(this));
    add_command("global", new GlobalSymbolFunc(this));
    add_command("local", new LocalSymbolFunc(this));
    add_command("split", new SplitStrFunc(this));
    add_command("join", new JoinStrFunc(this));
    add_command("substr", new SubStrFunc(this));

    add_command("type", new TypeSymbolFunc(this));
    add_command("class", new ClassSymbolFunc(this));
    add_command("istype", new IsTypeFunc(this));
    add_command("isclass", new IsClassFunc(this));
    add_command("iscomm", new IsCommFunc(this));
    add_command("isfunc", new IsFuncFunc(this));

    add_command("bquote", new BackQuoteFunc(this));

    add_command("postfix", new PostFixFunc(this));
    add_command("posteval", new PostEvalFunc(this));
    add_command("parse", new ParseFunc(this));

    add_command("if", new IfThenElseFunc(this));
    add_command("for", new ForFunc(this));
    add_command("while", new WhileFunc(this));
    add_command("switch", new SwitchFunc(this));

    add_command("open", new OpenFileFunc(this));
    add_command("close", new CloseFileFunc(this));
    add_command("print", new PrintFunc(this));
    add_command("gets", new GetStringFunc(this));

    add_command("usleep", new USleepFunc(this));
    add_command("update", new UpdateFunc(this));
    add_command("timeexpr", new TimeExprFunc(this));
    add_command("time", new TimeFunc(this));

    add_command("eval", new EvalFunc(this));
    add_command("shell", new ShellFunc(this));
    add_command("patchkey", new PatchKeyFunc(this));
    add_command("quit", new QuitFunc(this));
    add_command("exit", new ExitFunc(this));
    add_command("mute", new MuteFunc(this));
    add_command("empty", new EmptyFunc(this));

    add_command("ctoi", new CtoiFunc(this));
    add_command("isspace", new IsSpaceFunc(this));
    add_command("isdigit", new IsDigitFunc(this));
    add_command("isalpha", new IsAlphaFunc(this));

    add_command("arg", new GetArgFunc(this));
    add_command("narg", new NumArgFunc(this));

    add_command("continue", new ContinueFunc(this));
    add_command("break", new BreakFunc(this));

    add_command("func", new FuncObjFunc(this));
    
    add_command("date", new DateFunc(this));

    add_command("beep", new BeepFunc(this));
    add_command("ding", new DingFunc(this));

    add_command("return", new ReturnFunc(this));

  }
}

void ComTerp::set_attributes(AttributeList* alist) { 
    Unref(_alist);
    _alist = alist; 
    Resource::ref(_alist);
}

AttributeList* ComTerp::get_attributes() { return _alist;}


int ComTerp::runfile(const char* filename, boolean popen_flag) {
    int old_runflag = running();
    running(true);

    /* save tokens to restore after the file has run */
    int toklen;
    postfix_token* tokbuf = copy_postfix_tokens(toklen);
    int tokoff = _pfoff;

    /* swap in input pointer and function */
    push_servstate();
    FILE* fptr = popen_flag ? popen(filename, "r") : fopen(filename, "r");
    _inptr = fptr;
    _outfunc = nil;
    if (!fptr) cerr << "unable to run from file " << filename << "\n";
    

    ComValue* retval = nil;
    int status = 0;
    while( fptr && !feof(fptr)) {
	if (read_expr()) {
	    /* Drain a leftover orphaned stream from the PREVIOUS statement
	       now that read_expr() confirms a genuine next statement
	       exists (checking any earlier, e.g. unconditionally at the
	       top of the loop, would incorrectly drain the truly LAST
	       statement's retval too, on whatever trailing pass finds
	       nothing left to read and the while() condition was
	       nonetheless still true for). Before this iteration's own
	       eval_expr() runs, not after: draining can have visible side
	       effects of its own (e.g. a print() overdrive stream defers
	       each repetition's actual print() call until that element is
	       pulled -- draining fires all of them at once), and checking
	       this any later (e.g. the "save last thing on stack" spot
	       below, which used to have this check) would run it AFTER
	       the next statement's own eval_expr() already produced its
	       output, interleaving the two out of script order -- see the
	       identical fix and full explanation in ComTerpServ::runfile(),
	       comterpserv.c, the override actually exercised by
	       `comterp run <file>`. */
	    if (retval && retval->is_stream() && retval->stream_list() &&
	        retval->stream_list()->refcount_==1) {
	      orphan_stream_count(*retval);
	      delete retval;
	      retval = nil;
	    }
	    if (eval_expr(true)) {
	        this->err_print( stderr, "comterp" );
		FILEBUF(obuf, stdout, ios_base::out);
		ostream ostr(&obuf);
		ostr << "err\n";
		ostr.flush();
		status = -1;
	    } else if (quitflag()) {
	        status = 1;
	        break;
	    } else if (returnflag()) {
	        retval = new ComValue(pop_stack());
	        break;
	    } else {
	        /* save last thing on stack */
	        retval = new ComValue(pop_stack());
	    }
	}
    }
    if (popen_flag)
        pclose(fptr);
    else
        fclose(fptr);

    returnflag(false);

    pop_servstate();

    load_postfix(tokbuf, toklen, tokoff);
    delete tokbuf;

    if (retval) {
        push_stack(*retval);
	delete retval;
    } else
        push_stack(ComValue::nullval());

    running(old_runflag);
    return status;
}

ComterpHandler* ComTerp::handler() {
    return _handler;
}

void ComTerp::handler(ComterpHandler* handler) {
    _handler = handler;
}


void ComTerp::load_postfix(postfix_token* tokens, int toklen, int tokoff) {
    if (toklen>_pfsiz) {
       _pfsiz *= 2; 
       dmm_realloc_size(sizeof(postfix_token));
       if( dmm_realloc( (void **)&_pfbuf, (long)_pfsiz )) {
         cerr << "error in reallocing pfbuf in Parser::load_postfix_tokens";
         return;
	 }
      }
    for (int i=0; i<toklen; i++)
        _pfbuf[i] = tokens[i];
    _pfnum = toklen;
    _pfoff = tokoff;
}

void ComTerp::list_commands(ostream& out, boolean sorted) {
  int nfuncs = 0;
  int* funcids = get_commands(nfuncs, sorted);
  if (nfuncs) {
    int rowcnt = 0;
    for (int i=0; i<nfuncs; i++) {
      const char* command_name = symbol_pntr(funcids[i]);
      out << command_name;
      int slen = strlen(command_name);
      int tlen = 8-((slen+1)%8);
      rowcnt += slen + tlen;
      if (rowcnt>=64) {
	rowcnt = 0;
	out << "\n";
      } else
#if 0   
	out << "\t";
#else
      for(int t=0; t<=tlen; t++) out << ' ';
#endif
    }
    delete funcids;
  }
}

/* double the buffer when the next write would not fit */
static int* grow_if_full(int* buffer, int& bufsiz, int ncomm) {
  if (ncomm < bufsiz) return buffer;
  int* newbuf = new int[bufsiz*2];
  for (int j=0; j<ncomm; j++)
    newbuf[j] = buffer[j];
  bufsiz *= 2;
  delete [] buffer;
  return newbuf;
}

int* ComTerp::get_commands(int& ncomm, boolean sort) {
  TableIterator(ComValueTable) i(*localtable());
  int bufsiz = 256;
  int* buffer = new int[bufsiz];
  ncomm = 0;
  int opercnt = 0;
  while (i.more()) {
    int key = i.cur_key();
    ComValue* value = (ComValue*)i.cur_value();
    if (value->is_type(AttributeValue::CommandType)) {
      ComFunc* commfunc = (ComFunc*)value->obj_val();
      if (commfunc && commfunc->hidden()) {
        i.next();
        continue;
      }
      const char* command_name = symbol_pntr(key);
      int opid = opr_tbl_opstr(key);
      const char* operator_name = symbol_pntr(opr_tbl_operid(opid));
      /* An operator-bearing command writes twice per iteration, so capacity
         has to be checked before each write, not once between them. */
      if (operator_name) {
        buffer = grow_if_full(buffer, bufsiz, ncomm);
        buffer[ncomm++] = key;
	key = opr_tbl_operid(opid);
	opercnt++;
      }
      buffer = grow_if_full(buffer, bufsiz, ncomm);
      buffer[ncomm++] = key;
    }
    i.next();
  }
  
  if (sort) {
    int* sortedbuffer = new int[ncomm];
    int i = 0;  /* operators first */
    int j;
    for (j=0; j< ncomm; j++) sortedbuffer[j] = -1;
    for (j=0; j< ncomm; j++) {
      const char* str = symbol_pntr(buffer[j]);
      if (!isalpha(*str) && 
	  strcmp(str,"()")!=0 && strcmp(str,"[]")!=0 && strcmp(str,"{}")!=0 && 
	  strcmp(str,"<>")!=0 && strcmp(str,"<<>>")!=0 && 
          *str!='\'')  // for ipl ISA support
	  sortedbuffer[i++] = buffer[j];
    }
    if (i != opercnt) cerr << "bad number of operators\n";
      
    for (j=0; j<ncomm; j++) {
      if (!isalpha(*symbol_pntr(buffer[j]))) continue;

      /* count the number of strings greater than this one */
      int count = opercnt;
      for (int k=0; k<ncomm; k++) {
	if (!isalpha(*symbol_pntr(buffer[k]))) continue;
	count += (strcmp(symbol_pntr(buffer[j]), symbol_pntr(buffer[k])) > 0);
      }
      sortedbuffer[count] = buffer[j];
    }
    delete [] buffer;

    /* one more pass over the sorted buffer to remove duplicates */
    int copydist = 0;
    for (j=0; j<ncomm; j++) {
      if (sortedbuffer[j]<0) 
	copydist++;
      else 
	sortedbuffer[j-copydist] = sortedbuffer[j];
    }
    ncomm -= copydist;
    return sortedbuffer;
  } else
    return buffer;
}

ComValue* ComTerp::localvalue(int symid) {
  ComValueTable* table = localtable();
  if (table) {
    void* vptr = nil;
    table->find(vptr, symid);
    return (ComValue*)vptr;
  } else 
    return &ComValue::unkval();
}

ComValue* ComTerp::globalvalue(int symid) {
  ComValueTable* table = globaltable();
  if (table) {
    void* vptr = nil;
    table->find(vptr, symid);
    return (ComValue*)vptr;
  } else 
    return &ComValue::unkval();
}

void ComTerp::disable_prompt() { set_continuation_prompt_disabled(1); }
void ComTerp::enable_prompt() { set_continuation_prompt_disabled(0); }

ComFuncState* ComTerp::top_funcstate() {
  return _fsstack_top < 0 ? nil : _fsstack+_fsstack_top;
}

void ComTerp::pop_funcstate() {
  if (_fsstack_top >=0) _fsstack_top--;
}

void ComTerp::push_funcstate(ComFuncState& funcstate) {
  if (_fsstack_top+1 == _fsstack_siz) {
    _fsstack_siz *= 2;
    dmm_realloc_size(sizeof(ComFuncState));
    if(dmm_realloc((void**)&_fsstack, (unsigned long)_fsstack_siz) != 0) {
      KANRET("error in call to dmm_realloc");
      return;
    }
  } 
  _fsstack_top++;
  ComFuncState* sfs = _fsstack + _fsstack_top;
  *sfs = ComFuncState(funcstate);
}

void ComTerp::func_for_next_expr(ComFunc* func) {
  if (!_func_for_next_expr)
    _func_for_next_expr = func;
}

ComFunc* ComTerp::func_for_next_expr() {
  return _func_for_next_expr;
}

void ComTerp::val_for_next_func(ComValue& val) {
  if (_val_for_next_func) {
    delete _val_for_next_func;
  }
  _val_for_next_func = new ComValue(val);
}

ComValue& ComTerp::val_for_next_func() {
  if (_val_for_next_func) {
    return *_val_for_next_func;
  } else
    return ComValue::nullval();
}

void ComTerp::clr_val_for_next_func() {
  delete _val_for_next_func;
  _val_for_next_func = nil;
}

ComTerpState* ComTerp::top_servstate() {
  return _ctsstack_top < 0 ? nil : _ctsstack+_ctsstack_top;
}

void ComTerp::pop_servstate() {
  if (_ctsstack_top >=0) {

    ComTerpState* cts_state = top_servstate();

    /* clean up */
    delete _buffer;
    delete _pfbuf;
    delete [] _pfcomvals;

    /* restore copies of everything */
    _pfbuf = cts_state->pfbuf();
    _pfsiz = cts_state->pfsiz();
    _pfnum = cts_state->pfnum();
    _pfoff = cts_state->pfoff();
    _bufptr = cts_state->bufptr();
    _linenum = cts_state->linenum();
    //    _just_reset = cts_state->just_reset();
    _buffer = cts_state->buffer();
    _pfcomvals = cts_state->pfcomvals();
    _infunc = cts_state->infunc();
    _eoffunc = cts_state->eoffunc();
    _errfunc = cts_state->errfunc();
    _inptr = cts_state->inptr();
    _alist = cts_state->alist();
    
    _ctsstack_top--;
  }
}

void ComTerp::push_servstate() {
  ComTerpState cts_state;

  /* save copies of everything */
  cts_state.pfbuf() = _pfbuf;
  cts_state.pfsiz() = _pfsiz;
  cts_state.pfnum() = _pfnum;
  cts_state.pfoff() = _pfoff;
  cts_state.bufptr() = _bufptr;
  cts_state.linenum() = _linenum;
  //  cts_state.just_reset() = _just_reset;
  cts_state.buffer() = _buffer;
  cts_state.pfcomvals() = _pfcomvals;
  cts_state.infunc() = _infunc;
  cts_state.eoffunc() = _eoffunc;
  cts_state.errfunc() = _errfunc;
  cts_state.inptr() = _inptr;
  cts_state.alist() = _alist;

  /* re-initialize */
  if(dmm_calloc((void**)&_pfbuf, _pfsiz, sizeof(postfix_token)) != 0) 
    KANRET("error in call to dmm_calloc");
  _pfnum = _pfoff = 0;
  _buffer = new char[_bufsiz];
  _bufptr = 0;
  _linenum = 0;
  // _just_reset = false;
  _pfcomvals = nil;
  
  if (_ctsstack_top+1 == _ctsstack_siz) {
    _ctsstack_siz *= 2;
    dmm_realloc_size(sizeof(ComTerpState));
    if(dmm_realloc((void**)&_ctsstack, (unsigned long)_ctsstack_siz) != 0) {
      KANRET("error in call to dmm_realloc");
      return;
    }
  } 
  _ctsstack_top++;
  ComTerpState* ctss = _ctsstack + _ctsstack_top;
  *ctss = cts_state;
}

boolean ComTerp::stack_empty() { return _stack_top<0; }

void ComTerp::postfix_echo() {
  if (!_echo_postfix) return;
  postfix_echo(_pfbuf, _pfnum);
}

void ComTerp::postfix_echo(postfix_token* pfbuf, int pfnum) {
  // print everything in the pfbuf for this function
  FILEBUF(fbuf,handler() && handler()->wrfptr() ? handler()->wrfptr() : stdout, ios_base::out);
  ostream out(&fbuf);
 
  boolean oldbrief = brief();
  brief(true);

  ComValue val;
  for (int i=0; i<pfnum; i++) {
    ComValue val;
    token_to_comvalue(pfbuf+i, &val);
    val.comterp(this);
    out << val;
    if (val.is_type(AttributeValue::CommandType) ||
       (_detail_matched_delims && val.is_type(AttributeValue::SymbolType) && 
	val.nids() >= TOK_RPAREN )) {
      if (!_detail_matched_delims) {
	out << "[" << val.narg() << "|" << val.nkey() << "]";
	ComFunc* func = (ComFunc*)val.obj_val();
	if (func->post_eval()) out << "*";
      } else {
	char ldelim, rdelim;
	boolean dbldelim = 0;
	if (val.nids()==TOK_RPAREN) {ldelim = '('; rdelim = ')'; }
	else if (val.nids()==TOK_RBRACKET) {ldelim = '['; rdelim = ']'; }
	else if (val.nids()==TOK_RBRACE) {ldelim = '{'; rdelim = '}'; }
	else if (val.nids()==TOK_RANGBRACK) {ldelim = '<'; rdelim = '>'; }
	else if (val.nids()==TOK_RANGBRACK2) {ldelim = '<'; rdelim = '>'; dbldelim=1;}
	else {ldelim = ':'; rdelim = 0x0;};
	out << ldelim;
	if (dbldelim) out << ldelim;
	out << val.narg();
	if (rdelim) {
	  out << rdelim;
	  if (dbldelim) out << rdelim;
	}
      }
    }
    else if (val.is_type(AttributeValue::SymbolType) && 
	     (val.narg() || val.nkey()))
      out << "{" << val.narg() << "|" << val.nkey() << "}";
    else if (val.is_type(AttributeValue::KeywordType))
      out << "(" << val.keynarg_val() << ")";
    out << ((i==pfnum-1) ? "\n" : " ");
  }
  brief(oldbrief);
  out.flush();
}

int ComTerp::arg_str(int n) {
  if (n<0 || n>_narg_strs) return -1;
  return _arg_strs ? _arg_strs[n] : nil;
}

int ComTerp::narg_str() {
  return _narg_strs;
}

ComValue ComTerp::funcobj_arg(int n) {
  if (!_funcobj_argvals || n<0 || n>=_funcobj_nargs)
    return ComValue::nullval();
  if (_funcobj_argvals[n].is_object(FuncObjPendingArg::class_symid())) {
    FuncObjPendingArg* marker = (FuncObjPendingArg*)_funcobj_argvals[n].obj_val();
    ComValue pulled(pull_funcobj_pending(marker));
    if (pulled.is_stream()) {
      /* pin it, don't keep re-firing -- a stream is a stateful, single-
	 pass cursor, not a value; re-running the caller's constructing
	 expression on every read doesn't give a "fresh draw" the way it
	 does for a scalar (see the :posteval LANGUAGE.md caveat), it hands
	 back an eternally-unexhausted stream that silently discards
	 whatever progress a prior read already made -- a while loop
	 pulling from it can never see it end.  Same write-back-and-delete
	 treatment an explicit write already gives a keyword marker (see
	 pull_alist_pending): every later arg(n) read here finds the real,
	 same-identity stream object directly, no second pull. */
      _funcobj_argvals[n] = pulled;
      delete marker;
      return _funcobj_argvals[n];
    }
    return pulled;  /* re-fires every call, never memoized, for anything else */
  }
  return _funcobj_argvals[n];
}

ComValue ComTerp::pull_funcobj_pending(FuncObjPendingArg* marker) {
  /* reach back into the caller's still-parked buffer -- push_servstate()
     already stashed it on _ctsstack the moment this invocation's body
     started running, purely as ordinary nested-call bookkeeping, so there's
     nothing new to allocate here: borrow its pfbuf/pfcomvals/pfoff for the
     one post_eval_expr() call, then put this invocation's own buffer back. */
  ComTerpState* caller = top_servstate();
  if (!caller) return ComValue::nullval();

  postfix_token* save_pfbuf = _pfbuf;
  int save_pfsiz = _pfsiz;
  int save_pfnum = _pfnum;
  int save_pfoff = _pfoff;
  ComValue* save_pfcomvals = _pfcomvals;

  _pfbuf = caller->pfbuf();
  _pfsiz = caller->pfsiz();
  _pfnum = caller->pfnum();
  _pfoff = caller->pfoff();
  _pfcomvals = caller->pfcomvals();

  post_eval_expr(marker->tokcnt(), marker->offtop(), marker->pedepth());
  ComValue val(pop_stack());

  _pfbuf = save_pfbuf;
  _pfsiz = save_pfsiz;
  _pfnum = save_pfnum;
  _pfoff = save_pfoff;
  _pfcomvals = save_pfcomvals;

  return val;
}

AttributeValue* ComTerp::pull_alist_pending(AttributeList* al, int id, AttributeValue* found) {
  if (!found || !found->is_object(FuncObjPendingArg::class_symid()))
    return found;
  FuncObjPendingArg* marker = (FuncObjPendingArg*)found->obj_val();
  ComValue pulled(pull_funcobj_pending(marker));
  al->add_attr(id, pulled);  /* replaces the marker -- every later lookup
                                of this id finds the real value directly */
  delete marker;  /* add_attr() overwrote the slot that held it above --
                      nothing else knows this is a FuncObjPendingArg and
                      would otherwise free it (see fire_funcobj()'s own
                      cleanup pass for a marker that's never read at all) */
  return al->find(id);
}

AttributeValue* ComTerp::peek_alist_pending(AttributeList* al, int id, AttributeValue* found) {
  if (!found || !found->is_object(FuncObjPendingArg::class_symid()))
    return found;
  FuncObjPendingArg* marker = (FuncObjPendingArg*)found->obj_val();
  ComValue pulled(pull_funcobj_pending(marker));
  if (pulled.is_stream()) {
    /* pin it instead of peeking -- same reasoning as funcobj_arg's
       identical stream case: a stream is a stateful cursor, not a value,
       so treating it like any other re-firable keyword would silently
       reset it to "just constructed" on every read, never advancing (or
       never exhausting, if something loops on it).  Write it back and
       delete the marker exactly like an explicit write already does
       (pull_alist_pending) -- every later read of this id, peek or pull,
       finds the one real, same-identity stream object directly. */
    al->add_attr(id, pulled);
    delete marker;
    return al->find(id);
  }
  *_peek_scratch = pulled;  /* fresh every call for anything else --
     never written to al, marker never deleted here (fire_funcobj()'s
     cleanup pass frees it at invocation end if it's never frozen by a
     write) */
  return _peek_scratch;
}

void ComTerp::set_args(int argc, char** argv) {
  if (_arg_strs) delete _arg_strs;
  _narg_strs = argc;
  _arg_strs = new int[_narg_strs];
  for (int i=0; i<_narg_strs; i++) _arg_strs[i] = symbol_add(argv[i]);
  return;
}

void ComTerp::set_args(const char* argstr) {
  int argc = 0;
  const char* argptr = argstr;

  char buffer[BUFSIZ];
  int bufoff = 0;
  while (*argptr) {
    while(*argptr && isspace(*argptr)) argptr++;
    if (!*argptr) break;
    while (*argptr && !isspace(*argptr) && bufoff<BUFSIZ-1) {
      if(*argptr=='"') {
        while(*argptr && (*argptr!='"' || *(argptr-1)!='\\') && bufoff<BUFSIZ-1) 
          buffer[bufoff++] = *argptr++;
      }
      buffer[bufoff++] = *argptr++;
    }
    buffer[bufoff] = '\0';
    bufoff=0;
    argc++;
  }

  if(argc<=1) return;

  if (_arg_strs) delete _arg_strs;
  _narg_strs = argc;
  _arg_strs = new int[_narg_strs];

  argptr = argstr;
  int curarg=0;
  while (*argptr) {
    while(*argptr && isspace(*argptr)) argptr++;
    if (!*argptr) break;
    while (*argptr && !isspace(*argptr) && bufoff<BUFSIZ-1) {
      if(*argptr=='"') {
        while(*argptr && (*argptr!='"' || *(argptr-1)!='\\') && bufoff<BUFSIZ-1) 
          buffer[bufoff++] = *argptr++;
      }
      buffer[bufoff++] = *argptr++;
    }
    buffer[bufoff] = '\0';
    bufoff=0;
    _arg_strs[curarg++] = symbol_add(buffer);
  }

  return;
}


void ComTerp::err_str(char* buf, int bufsiz, const char* cmd) {
    ::err_str(buf, bufsiz, cmd);
    if (strlen(buf) > 0) {
        strncpy(_errbuf2, buf, BUFSIZ-1);
	_errbuf2[BUFSIZ-1] = '\0';
    }
}

void ComTerp::err_print(FILE* out, const char* cmd) {
    char buf[BUFSIZ];
    buf[0] = '\0';
    ::err_str(buf, BUFSIZ, cmd);
    if (strlen(buf) > 0) {
        strncpy(_errbuf2, buf, BUFSIZ-1);
	_errbuf2[BUFSIZ-1] = '\0';
	fprintf(out, "%s\n", buf);
        ::err_clear();
    }
}
