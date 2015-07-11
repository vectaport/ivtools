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

#include <cstdio>
#include <ctype.h>
#include <iostream.h>
#include <string.h>
#include <strstream>
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
#include <ComTerp/postfunc.h>
#include <ComTerp/randfunc.h>
#include <ComTerp/statfunc.h>
#include <ComTerp/strmfunc.h>
#include <ComTerp/symbolfunc.h>
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
#endif

#define TITLE "ComTerp"
#define STREAM_MECH

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

    _pfcomvals = nil;

    /* Create ComValue symbol table */
    _localtable = new ComValueTable(100);
#if 0  /* deferred until first use */
    if (!_globaltable) {
      _globaltable = new ComValueTable(100);
    }
#endif

    _errbuf = new char[BUFSIZ];

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
    _fd = -1;
    _arg_strs = nil;
    _narg_strs = 0;
    _top_commands = NULL;
}


ComTerp::~ComTerp() {
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

  if (!nested)
    _stack_top = -1;
  while (_pfoff < _pfnum) {
    load_sub_expr();
    eval_expr_internals();
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
  }

  pop_servstate();

  return FUNCOK;
}

void ComTerp::eval_expr_internals(int pedepth) {
  static int step_symid = symbol_add("step");
  ComValue sv = pop_stack(false);
  
  if (sv.type() == ComValue::CommandType) {

#ifdef STREAM_MECH
    /* if func has StreamType ComValue's for arguments */
    /* create another StreamType ComValue to hold all its */
    /* arguments, along with a pointer to the func. */
    boolean has_streams = false;
    int streamid = -1;
    if (!((ComFunc*)sv.obj_val())->post_eval())
      for(int i=0; i<sv.narg()+sv.nkey(); i++) {
	if (!stack_top(-i).is_symbol() && !stack_top(-i).is_attribute())
	  has_streams = stack_top(-i).is_stream();
	else {
	  AttributeValue* testval = 
	    lookup_symval(&stack_top(-i));
	  has_streams = testval ? testval->is_stream() : false;
	}
	if (has_streams) {
	  streamid = i;
	  break;
	}
      }
    if (has_streams) {
      AttributeValueList* avl = new AttributeValueList();
      
      /* if delims associated with symbol, put that first in stream list */
      if (_delim_func && sv.nids()!=1) {
	ComValue nameval(sv.command_symid(), ComValue::SymbolType);
	avl->Prepend(new AttributeValue(nameval));
      }

      for(int i=0; i<sv.narg()+sv.nkey(); i++) {
	ComValue topval(pop_stack(i==streamid));
	avl->Prepend(new AttributeValue(topval));
      }

      ComValue val((ComFunc*)sv.obj_val(), avl);
      // fprintf(stderr, "Just packed up stream for %s\n", symbol_pntr(((ComFunc*)sv.obj_val())->funcid()));
      val.stream_mode(1); // for external use
      push_stack(val);
      return;
    }
#endif

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
      func->push_funcstate(nargs, nkeys, pedepth, func->funcid());
    }

    /* output execution trace */
    if (this->trace_mode()) {
      for(int i=0; i<pedepth; i++) cerr << "    ";
      cerr << symbol_pntr(sv.command_symid());
      if (func->post_eval()) 
	cerr << ": nargs=" << nargs << " nkeys=" << nkeys << "\n";
      else {
	int ntotal = func->nargs() + func->nkeys();
	for(int i=0; i<ntotal; i++) {
	  if (i) 
	    cerr << " ";
	  else 
	    cerr << "(";
	  cerr << stack_top(i-ntotal+1);
	}
	cerr << ")\n";
      }
    }

    if (stepflag()) {
      FILEBUF(fbufout, handler() && handler()->wrfptr() ? handler()->wrfptr() : stdout, ios_base::out);
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
    func->pop_funcstate();

    if (_just_reset && !_func_for_next_expr) {
      push_stack(ComValue::blankval());
      _just_reset = false;
    }

    if (stack_base+1 < _stack_top) {
      fprintf(stderr, "func \"%s\" pushed more than a single value on stack\n", symbol_pntr(func->funcid()));
      fprintf(stderr, "stack_base %d, stack_top %d\n", stack_base, _stack_top);
      for(int i=stack_base+1; i<=_stack_top; i++)
          std::cerr << i << ":  " << _stack[i] << "\n";
    }
    else if (stack_base+1 > _stack_top)
      fprintf(stderr, "func \"%s\" failed to push a single value on stack\n", symbol_pntr(func->funcid()));
    
  } else if (sv.type() == ComValue::SymbolType) {

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
	int id = sv.symbol_val();
	AttributeValue* val = _alist->find(id);  
	if (val) {
	  ComValue newval(*val);
	  push_stack(newval);
	} else
	  push_stack(ComValue::nullval());
      } else 
	push_stack(lookup_symval(sv));
    }
    
  } else if (sv.is_object(Attribute::class_symid())) {
    

    push_stack(*((Attribute*)sv.obj_val())->Value());
    
  } else if (sv.type() == ComValue::BlankType) {

    if (!stack_empty()) eval_expr_internals(pedepth);

  } else {  /* everything else*/
    
    push_stack(sv);
    
  }
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
        AttributeValue* av = lookup_symval(&stack_top(0));
	if (av && av->is_array()) av->array_val()->nested_insert(true);
      } 
    }
    _pfoff++;
    if (stack_top().type() == ComValue::CommandType && 
	!_pfcomvals[_pfoff-1].pedepth()) break;
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
	      AttributeValue* av = lookup_symval(&stack_top(0));
	      if (av->is_array()) av->array_val()->nested_insert(true);
	    }
	  }
	}
	tokcnt--;
	offset++;
	if (_pfcomvals[offset-1].pedepth()!=pedepth)
	  continue;
	if (stack_top().is_type(ComValue::CommandType) 
	    && stack_top().pedepth() == pedepth) break;
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
  if (((int)_pfnum)+n < 0 || n>0) {
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
    cout << _stack[_stack_top] << "\n";
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
	_stack_siz *= 2;
	dmm_realloc_size(sizeof(ComValue));
	if(dmm_realloc((void**)&_stack, (unsigned long)_stack_siz) != 0) {
	    KANRET("error in call to dmm_realloc");
	    return;
	}
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
      char buf[bufsiz];
      strcpy(buf, sv->symbol_ptr()+2);
      command_symid = symbol_add(buf);
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

    /* handle case where symbol has arguments/keywords, but is not defined */
    else if (!vptr && (sv->narg() || sv->nkey())) {
      static int nil_symid = symbol_add("nil");
      localtable()->find(vptr, nil_symid);
    }

    if (vptr && ((ComValue*)vptr)->type() == ComValue::CommandType) {
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
	_stack_siz *= 2;
	dmm_realloc_size(sizeof(ComValue));
	if(dmm_realloc((void**)&_stack, (unsigned long)_stack_siz) != 0) {
	    KANRET("error in call to dmm_realloc");
	    return;
	}
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
    if (sv.type() == ComValue::SymbolType) {
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
      topval.assignval(*((Attribute*)topval.obj_val())->Value());
      return topval;
    }else 
      return topval;

  } else {
    cerr << "stack empty, blank returned\n";
    return ComValue::blankval();
  }
}

ComValue& ComTerp::lookup_symval(ComValue& comval) {
    if (comval.bquote()) return comval;

    if (comval.type() == ComValue::SymbolType) {
        void* vptr = nil;

	if (!comval.global_flag() && localtable()->find(vptr, comval.symbol_val()) ) {
	  comval.assignval(*(ComValue*)vptr);
	  return comval;
	} else  if (_alist) {
	  int id = comval.symbol_val();
	  AttributeValue* aval = _alist->find(id);  
	  if (aval) {
	    ComValue newval(*aval);
	    *&comval = newval;
	  }
	  return comval;
	} else if (globaltable()->find(vptr, comval.symbol_val())) {
	  comval.assignval(*(ComValue*)vptr);
	  return comval;
	} else
	  return ComValue::nullval();

    } else if (comval.is_object(Attribute::class_symid())) {

      comval.assignval(*((Attribute*)comval.obj_val())->Value());

    }       
    return comval;
}

AttributeValue* ComTerp::lookup_symval(ComValue* comval) {
    if (comval->bquote()) return nil;

    if (comval->type() == ComValue::SymbolType) {
        void* vptr = nil;

	if (!comval->global_flag() && localtable()->find(vptr, comval->symbol_val()) ) {
	  return (AttributeValue*)vptr;
	} else  if (_alist) {
	  int id = comval->symbol_val();
	  AttributeValue* aval = _alist->find(id);  
	  if (aval) {
	    return aval;
	  }
	  return nil;
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

int ComTerp::add_command(const char* name, ComFunc* func, const char* alias, const char* docstring2) {
    int symid = symbol_add((char *)name);

    if(docstring2) func->docstring2(docstring2);

    if (!_top_commands)
        _top_commands = new AttributeValueList();
    _top_commands->Append(new AttributeValue(symid, AttributeValue::SymbolType));

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
  _exit( status );
}

boolean ComTerp::quitflag() {
    return _quitflag;
}

void ComTerp::quitflag(boolean flag) {
    _quitflag = flag;
}

int ComTerp::run(boolean one_expr, boolean nested) {
  int old_runflag = running();
  running(true);

  int status = 1;
  _errbuf[0] = '\0';
  char errbuf_save[BUFSIZ];
  errbuf_save[0] = '\0';

  FILEBUF(fbuf, handler() && handler()->wrfptr() ? handler()->wrfptr() : (_fd>0 ? fdopen(_fd, "w") : stdout), ios_base::out);
  ostream out(&fbuf);
  boolean eolflag = false;
  boolean errorflag = false;

  while (!eof() && !quitflag() && !eolflag) {
    
    if (read_expr()) {
      status = 0;
      int top_before = _stack_top;
      eval_expr(nested);
      if (top_before == _stack_top)
	status = 2;
      err_str( _errbuf, BUFSIZ, "comterp" );
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
		print_stack_top(out);
		out << "\n"; out.flush(); 
	      }
	    } while (stack_top().is_known());
	  } else {
	    print_stack_top(out);
	    out << "\n"; out.flush(); 
	  }
	}
      } else {
	out << _errbuf << "\n"; out.flush();
	strcpy(errbuf_save, _errbuf);
	_errbuf[0] = '\0';
      }
    } else {
      err_str( _errbuf, BUFSIZ, "comterp" );
      if (strlen(_errbuf)>0) {
	errorflag = true;
	out << _errbuf << "\n"; out.flush();
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
  if (!_defaults_added) {
    _defaults_added = true;

    add_command("nil", new NilFunc(this));
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
    add_command("mod", new ModFunc(this));
    add_command("min", new MinFunc(this));
    add_command("max", new MaxFunc(this));
    add_command("abs", new AbsFunc(this));

    add_command("assign", new AssignFunc(this));
    add_command("mod_assign", new ModAssignFunc(this));
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

    add_command("stream", new StreamFunc(this));
    add_command("concat", new ConcatFunc(this));
    add_command("repeat", new RepeatFunc(this));
    add_command("iterate", new IterateFunc(this));
    add_command("next", new NextFunc(this));
    add_command("each", new EachFunc(this));
    add_command("filter", new FilterFunc(this));

    add_command("dot", new DotFunc(this));
    add_command("attrname", new DotNameFunc(this));
    add_command("attrval", new DotValFunc(this));

    add_command("list", new ListFunc(this));
    add_command("at", new ListAtFunc(this));
    add_command("size", new ListSizeFunc(this));
    add_command("tuple", new TupleFunc(this));
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
    add_command("pause", new ComterpPauseFunc(this));
    add_command("step", new ComterpStepFunc(this));
    add_command("stackheight", new ComterpStackHeightFunc(this));
    add_command("symid", new SymIdFunc(this));
    add_command("symval", new SymValFunc(this));
    add_command("symbol", new SymbolFunc(this));
    add_command("symadd", new SymAddFunc(this));
    add_command("symvar", new SymVarFunc(this));
    add_command("symstr", new SymStrFunc(this));
    add_command("strref", new StrRefFunc(this));
    add_command("global", new GlobalSymbolFunc(this));
    add_command("split", new SplitStrFunc(this));
    add_command("join", new JoinStrFunc(this));
    add_command("substr", new SubStrFunc(this));

    add_command("type", new TypeSymbolFunc(this));
    add_command("class", new ClassSymbolFunc(this));

    add_command("bquote", new BackQuoteFunc(this));

    add_command("postfix", new PostFixFunc(this));
    add_command("posteval", new PostEvalFunc(this));

    add_command("if", new IfThenElseFunc(this));
    add_command("for", new ForFunc(this));
    add_command("while", new WhileFunc(this));
    add_command("switch", new SwitchFunc(this));

    add_command("open", new OpenFileFunc(this));
    add_command("close", new CloseFileFunc(this));
    add_command("print", new PrintFunc(this));
    add_command("gets", new GetStringFunc(this));

    add_command("usleep", new USleepFunc(this));
#ifdef HAVE_ACE
    add_command("timeexpr", new TimeExprFunc(this));
#endif

    add_command("eval", new EvalFunc(this));
    add_command("shell", new ShellFunc(this));
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
	    if (eval_expr(true)) {
	        err_print( stderr, "comterp" );
		FILEBUF(obuf, stdout, ios_base::out);
		ostream ostr(&obuf);
		ostr << "err\n";
		ostr.flush();
		status = -1;
	    } else if (quitflag()) {
	        status = 1;
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
      const char* command_name = symbol_pntr(key);
      int opid = opr_tbl_opstr(key);
      const char* operator_name = symbol_pntr(opr_tbl_operid(opid));
      if (operator_name) {
	key = opr_tbl_operid(opid);
	opercnt++;
      }
      if (ncomm==bufsiz) {
	int* newbuf = new int[bufsiz*2];
	for (int j=0; j<bufsiz; j++) 
	  newbuf[j] = buffer[j];
	bufsiz *= 2;
	delete buffer;
	buffer = newbuf;
      }
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
    delete buffer;

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

extern int _continuation_prompt_disabled;  // from ComUtil/parser.c

void ComTerp::disable_prompt() { _continuation_prompt_disabled = 1; }
void ComTerp::enable_prompt() { _continuation_prompt_disabled = 0; }

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
}

int ComTerp::arg_str(int n) {
  if (n<0 || n>_narg_strs) return -1;
  return _arg_strs ? _arg_strs[n] : nil;
}

int ComTerp::narg_str() {
  return _narg_strs;
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

