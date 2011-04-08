/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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

#include <ComTerp/comfunc.h>
#include <ComTerp/comterp.h>
#include <ComTerp/comvalue.h>
#include <Attribute/attrlist.h>
#include <string.h>

#define TITLE "ComFunc"

/*****************************************************************************/

ComFunc::ComFunc(ComTerp* comterp) {
    _comterp = comterp;
}

void ComFunc::reset_stack() {
  if (!post_eval()) {
    int count = nargs() + nkeys() - npops();
    for (int i=1; i<=npops(); i++) 
      _comterp->stack_top(i).AttributeValue::~AttributeValue();
    
    _comterp->decr_stack(count);
  } else 
    _comterp->decr_stack(1);
  _comterp->_just_reset = true;
  npops() = 0;
}


ComValue& ComFunc::stack_arg(int n, boolean symbol, ComValue& dflt) {
    if (post_eval()) return stack_arg_post(n, symbol, dflt);

    int count = nargs() + nkeys() - npops();

    for (int i=0; i<count; i++) {
        ComValue& argref = _comterp->stack_top(i-count+1);
        if( argref.type() == ComValue::KeywordType) 
	    return ComValue::nullval();
        if (i == n) {
  	    if (i+1 < count) {
	      ComValue& keyref = _comterp->stack_top(i-count+2);
	      if (keyref.is_type(ComValue::KeywordType) &&
		  keyref.keynarg_val())
		return ComValue::nullval();
	    }
	    if (!symbol) 
	        argref = _comterp->lookup_symval(argref);
	    return argref;
	}
    }
    return dflt;
}

ComValue& ComFunc::stack_key(int id, boolean symbol, ComValue& dflt, boolean use_dflt_for_no_key) {
  if (post_eval()) 
    return stack_key_post(id, symbol, dflt, use_dflt_for_no_key);

  int count = nargs() + nkeys() - npops();
  for (int i=0; i<count; i++) {
    ComValue& keyref = _comterp->stack_top(-i);
    if( keyref.type() == ComValue::KeywordType) {
	    if (keyref.symbol_val() == id) {
	      if (i+1==count || keyref.keynarg_val() == 0) {
		if (use_dflt_for_no_key) 
		  return ComValue::trueval();
		else
		  return dflt;
	      } else {
		ComValue& valref = _comterp->stack_top(-i-1);
		if (valref.type() == ComValue::KeywordType) {
		  if (use_dflt_for_no_key) 
		    return ComValue::trueval();
		  else
		    return dflt;
		} else {
		  if (!symbol) 
		    valref = _comterp->lookup_symval(valref);
		  return valref;
		}
	      }
	    }
    }
  }
  return use_dflt_for_no_key ? dflt : ComValue::nullval();
}

ComValue& ComFunc::stack_dotname(int n) {
    return _comterp->stack_top(n+1+npops());
}

ComValue& ComFunc::stack_arg_post_eval(int n, boolean symbol, ComValue& dflt) {
  ComValue argoff(comterp()->stack_top());
  int offtop = argoff.int_val()-comterp()->_pfnum;
  int argcnt;
  for (int i=0; i<nkeys(); i++) {
    argcnt = 0;
    skip_key_in_expr(offtop, argcnt);
  }

  if (n>=nargsfixed()) return dflt;  

  for (int i=nargsfixed(); i>n; i--) {
    argcnt = 0;
    skip_arg_in_expr(offtop, argcnt);
  }

  comterp()->post_eval_expr(argcnt, offtop, pedepth()+1);

  return comterp()->pop_stack(!symbol);
}

ComValue& ComFunc::stack_key_post_eval
(int id, boolean symbol, ComValue& dflt, boolean use_dflt_for_no_key) {
  ComValue argoff(comterp()->stack_top());
  int offtop = argoff.int_val()-comterp()->_pfnum;
  int count = 0;
  while (count < nkeys()) {
    ComValue& curr = comterp()->expr_top(offtop);
    if (!curr.is_type(ComValue::KeywordType))
      return use_dflt_for_no_key ? dflt : ComValue::nullval();
    count++;
    int argcnt = 0;
    skip_key_in_expr(offtop, argcnt);
    if (curr.symbol_val() == id) {
      if (argcnt) {
	comterp()->post_eval_expr(argcnt, offtop, pedepth()+1);
	return comterp()->pop_stack(!symbol);
      } else
	return use_dflt_for_no_key ? dflt : ComValue::trueval();
    } 
  }
  return use_dflt_for_no_key ? dflt : ComValue::nullval();
}

ComValue& ComFunc::stack_arg_post(int n, boolean symbol, ComValue& dflt) {
  ComValue argoff(comterp()->stack_top());
  int offtop = argoff.int_val()-comterp()->_pfnum;
  int argcnt;
  for (int i=0; i<nkeys(); i++) {
    argcnt = 0;
    skip_key_in_expr(offtop, argcnt);
  }

  if (n>=nargsfixed()) return dflt;  

  for (int i=nargsfixed(); i>n; i--) {
    argcnt = 0;
    skip_arg_in_expr(offtop, argcnt);
  }

  int loc = comterp()->_pfnum + offtop + argcnt-1;
  return comterp()->_pfcomvals[loc];
}

ComValue& ComFunc::stack_key_post
(int id, boolean symbol, ComValue& dflt, boolean use_dflt_for_no_key) {
  ComValue argoff(comterp()->stack_top());
  int offtop = argoff.int_val()-comterp()->_pfnum;
  int count = 0;
  while (count < nkeys()) {
    ComValue& curr = comterp()->expr_top(offtop);
    if (!curr.is_type(ComValue::KeywordType))
      return use_dflt_for_no_key ? dflt : ComValue::nullval();
    count++;
    int argcnt = 0;
    skip_key_in_expr(offtop, argcnt);
    if (curr.symbol_val() == id) {
      if (argcnt) {
	int loc = comterp()->_pfnum + offtop + argcnt-1;
	return comterp()->_pfcomvals[loc];
      } else
	return use_dflt_for_no_key ? dflt : ComValue::trueval();
    } 
  }
  return use_dflt_for_no_key ? dflt : ComValue::nullval();
}

boolean ComFunc::skip_key_on_stack(int& stackptr, int& argcnt) {
  return comterp()->skip_key(&comterp()->stack_top(), stackptr, argcnt);
}

boolean ComFunc::skip_arg_on_stack(int& stackptr, int& argcnt) {
  return comterp()->skip_arg(&comterp()->stack_top(), stackptr, argcnt);
}

boolean ComFunc::skip_key_in_expr(int& offtop, int& argcnt) {
  return comterp()->skip_key(&comterp()->_pfcomvals[comterp()->_pfnum-1], 
			     offtop, argcnt);
}

boolean ComFunc::skip_arg_in_expr(int& offtop, int& argcnt) {
  return comterp()->skip_arg(&comterp()->_pfcomvals[comterp()->_pfnum-1], 
			     offtop, argcnt);
}

ComValue& ComFunc::pop_stack() {

    /* get rid of keywords -- use stack_key and stack_arg to get those */
    if (!npops() && nkeys()) {
        int count = nargs() + nkeys();
	int nkey = nkeys();
        for (int i=0; i<count; i++) {
	    ComValue& val = _comterp->pop_stack();
	    npops()++;
	    if (val.type() == ComValue::KeywordType) nkey--;
	    if (nkey==0) break;    
	}
    }

    if (npops()<nargs()+nkeys()) {
        npops()++;
	return _comterp->pop_stack();
    } else 
        return ComValue::nullval();
}

ComValue& ComFunc::pop_symbol() {
    /* get rid of keywords -- use stack_key and stack_arg to get those */
    if (!npops() && nkeys()) {
        int count = nargs() + nkeys();
	int nkey = nkeys();
        for (int i=0; i<count; i++) {
	    ComValue& val = _comterp->pop_stack();
	    npops()++;
	    if (val.type() == ComValue::KeywordType) nkey--;
	    if (nkey==0) break;    
	}
    }

    if (npops()<nargs()+nkeys()) {
        npops()++;
	return _comterp->pop_symbol();
    } else 
        return ComValue::nullval();
}

void ComFunc::push_stack(ComValue& val) {
    _comterp->push_stack(val);
}

void ComFunc::push_stack(AttributeValue& val) {
    _comterp->push_stack(val);
}

ComValue& ComFunc::lookup_symval(ComValue& sym) {
    ComValue& retval = _comterp->lookup_symval(sym);
    return retval;
}

void ComFunc::assign_symval(int id, ComValue* sym) {
    _comterp->localtable()->insert(id, sym);
    return;
}

int ComFunc::bintest(const char* command) {
  char combuf[BUFSIZ];
  sprintf( combuf, "which %s", command );
  FILE* fptr = popen(combuf, "r");
  char testbuf[BUFSIZ];	
  fgets(testbuf, BUFSIZ, fptr);  
  pclose(fptr);
  if (strncmp(testbuf+strlen(testbuf)-strlen(command)-1, 
	      command, strlen(command)) != 0) {
    return -1;
  }
  return 0;
}

boolean ComFunc::bincheck(const char* command) {
  int status = bintest(command);
  return !status;
}

ComFuncState* ComFunc::funcstate() {
  return _comterp->top_funcstate();
}

void ComFunc::push_funcstate(int nargs, int nkeys, int pedepth,
			     int command_symid) {
  ComFuncState cfs(nargs, nkeys, pedepth, command_symid);
  _comterp->push_funcstate(cfs);
}

void ComFunc::pop_funcstate() {
  _comterp->pop_funcstate();
}

void ComFunc::exec(int nargs, int nkeys, int pedepth,
		   int command_symid) {
  push_funcstate(nargs, nkeys, pedepth, command_symid);
  execute();
  pop_funcstate();
}

int& ComFunc::nargs() {
  return _comterp->top_funcstate()->nargs();
}

int& ComFunc::nkeys() {
  return _comterp->top_funcstate()->nkeys();
}

int& ComFunc::npops() {
  return _comterp->top_funcstate()->npops();
}

int ComFunc::nargspost() {
  ComFuncState* funcstate = _comterp->top_funcstate();
  if (funcstate->nargspost()>=0) 
    return funcstate->nargspost();

  int nargs = funcstate->nargs();
  int nkeys = funcstate->nkeys();
  int topptr = 0;
  if (post_eval()) {
    ComValue argoff(comterp()->stack_top());
    topptr = argoff.int_val()-comterp()->_pfnum;
  }
  int argcnt=0;
  while (nargs>0 || nkeys>0) {
    ComValue& val = comterp()->expr_top(topptr);
    int cnt = 0;
    if (val.is_type(ComValue::KeywordType)) {
      argcnt++;
      skip_key_in_expr(topptr, cnt);
      argcnt += cnt;
      nargs -= cnt ? 1 : 0;
      nkeys--;
    } else {
      skip_arg_in_expr(topptr, cnt);
      argcnt += cnt;
      nargs--;
    }
  }
  return argcnt;
}

int ComFunc::nargskey() {
  ComFuncState* funcstate = _comterp->top_funcstate();
  if (funcstate->nargskey()>=0) 
    return funcstate->nargskey();

  int nkeys = funcstate->nkeys();
  int topptr = 0;
  if (post_eval()) {
    ComValue argoff(comterp()->stack_top());
    topptr = argoff.int_val()-comterp()->_pfnum;
  }
  int nkeyargs = 0;
  while (nkeys>0) {
    int argcnt = 0;
    if (!post_eval()) { 
      skip_key_on_stack(topptr, argcnt);
    } else {
      skip_key_in_expr(topptr, argcnt);
    }
    nkeyargs += argcnt ? 1 : 0;
    nkeys--;
  }
  funcstate->nargskey() = nkeyargs;
  return nkeyargs;
}

int& ComFunc::pedepth() {
  return _comterp->top_funcstate()->pedepth();
}

AttributeList* ComFunc::stack_keys(boolean symbol, AttributeValue& dflt) {
  AttributeList* al = new AttributeList();
  int count = nargs() + nkeys() - npops();
  for (int i=0; i<count; i++) {
    ComValue& keyref = _comterp->stack_top(-i);
    if( keyref.type() == ComValue::KeywordType) {
      int key_symid = keyref.symbol_val();
      if (i+1==count || keyref.keynarg_val() == 0) 
	al->add_attr(key_symid, dflt);
      else {
	ComValue& valref = _comterp->stack_top(-i-1);
	if (valref.type() == ComValue::KeywordType) 
	  al->add_attr(key_symid, dflt);
	else {
	  if (!symbol) 
	    valref = _comterp->lookup_symval(valref);
	  al->add_attr(key_symid, valref);
	}
      }
    }
  }
  return al;
}

/*****************************************************************************/

ComFuncState::ComFuncState(int narg, int nkey, int pedepth, 
			   int command_symid) {
  _nargs = narg;
  _nkeys = nkey;
  _npops = 0;
  _nargskey = -1;
  _nargspost = -1;
  _pedepth = pedepth;
  _command_symid = command_symid;
}

ComFuncState::ComFuncState(ComFuncState& cfs) {
  *this = cfs;
}
