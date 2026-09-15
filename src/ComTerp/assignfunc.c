/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1997,1999 Vectaport Inc.
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

#include <ComTerp/assignfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/listfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <InterViews/resource.h>

#include <fstream>
#include <iostream>
using std::cout;
using std::cerr;

#define TITLE "AssignFunc"

/*****************************************************************************/


AssignFunc::AssignFunc(ComTerp* comterp) : ComFunc(comterp) {
}


void AssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.is_command() && stack_arg_post_eval_size(0)==1) {
        cout << "WARNING:  assignment to command \"" << operand1.command_name() << "\" without args not allowed -- line " << funcstate()->linenum() << "\n";
	reset_stack();
	push_stack(ComValue::nullval());
	return;
    }
    
    if (operand1.type() != ComValue::SymbolType) {
        // if lhs is global()/local()/at() (including lst@N=val), set lhs_assign on its ComValue to distinguish lhs from rhs context
        static int global_symid = symbol_add("global");
        static int local_symid = symbol_add("local");
        static int at_symid = symbol_add("at");
        ComValue argoff(comterp()->stack_top());
        int offtop = argoff.int_val() - comterp()->pfnum();
        int arg0top = offtop;
        int argcnt = 0;
        skip_arg_in_expr(arg0top, argcnt);
        int startidx = comterp()->pfnum() - 1 + arg0top;
        ComValue& startval = comterp()->pfcomvals()[startidx];
        if (startval.is_type(ComValue::CommandType)) {
            ComFunc* func = (ComFunc*)startval.obj_val();
            if (func->funcid() == global_symid || func->funcid() == local_symid ||
                func->funcid() == at_symid)
                startval.lhs_assign(1);
        }
        operand1 = stack_arg_post_eval(0, true /* no symbol or attribute lookup */);
    }
    ComValue* operand2 = new ComValue(stack_arg_post_eval(1, true /* no symbol or attribute lookup */));
#ifdef POSTEVAL_EXPERIMENT
    if (operand2->is_attribute() || operand2->is_symbol()) lookup_symval(*operand2);
#else
    if (operand2->is_attribute()) lookup_symval(*operand2);
#endif
    if (operand1.type() == ComValue::SymbolType) {
        AttributeList* attrlist = comterp()->get_attributes();
	/* global() lvalue tested before func-frame branch; old-value cleanup reads globaltable directly via globalvalue(), not lookup_symval() (nil for bquoted lvalue symbols) */
	if (operand1.global_flag()) {
	    ComValue* oldval = comterp()->globalvalue(operand1.symbol_val());
	    if (oldval) {
	      comterp()->globaltable()->remove(operand1.symbol_val());
	      delete oldval;
	    }
	    comterp()->globaltable()->insert(operand1.symbol_val(), operand2);
	} else if (operand1.local_flag()) {
	    /* local() lvalue: write the default (per-instance) symbol table, skipping any func frame -- the session-scope escape */
	    ComValue* oldval = comterp()->localvalue(operand1.symbol_val());
	    if (oldval) {
	      comterp()->localtable()->remove(operand1.symbol_val());
	      delete oldval;
	    }
	    comterp()->localtable()->insert(operand1.symbol_val(), operand2);
	} else if (attrlist) {
	    if (value_contains_container(*operand2, (void*)attrlist, true)) {
	      fprintf(stderr, "WARNING: refusing to insert an attrlist into itself -- line %d\n",
		      funcstate()->linenum());
	      delete operand2;
	      reset_stack();
	      push_stack(ComValue::nullval());
	      return;
	    }
	    Resource::ref(attrlist);
	    Attribute* attr = new Attribute(operand1.symbol_val(),
					    operand2);
	    attrlist->add_attribute(attr);
	    Unref(attrlist);
	}
	else {
	    AttributeValue* oldval = comterp()->lookup_symval(&operand1);
	    if (oldval) {
	      comterp()->localtable()->remove(operand1.symbol_val());
	      delete (ComValue*)oldval;
	    }
            comterp()->localtable()->insert(operand1.symbol_val(), operand2);
	}
    } else if (operand1.is_object(Attribute::class_symid())) {
      Attribute* attr = (Attribute*)operand1.obj_val();
      AttributeList* owner = attr->Owner();
      if (owner && value_contains_container(*operand2, (void*)owner, true)) {
	fprintf(stderr, "WARNING: refusing to insert an attrlist into itself -- line %d\n",
		funcstate()->linenum());
	delete operand2;
	reset_stack();
	push_stack(ComValue::nullval());
	return;
      }
      attr->Value(operand2);
    } else if (operand1.is_array() && operand1.lhs_assign()) {
      /* the @ operator: lst@N=val -- ListAtFunc handed back a [list, idx] pair, so complete the write by re-driving at() with a real :set keyword */
      AttributeValueList* pair = operand1.array_val();
      static int set_symid = symbol_add("set");
      push_stack(*pair->Get(0));
      push_stack(*pair->Get(1));
      push_stack(*operand2);
      ComValue setkey(set_symid, 1);
      push_stack(setkey);
      ListAtFunc atfunc(comterp());
      atfunc.funcid(symbol_add("at"));
      /* narg counts non-keyword args including the value after a keyword -- 4 pushes here mean narg=3, nkey=1, not narg=2 */
      atfunc.exec(3, 1);
      ComValue result(pop_stack());
      *operand2 = result;
    } else {
        cout << "WARNING:  assignment to something other than a symbol or attribute (" <<
          symbol_pntr(operand1.type_symid()) << ") ignored -- line " << funcstate()->linenum() << "\n";
	cout << "comterp stack:  ";
        print_stack_arg_post_eval(0);
	delete operand2;
    }
    reset_stack();
    push_stack(*operand2);
}

ModAssignFunc::ModAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void ModAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true /* no symbol lookup */));
    if (operand2.is_attribute()) lookup_symval(operand2);
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
        if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*op1val);
	push_stack(operand2);
	ModFunc modfunc(comterp());
	modfunc.funcid(symbol_add("mod"));
	modfunc.exec(2,0);
	ComValue result(pop_stack());
        op1val->assignval(result);
	push_stack(result);
    }

}

MpyAssignFunc::MpyAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void MpyAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true /* no symbol lookup */));
    if (operand2.is_attribute()) lookup_symval(operand2);
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
	if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*op1val);
	push_stack(operand2);
	MpyFunc mpyfunc(comterp());
	mpyfunc.funcid(symbol_add("mpy"));
	mpyfunc.exec(2,0);
	ComValue result(pop_stack());
        op1val->assignval(result);
	push_stack(result);
    }

}

AddAssignFunc::AddAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}

void AddAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true /* no symbol lookup */));
    if (operand2.is_attribute()) lookup_symval(operand2);
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
        if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*op1val);
	push_stack(operand2);
	AddFunc addfunc(comterp());
	addfunc.funcid(symbol_add("add"));
	addfunc.exec(2,0);
	ComValue result(pop_stack());
        op1val->assignval(result);
	push_stack(result);
    }

}

SubAssignFunc::SubAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void SubAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true /* no symbol lookup */));
    if (operand2.is_attribute()) lookup_symval(operand2);
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
        if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*op1val);
	push_stack(operand2);
	SubFunc subfunc(comterp());
	subfunc.funcid(symbol_add("sub"));
	subfunc.exec(2,0);
	ComValue result(pop_stack());
        op1val->assignval(result);
	push_stack(result);
    }

}

DivAssignFunc::DivAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void DivAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true /* no symbol lookup */));
    if (operand2.is_attribute()) lookup_symval(operand2);
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
	AttributeValue* op1val = comterp()->lookup_symval(&operand1);
	if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*op1val);
	push_stack(operand2);
	DivFunc divfunc(comterp());
	divfunc.funcid(symbol_add("div"));
	divfunc.exec(2,0);
	ComValue result(pop_stack());
        op1val->assignval(result);
	push_stack(result);
    }

}

IncrFunc::IncrFunc(ComTerp* comterp) : AssignFunc(comterp) {
}

void IncrFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
	if (!op1val) 
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*op1val);
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    AddFunc addfunc(comterp());
	    addfunc.funcid(symbol_add("add"));
	    addfunc.exec(2,0);
	    ComValue result(pop_stack());
            op1val->assignval(result);
	    push_stack(result);
	}
    } else 
        push_stack(ComValue::nullval());

}

IncrAfterFunc::IncrAfterFunc(ComTerp* comterp) : AssignFunc(comterp) {
}

void IncrAfterFunc::execute() {
    ComValue operand1(stack_arg(0, true));

    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
	if (!op1val)
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*op1val);
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    AddFunc addfunc(comterp());
	    addfunc.funcid(symbol_add("add"));
	    addfunc.exec(2,0);
	    ComValue result(pop_stack());
	    push_stack(*op1val);
            op1val->assignval(result);
	}
    } else 
        push_stack(ComValue::nullval());
}

DecrFunc::DecrFunc(ComTerp* comterp) : AssignFunc(comterp) {
}

void DecrFunc::execute() {
    ComValue operand1(stack_arg(0,true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
	if (!op1val)
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*op1val);
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    SubFunc subfunc(comterp());
	    subfunc.funcid(symbol_add("sub"));
	    subfunc.exec(2,0);
	    ComValue result(pop_stack());
            op1val->assignval(result);
	    push_stack(result);
	}
    } else 
        push_stack(ComValue::nullval());

}

DecrAfterFunc::DecrAfterFunc(ComTerp* comterp) : AssignFunc(comterp) {
}

void DecrAfterFunc::execute() {
    ComValue operand1(stack_arg(0,true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeValue* op1val = comterp()->lookup_symval(&operand1);
	if (!op1val)
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*op1val);
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    SubFunc subfunc(comterp());
	    subfunc.funcid(symbol_add("sub"));
	    subfunc.exec(2,0);
	    ComValue result(pop_stack());
	    push_stack(*op1val);
            op1val->assignval(result);
	}
    } else 
        push_stack(ComValue::nullval());

}
