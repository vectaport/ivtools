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

#define TITLE "AssignFunc"

/*****************************************************************************/

int AssignFunc::_symid = -1;

AssignFunc::AssignFunc(ComTerp* comterp) : ComFunc(comterp) {
}


void AssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue* operand2 = new ComValue(stack_arg_post_eval(1, true));
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeList* attrlist = comterp()->get_attributes();
	if (attrlist) {
	    Resource::ref(attrlist);
	    Attribute* attr = new Attribute(operand1.symbol_val(), 
					    operand2);
	    attrlist->add_attribute(attr);
	    Unref(attrlist);
	} else 
	    comterp()->localtable()->insert(operand1.symbol_val(), operand2);
    } else if (operand1.is_object(Attribute::class_symid())) {
      Attribute* attr = (Attribute*)operand1.obj_val();
      attr->Value(operand2);
    } else {
        cerr << "assignment to something other than a symbol ignored\n";
	// should say something about assignment from something other than a symbol as well.
    }
    push_stack(*operand2);
}

ModAssignFunc::ModAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void ModAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true));
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*(ComValue*)op1val);
	delete (ComValue*)op1val;
	push_stack(operand2);
        push_funcstate(2,0);
	static ModFunc* subfunc = new ModFunc(comterp());
	subfunc->execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

MpyAssignFunc::MpyAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void MpyAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true));
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*(ComValue*)op1val);
	delete (ComValue*)op1val;
	push_stack(operand2);
        push_funcstate(2,0);
	static MpyFunc* subfunc = new MpyFunc(comterp());
	subfunc->execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

AddAssignFunc::AddAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void AddAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true));
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*(ComValue*)op1val);
	delete (ComValue*)op1val;
	push_stack(operand2);
        push_funcstate(2,0);
	static AddFunc* subfunc = new AddFunc(comterp());
	subfunc->execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

SubAssignFunc::SubAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void SubAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true));
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*(ComValue*)op1val);
	delete (ComValue*)op1val;
	push_stack(operand2);
        push_funcstate(2,0);
	static SubFunc* subfunc = new SubFunc(comterp());
	subfunc->execute();
        pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

DivAssignFunc::DivAssignFunc(ComTerp* comterp) : AssignFunc(comterp) {
}


void DivAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    if (operand1.type() != ComValue::SymbolType) {
      operand1.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
    }
    ComValue operand2(stack_arg_post_eval(1, true));
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val) {
	    push_stack(ComValue::nullval());
	    return;
	}
	push_stack(*(ComValue*)op1val);
	delete (ComValue*)op1val;
	push_stack(operand2);
        push_funcstate(2,0);
	static DivFunc* subfunc = new DivFunc(comterp());
	subfunc->execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
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
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val) 
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*(ComValue*)op1val);
	    delete (ComValue*)op1val;
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    push_funcstate(2,0);
	    static AddFunc* subfunc = new AddFunc(comterp());
	    subfunc->execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*result);
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
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val)
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*(ComValue*)op1val);
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    push_funcstate(2,0);
	    static AddFunc* subfunc = new AddFunc(comterp());
	    subfunc->execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*(ComValue*)op1val);
	    delete (ComValue*)op1val;
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
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val)
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*(ComValue*)op1val);
	    delete (ComValue*)op1val;
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    push_funcstate(2,0);
	    static SubFunc* subfunc = new SubFunc(comterp());
	    subfunc->execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*result);
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
        void* op1val = nil;
        _comterp->localtable()->find_and_remove(op1val, operand1.symbol_val());
	if (!op1val)
	    push_stack(ComValue::nullval());
	else {
	    push_stack(*(ComValue*)op1val);
	    ComValue one;
	    one.type(ComValue::IntType);
	    one.int_ref() = 1;
	    push_stack(one);
	    push_funcstate(2,0);
	    static SubFunc* subfunc = new SubFunc(comterp());
	    subfunc->execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*(ComValue*)op1val);
	    delete (ComValue*)op1val;
	}
    } else 
        push_stack(ComValue::nullval());

}

