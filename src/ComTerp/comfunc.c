/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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
#include <string.h>

#define TITLE "ComFunc"

/*****************************************************************************/

ComFunc::ComFunc(ComTerp* comterp) {
    _comterp = comterp;
}

void ComFunc::argcnts(int narg, int nkey, int nargskey) {
    _nargs = narg;
    _nkeys = nkey;
    _nargskey = nargskey;
    _npops = 0;
}

void ComFunc::reset_stack() {
    int count = _nargs + _nkeys - _npops;
    for (int i=1; i<=_npops; i++) 
        _comterp->stack_top(i).AttributeValue::~AttributeValue();

    _comterp->decr_stack(count);
    _comterp->_just_reset = true;
    _npops = 0;
}


ComValue& ComFunc::stack_arg(int n, boolean symbol, ComValue& dflt) {
    int count = nargs() + nkeys() - _npops;

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

ComValue& ComFunc::stack_key(int id, boolean symbol, ComValue& dflt, boolean always) {
  int count = nargs() + nkeys() - _npops;
  for (int i=0; i<count; i++) {
    ComValue& keyref = _comterp->stack_top(-i);
    if( keyref.type() == ComValue::KeywordType) {
	    if (keyref.symbol_val() == id) {
	      if (i+1==count || keyref.narg() == 0) {
		if (always) 
		  return ComValue::trueval();
		else
		  return dflt;
	      } else {
		ComValue& valref = _comterp->stack_top(-i-1);
		if (valref.type() == ComValue::KeywordType) {
		  if (always) 
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
  return always ? dflt : ComValue::nullval();
}

ComValue& ComFunc::stack_dotname(int n) {
    return _comterp->stack_top(n+1+_npops);
}

ComValue& ComFunc::pop_stack() {

    /* get rid of keywords -- use stack_key and stack_arg to get those */
    if (!_npops && _nkeys) {
        int count = _nargs + _nkeys;
	int nkey = _nkeys;
        for (int i=0; i<count; i++) {
	    ComValue& val = _comterp->pop_stack();
	    _npops++;
	    if (val.type() == ComValue::KeywordType) nkey--;
	    if (nkey==0) break;    
	}
    }

    if (_npops<_nargs+_nkeys) {
        _npops++;
	return _comterp->pop_stack();
    } else 
        return ComValue::nullval();
}

ComValue& ComFunc::pop_symbol() {
    /* get rid of keywords -- use stack_key and stack_arg to get those */
    if (!_npops && _nkeys) {
        int count = _nargs + _nkeys;
	int nkey = _nkeys;
        for (int i=0; i<count; i++) {
	    ComValue& val = _comterp->pop_stack();
	    _npops++;
	    if (val.type() == ComValue::KeywordType) nkey--;
	    if (nkey==0) break;    
	}
    }

    if (_npops<_nargs+_nkeys) {
        _npops++;
	return _comterp->pop_symbol();
    } else 
        return ComValue::nullval();
}

void ComFunc::push_stack(ComValue& val) {
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
