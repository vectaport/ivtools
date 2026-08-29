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
        // if lhs is a global() or local() call, set lhs_assign flag on its
        // ComValue so the scope command can distinguish lhs from rhs context.
        // Same for at() (including via the @ operator, lst@N=val, #318) --
        // ListAtFunc checks its own lhs_assign() (symbolfunc.c's
        // GlobalFunc/LocalFunc do the same) to tell "lst@0=val" apart from
        // an ordinary read.  Only matters for the ArrayType (plain list)
        // case -- an AttributeList read always returns a detached, single-
        // entry copy (never a live handle back into the source list, on
        // purpose -- see ListAtFunc's own comment, listfunc.c), so
        // al@N=val can't reach this far as a writable lvalue at all; it
        // falls through to the WARNING branch below like any other
        // non-writable assignment target.
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
	/* global() lvalue must be tested BEFORE the func-frame branch:
	   the whole point of global(x)=val is to escape the frame.
	   The old-value cleanup must read the globaltable DIRECTLY
	   (globalvalue()), not via lookup_symval: the lvalue symbol
	   carries bquote (the anti-resolution shield), and lookup_symval
	   returns nil for bquoted symbols -- so the old cleanup never
	   fired and every reassignment stacked a fresh table entry over
	   the orphaned old one (~95 bytes leaked per global(x)=val,
	   observable as global(:cnt) growth). */
	if (operand1.global_flag()) {
	    ComValue* oldval = comterp()->globalvalue(operand1.symbol_val());
	    if (oldval) {
	      comterp()->globaltable()->remove(operand1.symbol_val());
	      delete oldval;
	    }
	    comterp()->globaltable()->insert(operand1.symbol_val(), operand2);
	} else if (operand1.local_flag()) {
	    /* local() lvalue: write the default (per-instance) symbol table,
	       skipping any func frame -- outside a func this is what bare
	       assignment does anyway; inside one it is the session-scope
	       escape (global() being the process-scope escape). */
	    ComValue* oldval = comterp()->localvalue(operand1.symbol_val());
	    if (oldval) {
	      comterp()->localtable()->remove(operand1.symbol_val());
	      delete oldval;
	    }
	    comterp()->localtable()->insert(operand1.symbol_val(), operand2);
	} else if (attrlist) {
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
      attr->Value(operand2);
    } else if (operand1.is_array() && operand1.lhs_assign()) {
      /* #318 (@ operator): lst@N=val.  ListAtFunc (listfunc.c), seeing its
	 own lhs_assign() flag set and an ArrayType before-part, handed back
	 a [list, idx] pair instead of performing a read (lhs_assign() still
	 set on the pair itself, so this branch can tell it apart from an
	 ordinary array-valued read reaching here some other way).  Complete
	 the write by re-driving at() itself with a real :set keyword --
	 pushed the same way comterp.c's own EvalFunc call does (value, then
	 a KeywordType marker carrying the keyword's symid+narg) -- so the
	 mutation goes through at()'s single, already-tested :set code path
	 instead of a second, hand-rolled one here. */
      AttributeValueList* pair = operand1.array_val();
      static int set_symid = symbol_add("set");
      push_stack(*pair->Get(0));
      push_stack(*pair->Get(1));
      push_stack(*operand2);
      ComValue setkey(set_symid, 1);
      push_stack(setkey);
      ListAtFunc atfunc(comterp());
      atfunc.funcid(symbol_add("at"));
      /* narg counts non-keyword args INCLUDING the value that follows a
	 keyword (see the ~~ spread-operator rebuild, comterp.c ~316) --
	 4 physical pushes above (list, idx, :set's value, :set's marker)
	 means narg=3 (list, idx, and the :set value it counts) and
	 nkey=1 (just the marker), not narg=2. */
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
