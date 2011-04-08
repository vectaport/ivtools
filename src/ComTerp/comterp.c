/*
 * Copyright (c) 1994,1995,1998 Vectaport Inc.
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

#include <ComTerp/_comterp.h>
#include <ComTerp/_comutil.h>
#include <ComTerp/assignfunc.h>
#include <ComTerp/boolfunc.h>
#include <ComTerp/comfunc.h>
#include <ComTerp/comterp.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/condfunc.h>
#include <ComTerp/ctrlfunc.h>
#include <ComTerp/helpfunc.h>
#include <ComTerp/mathfunc.h>
#include <ComTerp/numfunc.h>
#include <ComTerp/strmfunc.h>
#include <Attribute/attrlist.h>

#include <ctype.h>
#include <iostream.h>
#include <strstream.h>
#include <unistd.h>

#define TITLE "ComTerp"

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
    _stack_siz = 256;
    if(dmm_calloc((void**)&_stack, _stack_siz, sizeof(ComValue)) != 0) 
	KANRET("error in call to dmm_calloc");


    _pfoff = 0;
    _quitflag = false;

    /* Create ComValue symbol table */
    _localtable = new ComValueTable(100);
    if (_globaltable) {
      _globaltable = new ComValueTable(100);
    }

    _errbuf = new char[BUFSIZ];

    _alist = nil;
    _brief = true;
    _just_reset = false;
    _defaults_added = false;
}


ComTerp::~ComTerp() {
    /* Free stack */
    if(dmm_free((void**)&_stack) != 0) 
	KANRET ("error in call to dmm_free");

    delete _errbuf;
}

const ComValue* ComTerp::stack(unsigned int &top) const {
    top = _stack_top;
    return _stack;
}

boolean ComTerp::read_expr() {
    unsigned int toklen, tokstart;
    int status = parser (_inptr, _infunc, _eoffunc, _errfunc, (FILE*)_outptr, _outfunc,
			 _buffer, _bufsiz, &_bufptr, _token, _toksiz, &_linenum,
			 &_pfbuf, &_pfsiz, &_pfnum);

    _pfoff = 0;
    return _pfbuf[_pfnum-1].type != TOK_EOF && _buffer[0] != '\0' && status==0;
}

boolean ComTerp::eof() {

    return _pfbuf[_pfnum-1].type == TOK_EOF;
}

boolean ComTerp::brief() const {
  return _brief;
}

int ComTerp::eval_expr(boolean nested) {
    _pfoff = 0;
    if (!nested)
        _stack_top = -1;
    while (_pfoff < _pfnum) {
	int nargkey = load_sub_expr();
	ComValue& sv = pop_stack();

	if (sv.type() == ComValue::CommandType) {

 	     ComFunc* func = (ComFunc*)sv.obj_val();
	     func->argcnts(sv.narg(), sv.nkey(), nargkey);
  	     func->execute();
	     if (_just_reset) {
	       push_stack(ComValue::blankval());
	       _just_reset = false;
	     }

	} else if (sv.type() == ComValue::SymbolType) {

	    if (_alist) {
     	        int id = sv.symbol_val();
	        AttributeValue* val = _alist->find(id);  
	        if (val) {
		    ComValue newval(*val);
		    push_stack(newval);
		} else
		    push_stack(ComValue::nullval());
	    } else 
		push_stack(sv);

	} else {  /* everything else*/

	    push_stack(sv);

	} 
    }
    return FUNCOK;
}

int ComTerp::load_sub_expr() {

    /* find the index of the last lazy_eval command in the postfix buffer */
    int top_lazy_eval = -1;
    int pfptr = _pfnum-1;
    while (pfptr > _pfoff ) {
      
        void *vptr = nil;

	/* look up ComFunc and check lazy_eval flag */
        if (_pfbuf[pfptr].type==TOK_COMMAND)
	  localtable()->find(vptr, _pfbuf[pfptr].v.dfintval);
        ComValue* comptr = (ComValue*)vptr;

        if (comptr && comptr->is_type(AttributeValue::CommandType)) {
	    ComFunc* comfunc = (ComFunc*)comptr->obj_val();
	    if (comfunc && comfunc->lazy_eval()) {
	        top_lazy_eval = pfptr;
	    }
	}
        pfptr--;
    }

    /* push tokens onto the stack until the last lazy_eval command is pushed */
    /* or if none, the first !lazy_eval command is pushed */
    boolean break_flag;
    while (_pfoff < _pfnum ) {
        push_stack(_pfbuf + _pfoff);
        _pfoff++;
	if (stack_top().type() == ComValue::CommandType && 
	(top_lazy_eval<0 || top_lazy_eval == _pfnum) ) break;
    }

    /* count down on stack to determine the number of */
    /* args associated with keywords for this command */
    if (stack_top().type() == ComValue::CommandType) {
      int nargs_after_key = 0;
      for (int i=0; i<_pfbuf[_pfoff-1].narg+_pfbuf[_pfoff-1].nkey; i++) {
	ComValue& val = stack_top(-i-1);
	if (val.is_type(ComValue::KeywordType))
	  nargs_after_key += val.keynarg_val();
      }
      return nargs_after_key;
    } else
      return 0;
    
}

int ComTerp::print_stack() const {
    for (int i = _stack_top; i >= 0; i--) {
	cout << _stack[i] << "\n";
    }
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
    ComValue* sv = _stack + _stack_top;
    *sv  = ComValue(token);

    /* See if this really is a command with a ComFunc */
    if (sv->type() == ComValue::SymbolType) {
        void* vptr = nil;
	unsigned int command_symid = sv->int_val();
	localtable()->find(vptr, command_symid);
	if (vptr && ((ComValue*)vptr)->type() == ComValue::CommandType) {
	    sv->obj_ref() = ((ComValue*)vptr)->obj_ref();
	    sv->type(ComValue::CommandType);
	    sv->command_symid(command_symid);
	}
    } else if (sv->type() == ComValue::KeywordType) {
      sv->keynarg_ref() = token->narg;
    }
    _just_reset = false;
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
    ComValue* sv = _stack + _stack_top;
    *sv  = ComValue(value);
    if (sv->type() == ComValue::KeywordType)
      sv->keynarg_ref() = value.keynarg_val();
    _just_reset = false;
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
    for (int i=0; i<n; i++) {
        ComValue& stacktop = _stack[_stack_top--];
	stacktop.AttributeValue::~AttributeValue();
    }
}

ComValue& ComTerp::pop_stack() {
    ComValue& stacktop = _stack[_stack_top--];
    return lookup_symval(stacktop);
}

ComValue& ComTerp::lookup_symval(ComValue& comval) {
    if (comval.type() == ComValue::SymbolType) {
        void* vptr = nil;
	if (localtable()->find(vptr, comval.symbol_val())) {
	    comval.assignval(*(ComValue*)vptr);
	    return comval;
	} else {
	    if (_alist) {
     	        int id = comval.symbol_val();
	        AttributeValue* aval = _alist->find(id);  
	        if (aval) {
		    ComValue newval(*aval);
		    *&comval = newval;
		}
		return comval;
	    } else 
	        return ComValue::nullval();
	}
    }       
    return comval;
}

ComValue& ComTerp::stack_top(int n) {
  if (_stack_top+n < 0 || _stack_top+n >= _stack_siz) {
    return ComValue::unkval();    
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

int ComTerp::add_command(const char* name, ComFunc* func) {
    int symid = symbol_add((char *)name);
    ComValue* comval = new ComValue();
    comval->type(ComValue::CommandType);
    comval->obj_ref() = (void*)func;
    comval->command_symid(symid);
    localtable()->insert(symid, comval);
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

int ComTerp::run() {

    char buffer[BUFSIZ];
    _errbuf[0] = '\0';

    while (!eof() && !quitflag()) {
	
	while (read_expr()) {
            err_str( _errbuf, BUFSIZ, "comterp" );
	    if (strlen(_errbuf)==0) {
		eval_expr();
		err_str( _errbuf, BUFSIZ, "comterp" );
		if (strlen(_errbuf)==0) {
		    if (quitflag()) 
			break;
		    else
			print_stack_top();
		    err_str( _errbuf, BUFSIZ, "comterp" );
		}
	    } 
            if (strlen(_errbuf)>0) {
		cout << _errbuf << "\n";
    		_errbuf[0] = '\0';
            }
	    _stack_top = -1;
	}
    }
    return 0;
}

void ComTerp::add_defaults() {
  if (!_defaults_added) {
    _defaults_added = true;

    add_command("add", new AddFunc(this));
    add_command("sub", new SubFunc(this));
    add_command("minus", new MinusFunc(this));
    add_command("mpy", new MpyFunc(this));
    add_command("div", new DivFunc(this));
    add_command("mod", new ModFunc(this));
    add_command("min", new MinFunc(this));
    add_command("max", new MaxFunc(this));

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
    add_command("repeat", new RepeatFunc(this));
    add_command("iterate", new IterateFunc(this));

    add_command("exp", new ExpFunc(this));
    add_command("log", new LogFunc(this));
    add_command("log10", new Log10Func(this));
    add_command("pow", new PowFunc(this));

    add_command("acos", new ACosFunc(this));
    add_command("asin", new ASinFunc(this));
    add_command("atan", new ATanFunc(this));
    add_command("atan2", new ATan2Func(this));
    add_command("cos", new CosFunc(this));
    add_command("sin", new SinFunc(this));
    add_command("tan", new TanFunc(this));

    add_command("cond", new CondFunc(this));
    add_command("seq", new SeqFunc(this));
    add_command("run", new RunFunc(this));

    add_command("help", new HelpFunc(this));
    add_command("symid", new SymIdFunc(this));
    add_command("symval", new SymValFunc(this));

    add_command("shell", new ShellFunc(this));
    add_command("quit", new QuitFunc(this));
    add_command("exit", new ExitFunc(this));
  }
}

void ComTerp::set_attributes(AttributeList* alist) { 
    Unref(_alist);
    _alist = alist; 
    Resource::ref(_alist);
}

AttributeList* ComTerp::get_attributes() { return _alist;}


int ComTerp::runfile(const char* filename) {
    /* save tokens to restore after the file has run */
    int toklen;
    postfix_token* tokbuf = copy_postfix_tokens(toklen);
    int tokoff = _pfoff;

    /* swap in input pointer and function */
    void* save_inptr = _inptr;
    infuncptr save_infunc = _infunc;
    outfuncptr save_outfunc = _outfunc;
    FILE* fptr = fopen(filename, "r");
    _inptr = fptr;
    _outfunc = nil;
    

    ComValue* retval = nil;
    int status = 0;
    while( !feof(fptr)) {
	if (read_expr()) {
	    if (eval_expr(true)) {
	        err_print( stderr, "comterp" );
	        filebuf obuf(1);
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

    _inptr = save_inptr;
    _infunc = save_infunc;
    _outfunc = save_outfunc;

    load_postfix(tokbuf, toklen, tokoff);
    delete tokbuf;

    if (retval) {
        push_stack(*retval);
	delete retval;
    } else
        push_stack(ComValue::nullval());

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
      char* command_name = symbol_pntr(funcids[i]);
      out << command_name;
      int slen = strlen(command_name);
      int tlen = 8-((slen+1)%8);
      rowcnt += slen + tlen;
      if (rowcnt>=64) {
	rowcnt = 0;
	out << "\n";
      } else
	out << "\t";
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
      }
      buffer[ncomm++] = key;
    }
    i.next();
  }
  if (sort) {
    int* sortedbuffer = new int[ncomm];
    int i = 0;  /* operators first */
    for (int j=0; j< ncomm; j++) sortedbuffer[j] = -1;
    for (int j=0; j< ncomm; j++)
      if (!isalpha(*symbol_pntr(buffer[j])))
	  sortedbuffer[i++] = buffer[j];
    if (i != opercnt) cerr << "bad number of operators\n";
      
    for (int j=0; j<ncomm; j++) {
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
    for (int j=0; j<ncomm; j++) {
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

