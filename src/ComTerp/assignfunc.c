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
#include <ComTerp/postfunc.h>
#include <ComTerp/strmfunc.h>
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
    boolean from_each = false;
    ComValue operand1(stack_arg(0, true));
    if (operand1.is_command() && stack_arg_post_eval_size(0)==1) {
        cout << "WARNING:  assignment to command \"" << operand1.command_name() << "\" without args not allowed -- line " << funcstate()->linenum() << "\n";
	reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    if (operand1.type() != ComValue::SymbolType) {
        // if lhs is global()/local()/at() (including lst@N=val),
        // set lhs_assign on its ComValue to distinguish lhs from rhs context
        static int global_symid = symbol_add("global");
        static int local_symid = symbol_add("local");
        static int temp_symid = symbol_add("temp");
        static int at_symid = symbol_add("at");
        static int each_symid = symbol_add("each");
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
                func->funcid() == temp_symid || func->funcid() == at_symid)
                startval.lhs_assign(1);
            else if (func->funcid() == each_symid) {
                /* **lhs: flag each()'s own token so it hands back its
                   stream instead of draining it itself (EachFunc), then
                   flag the at()/global/local/temp token each() wraps --
                   its own root token sits immediately before each()'s,
                   the same postfix layout NextFunc's lhs-walk relies on --
                   the same way as an unwrapped lhs just above. */
                startval.lhs_assign(1);
                from_each = true;
                ComValue& innerval = comterp()->pfcomvals()[startidx - 1];
                if (innerval.is_type(ComValue::CommandType)) {
                    ComFunc* innerfunc = (ComFunc*)innerval.obj_val();
                    if (innerfunc->funcid() == global_symid || innerfunc->funcid() == local_symid ||
                        innerfunc->funcid() == temp_symid || innerfunc->funcid() == at_symid)
                        innerval.lhs_assign(1);
                }
            }
        }
        operand1 = stack_arg_post_eval(0, true /* no symbol or attribute lookup */);
    }
    int rhs_wrapper = AttributeValue::NoWrapper;
    ComValue* operand2 = new ComValue(stack_arg_post_eval(1, true /* no symbol or attribute lookup */,
							   ComValue::nullval(), &rhs_wrapper));
#ifdef POSTEVAL_EXPERIMENT
    if (operand2->is_attribute() || operand2->is_symbol()) lookup_symval(*operand2);
#else
    if (operand2->is_attribute()) lookup_symval(*operand2);
#endif
    /* clear any kwoverride() tag an operator's copy-constructed result
       carried over from an operand -- an assignment's RHS is always fresh */
    operand2->kwoverride(0);
    /* global/local/temp/attrlist/attribute branches hand this pointer to a
       persistent structure that deletes it later. */
    /* every other branch must delete operand2 itself once done, or its ref
       on whatever Resource it holds is never released. */
    boolean operand2_owned = false;
    if (operand1.type() == ComValue::SymbolType) {
        AttributeList* attrlist = comterp()->get_attributes();
	/* global() lvalue tested before func-frame branch; old-value cleanup reads
	   globaltable directly via globalvalue(), not lookup_symval() (nil for bquoted symbols) */
	if (operand1.global_flag()) {
	    ComValue* oldval = comterp()->globalvalue(operand1.symbol_val());
	    if (oldval) {
	      comterp()->globaltable()->remove(operand1.symbol_val());
	      delete oldval;
	    }
	    comterp()->globaltable()->insert(operand1.symbol_val(), operand2);
	    operand2_owned = true;
	} else if (operand1.local_flag()) {
	    /* local() lvalue: write the default (per-instance) symbol table,
	       skipping any func frame -- the session-scope escape */
	    ComValue* oldval = comterp()->localvalue(operand1.symbol_val());
	    if (oldval) {
	      comterp()->localtable()->remove(operand1.symbol_val());
	      delete oldval;
	    }
	    comterp()->localtable()->insert(operand1.symbol_val(), operand2);
	    operand2_owned = true;
	} else if (operand1.temp_flag()) {
	    /* temp() lvalue: write the call's own temp frame -- add_attribute()
	       replaces by symid, so a later temp(x)=val on x just updates it. */
	    AttributeList* tempframe = comterp()->get_tempframe();
	    if (!tempframe) {
	      cout << "WARNING:  temp() used outside any func call -- line "
		   << funcstate()->linenum() << "\n";
	      delete operand2;
	      reset_stack();
	      push_stack(ComValue::nullval());
	      return;
	    }
	    Attribute* attr = new Attribute(operand1.symbol_val(), operand2);
	    tempframe->add_attribute(attr);
	    operand2_owned = true;
	} else {
	    /* targets_attrlist mirrors write_funcscope_symval()'s own
	       routing: an existing temp() name wins, else attrlist is the
	       target when declared there or when no temp frame exists to
	       divert a new name into. */
	    /* eval(:alist) binds a receiver without allocating a temp
	       frame, so a brand-new name there falls straight to attrlist. */
	    AttributeList* tempframe = comterp()->get_tempframe();
	    boolean targets_attrlist = attrlist &&
	      !(tempframe && tempframe->find(operand1.symbol_val())) &&
	      (attrlist->GetAttr(operand1.symbol_val()) || !tempframe);
	    if (targets_attrlist && value_contains_container(*operand2, (void*)attrlist, true)) {
	      fprintf(stderr, "WARNING: refusing to insert an attrlist into itself -- line %d\n",
		      funcstate()->linenum());
	      delete operand2;
	      reset_stack();
	      push_stack(ComValue::nullval());
	      return;
	    }
	    /* a brand-new name stays call-local only when a temp frame
	       exists to hold it -- see SLICES.md. */
	    comterp()->write_funcscope_symval(operand1.symbol_val(), operand2);
	    operand2_owned = true;
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
      /* Same home stamp as AttrListFunc::stamp_home_attrs, for a func
	 written onto an existing attrlist's field; same one-time rule. */
      if (owner && operand2->is_object(FuncObj::class_symid())) {
	FuncObj* fo = (FuncObj*)operand2->obj_val();
	if (!fo->home_attrs().is_object(AttributeList::class_symid())) {
	  ComValue homeval(AttributeList::class_symid(), owner);
	  fo->home_attrs(homeval);
	}
      }
      operand2_owned = true;
    } else if (operand1.is_array() && operand1.lhs_assign()) {
      /* the @ operator: lst@N=val -- ListAtFunc handed back a [list, idx] pair,
         so complete the write by re-driving at() with a real :set keyword */
      ComValue result(NextFunc::write_at_pair(comterp(), operand1, *operand2));
      *operand2 = result;
    } else if (operand1.is_stream() && operand1.lhs_assign()) {
      if (from_each) {
	/* **lst@lo:hi=val(s): batch-drain now -- zip-writes operand2
	   alongside the streamed index, or broadcasts it if operand2 isn't
	   itself a stream -- see NextFunc::zip_assign_stream (strmfunc.c),
	   the shared drain-and-write loop next()'s own zipper also uses. */
	boolean rhs_is_stream = operand2->is_stream();
	ComValue idxstream(operand1);
	int count = NextFunc::zip_assign_stream(comterp(), idxstream, operand2, rhs_is_stream,
						 funcstate()->linenum());
	delete operand2;
	reset_stack();
	if (count < 0) {
	  push_stack(ComValue::nullval());
	  return;
	}
	ComValue retval(count, ComValue::IntType);
	push_stack(retval);
	/* count, not the written values -- list() already exists for
	   collecting those, and building both would double the work */
	comterp()->stack_top().wrapper(AttributeValue::BracketWrapper);
	return;
      }
      /* lst@lo:hi=val(s), no ** -- stream builds stream: defer into a lazy
	 wrapper (AssignAtNextFunc) that writes one element per pull, rather
	 than draining here, matching a plain var's own default. */
      static AssignAtNextFunc* aanfunc = nil;
      if (!aanfunc) {
	aanfunc = new AssignAtNextFunc(comterp());
	aanfunc->funcid(symbol_add("assignatnext"));
      }
      AttributeValueList* avl = new AttributeValueList();
      avl->Append(new AttributeValue(operand1));
      avl->Append(new AttributeValue(*operand2));
      delete operand2;
      ComValue stream(aanfunc, avl);
      stream.stream_mode(STREAM_INTERNAL);
      reset_stack();
      push_stack(stream);
      return;
    } else if (operand1.unknown() && operand1.lhs_assign()) {
      /* a locked attrlist's dot lookup found no such entry -- the write
         never happens, and the expression reports nil, not the RHS. */
      delete operand2;
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    } else {
        cout << "WARNING:  assignment to something other than a symbol or attribute (" <<
          symbol_pntr(operand1.type_symid()) << ") ignored -- line " << funcstate()->linenum() << "\n";
	cout << "comterp stack:  ";
        print_stack_arg_post_eval(0);
	delete operand2;
	operand2_owned = true;
    }
    reset_stack();
    push_stack(*operand2);
    /* carries a bracketed count (e.g. next()/feed()'s each()-forced drain)
       through assignment, so s=next(ring **0..3) still echoes [4]. */
    if (rhs_wrapper != AttributeValue::NoWrapper) comterp()->stack_top().wrapper(rhs_wrapper);
    if (!operand2_owned) delete operand2;
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
	result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	    result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	    result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	    result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
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
	    result.kwoverride(0);   /* an operator's result may inherit a tag from its operand */
	    push_stack(*op1val);
            op1val->assignval(result);
	}
    } else 
        push_stack(ComValue::nullval());

}
