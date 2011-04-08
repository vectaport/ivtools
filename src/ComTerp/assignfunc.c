/*
 * Copyright (c) 1997 Vectaport Inc.
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

AssignFunc::AssignFunc(ComTerp* comterp) : ComFunc(comterp) {
}


void AssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    ComValue* operand2 = new ComValue(stack_arg(1));
    reset_stack();
    if (operand1.type() == ComValue::SymbolType) {
        AttributeList* attrlist = comterp()->get_attributes();
	if (attrlist) {
	    Resource::ref(attrlist);
	    Attribute* attr = new Attribute(operand1.symbol_val(), 
					    new AttributeValue(*operand2));
	    attrlist->add_attribute(attr);
	    Unref(attrlist);
	} else 
	    comterp()->localtable()->insert(operand1.symbol_val(), operand2);
    } else {
        cerr << "assignment to something other than a symbol ignored\n";
    }
    push_stack(*operand2);
}

ModAssignFunc::ModAssignFunc(ComTerp* comterp) : ModFunc(comterp) {
}


void ModAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    ComValue operand2(stack_arg(1));
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
	ModFunc::execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

MpyAssignFunc::MpyAssignFunc(ComTerp* comterp) : MpyFunc(comterp) {
}


void MpyAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    ComValue operand2(stack_arg(1));
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
	MpyFunc::execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

AddAssignFunc::AddAssignFunc(ComTerp* comterp) : AddFunc(comterp) {
}


void AddAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    ComValue operand2(stack_arg(1));
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
	AddFunc::execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

SubAssignFunc::SubAssignFunc(ComTerp* comterp) : SubFunc(comterp) {
}


void SubAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    ComValue operand2(stack_arg(1));
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
	SubFunc::execute();
        pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

DivAssignFunc::DivAssignFunc(ComTerp* comterp) : DivFunc(comterp) {
}


void DivAssignFunc::execute() {
    ComValue operand1(stack_arg(0, true));
    ComValue operand2(stack_arg(1));
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
	DivFunc::execute();
	pop_funcstate();
	ComValue* result = new ComValue(pop_stack());
        _comterp->localtable()->insert(operand1.symbol_val(), result);
	push_stack(*result);
    }

}

IncrFunc::IncrFunc(ComTerp* comterp) : AddFunc(comterp) {
}

void IncrFunc::execute() {
    ComValue operand1(stack_arg(0, true));
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
  	    AddFunc::execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*result);
	}
    } else 
        push_stack(ComValue::nullval());

}

IncrAfterFunc::IncrAfterFunc(ComTerp* comterp) : AddFunc(comterp) {
}

void IncrAfterFunc::execute() {
    ComValue operand1(stack_arg(0, true));
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
  	    AddFunc::execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*(ComValue*)op1val);
	}
    } else 
        push_stack(ComValue::nullval());

}

DecrFunc::DecrFunc(ComTerp* comterp) : SubFunc(comterp) {
}

void DecrFunc::execute() {
    ComValue operand1(stack_arg(0,true));
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
  	    SubFunc::execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*result);
	}
    } else 
        push_stack(ComValue::nullval());

}

DecrAfterFunc::DecrAfterFunc(ComTerp* comterp) : SubFunc(comterp) {
}

void DecrAfterFunc::execute() {
    ComValue operand1(stack_arg(0,true));
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
  	    SubFunc::execute();
	    pop_funcstate();
	    ComValue* result = new ComValue(pop_stack());
            _comterp->localtable()->insert(operand1.symbol_val(), result);
	    push_stack(*(ComValue*)op1val);
	}
    } else 
        push_stack(ComValue::nullval());

}

