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
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <InterViews/resource.h>

#include <fstream>
#include <iostream>
using std::cout;
using std::cerr;

#define TITLE "AssignFunc"

/*****************************************************************************/

int AssignFunc::_symid = -1;

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
        // Same for a dot expression (lst.N=val, #318) -- DotFunc checks its
        // own lhs_assign() (confirmed live: startval here is CommandType
        // funcname=dot for any X.Y=val shape) to tell "l.0" apart from an
        // ordinary read, and -- for a plain-list before-part with a numeric
        // rhs specifically -- hands back a [list, idx] pair instead of
        // performing a read, so this function can perform the write
        // itself below (see that branch for why a pair, not a spare
        // ComValue field).  Harmless no-op for every other dot shape
        // (al.b=val already works via the Attribute branch below and
        // never consults this flag; al.N=val is deliberately excluded by
        // DotFunc too --
        // a numeric attrlist walk is read-only reflection, not an address).
        static int global_symid = symbol_add("global");
        static int local_symid = symbol_add("local");
        static int dot_symid = symbol_add("dot");
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
                func->funcid() == dot_symid)
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
      /* #318 write: lst.N=val.  DotFunc, seeing its own lhs_assign() flag
	 set (above) and a plain-list before-part with a numeric rhs,
	 doesn't perform a read -- it hands back a tiny 2-element
	 [list, idx] ArrayValueList (lhs_assign() still set, so this
	 branch can tell it apart from an ordinary array-valued read
	 reaching here some other way) instead of stashing the index in a
	 spare ComValue field -- nids() was tried first and confirmed live
	 NOT to survive the round trip for an ArrayType value. A plain
	 list has no per-element Attribute the way a named attrlist key
	 does (the branch just above), so there's nothing to write through
	 directly.

	 Mutating via AttributeValueList::Set() directly -- the exact call
	 ListAtFunc's own :set branch makes internally (listfunc.c) --
	 rather than driving ListAtFunc's exec() with a manually-built
	 :set keyword pair from here: confirmed live that a keyword pushed
	 the normal way (value, then its KeywordType marker on top, two
	 physical stack slots) doesn't round-trip cleanly through a
	 manually-invoked exec() -- ListAtFunc's own reset_stack() decrements
	 by nargs()+nkeys() as a plain *count* (2+1=3), one short of the 4
	 slots actually pushed, leaving a stray value that corrupted the
	 stack for every statement after.  Calling the same underlying
	 mutation directly sidesteps that keyword-accounting mismatch
	 entirely -- same effect, no exec() stack-balance risk. */
      AttributeValueList* pair = operand1.array_val();
      AttributeValueList* avl = pair->Get(0)->array_val();
      int idx = pair->Get(1)->int_val();
      AttributeValue* oldv = avl->Set(idx, new AttributeValue(*operand2));
      delete oldv;
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
	    subfunc.exec(2,0);
	    ComValue result(pop_stack());
	    push_stack(*op1val);
            op1val->assignval(result);
	}
    } else 
        push_stack(ComValue::nullval());

}
