/*
 * Copyright (c) 2000 IET Inc.
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

/*
 * ComFunc is a simple action callback pointed to by a ComCom
 * (or possibly any other external function binding mechanism)
 */

#if !defined(_comfunc_h)
#define _comfunc_h

#include <stdlib.h>
#include <OS/types.h>
#include <ComTerp/comvalue.h>
#include <Attribute/classid.h>
#include <Unidraw/Components/component.h>

class AttributeList;
class ComFuncState;
class Component;
class ComTerpServ;


//: command base class for extending ComTerp.
// class whose derived classes get constructed and added to a 
// ComTerp command interpreter, one per command to be supported.
class ComFunc {
public:
    ComFunc(ComTerp*);
    virtual ~ComFunc() {}

    virtual void execute() = 0;
    // method that needs to be filled in, that will take ComValue arguments
    // off the stack, then compute and push a ComValue result on the stack.

    void exec(int nargs, int nkeys, int pedepth=0, int command_symid=0);
    // invokes push_funcstate, then plain execute, then pop_funcstate.
    // for use from the body of regular execute methods.

    int& nargs();
    // number of white space separated arguments inside parentheses
    // that are not keywords (keywords indicated by a ':' prefix 
    // on a symbol).
    int& nkeys();
    // number of arguments inside parentheses that are keywords (keywords 
    // indicated by a ':' prefix  on a symbol).  Arguments that follow
    // keywords are counted by nargs().  That's why there is a need for
    // nargsfixed() and nargstotal() methods, to distinguish between
    // arguments associated with fixed arguments (the ones that
    // precede the keyword arguments), and the ones enmeshed with the
    // keywords.  

    int nargsfixed() {return nargs() - nargskey();}
    // number of arguments prior to any keyword.  The ones that require a 
    // fixed order to determine their meaning (unlike the arguments that 
    // follow keywords, which can be in any order).
    int nargstotal() {return nargs() + nkeys();}
    // nargs()+nkeys()
    int nargskey();
    // number of arguments follow keywords, assumed to be less than
    // or equal to nkeys()  (this requirement could be relaxed).
    int nargspost();
    // number of arguments to a post-evaluating command 
    // that represent commands and arguments ready to be evaluated
    // by the post-eval methods: stack_arg_post_eval() and 
    // stack_key_post_eval().

    ComFuncState* funcstate();
    // current ComFuncState for use of current ComFunc.
    void push_funcstate(int nargs, int nkeys, int pedepth=0, 
                        int command_symid=0);
    // push new ComFuncState on a stack of them.
    void pop_funcstate();
    // pop the top one off the ComFuncState stack.

    ComTerp* comterp() { return _comterp; }
    // return ComTerp this ComFunc is associated with.
    ComTerpServ* comterpserv();
    // return ComTerpServ this ComFunc is associated with.
    void comterpserv( ComTerpServ* serv) { _comterp = (ComTerp*)serv; }


    ComValue pop_stack(); 
    // pop top off the stack.
    ComValue pop_symbol();
    // pop top off the stack preserving symbol ids if ComValue is a symbol type.
    void push_stack(ComValue&);
    // push ComValue onto the stack.
    void push_stack(AttributeValue&);
    // push AttributeValue onto the stack, converting to a ComValue
    // in the process.
    void reset_stack();
    // reset the stack to its state before this ComFunc execute() method
    // was called.  Needs to be called once and only once in each
    // derived execute() method.  Save it to last if you're using references
    // to ComValue objects directly on the stack.  Otherwise, call it as
    // soon as all the arguments have been loaded into local copies.
    

    ComValue& stack_arg(int n, boolean symbol=false, 
			ComValue& dflt=ComValue::nullval());
    // return the nth argument on the stack for this ComFunc execute() call.

    ComValue& stack_key(int id, boolean symbol=false, 
			ComValue& dflt=ComValue::trueval(), 
			boolean use_dflt_for_no_key=false
			/* the antonym would be use_dflt_for_no_arg */);
    // return the value of an argument that follows a keyword,
    // optionally return the value of 'dflt' if no argument follows
    // the keyword.  If 'use_dflt_for_no_key' is true, 'dflt' gets returned
    // as the value when a keyword is not found.

    ComValue& stack_dotname(int n);
    // unused method to get at a dotted list of names, i.e. a.b.c

    ComValue stack_arg_post_eval(int n, boolean symbol=false, 
				 ComValue& dflt=ComValue::nullval());
    // evaluate the nth argument for this post-evaluating ComFunc.

    ComValue stack_key_post_eval(int id, boolean symbol=false, 
				  ComValue& dflt=ComValue::trueval(), 
				  boolean use_dflt_for_no_key=false);
    // evaluate the argument following a keyword for this post-evaluating ComFunc.
    // Optionally return the value of 'dflt' if no argument follows
    // the keyword.  If 'use_dflt_for_no_key' is true, 'dflt' gets returned
    // as the value when a keyword is not found.

    AttributeList* stack_keys(boolean symbol = false, 
			      AttributeValue& dflt=ComValue::trueval());
    // return newly-constructed AttributeList (which needs referencing)
    // that contains a copy of each keyword/value pair in the arguments
    // to the invocation of this ComFunc.  'dflt' is used whenever a 
    // keyword has no matching argument.

    void funcid(int id) { _funcid = id; }
    // set symbol id of name for func
    int funcid() const { return _funcid; }
    // get symbol id of name for func

    ComValue& lookup_symval(ComValue&);
    // lookup variable value given a symbol ComValue
    ComValue& lookup_symval(int symid);
    // lookup variable value given a symbol id.
    void assign_symval(int id, ComValue*);

    virtual boolean post_eval() { return false; }
    virtual const char* docstring() { return "%s: no docstring method defined"; }
    static int bintest(const char* name);
    static boolean bincheck(const char* name);
    
    friend ostream& operator << (ostream& s, const ComFunc&);
    // print contents to ostream, brief or not depending on
    // associated ComTerp brief flag.

    Component* context() { return _context; }
    void context(Component* comp) { _context = comp; }

protected:

    int& npops();
    // number of calls to pop_stack() since execute method called.
    int& pedepth();
    // depth of being embedded in blocks of post-evaluated ccommands 
    // (commands whose arguments are not pre-evaluated for them).


    ComValue& stack_arg_post(int n, boolean symbol=false, 
			     ComValue& dflt=ComValue::nullval());
    // find the ComValue object in the unevaluated input arguments 
    // of a post-eval ComFunc that represents the start of the
    // code for the nth argument (prior to any keywords).

    ComValue& stack_key_post(int id, boolean symbol=false, 
			     ComValue& dflt=ComValue::trueval(), 
			     boolean use_dflt_for_no_key=false);
    // find the ComValue object in the unevaluated input arguments 
    // of a post-eval ComFunc that represents the start of the
    // code for the argument that follows a keyword.  If no argument
    // follows keyword, 'dflt' is returned, unless 'use_dflt_for_no_key'
    // is true, when 'dflt' gets returned when no matching keyword is found.

    boolean skip_key_on_stack(int& stackptr, int& arglen);
    // skip a keyword going down the stack.
    boolean skip_arg_on_stack(int& stackptr, int& arglen);
    // skip an argument going down the stack.
    boolean skip_key_in_expr(int& topptr, int& arglen);
    // skip a keyword in a buffer of unevaluated ComValue objects,
    // decrementing an index into a buffer set up to hold the 
    // currently interpreting expression.
    boolean skip_arg_in_expr(int& topptr, int& arglen);
    // skip an argument in unevaluated ComValue objects, 
    // decrementing an index into a buffer set up to hold the
    // currently interpreting expression.

    ComTerp* _comterp;
    int _funcid;
    Component* _context;

    CLASS_SYMID("ComFunc");
};

//: state object for holding invocation specific data about a ComFunc.
// object that holds the state of a ComFunc used in a particular context,
// which allows for nested and recursive use of a singular ComFunc.
class ComFuncState {
public:
  ComFuncState(int nargs, int nkeys, int pedepth=0, int command_symid=0);
  // initialize with number of arguments (including arguments following
  // keywords), number of keywords, an option post-eval depth (nesting
  // within blocks of post-evaluation commands), and an optional 
  // symbol id associated with the ComFunc.
  ComFuncState(const ComFuncState&);
  // copy constructor.

  int& nargs() {return _nargs;}
  // number of arguments (including arguments following keywords).
  int& nkeys() {return _nkeys;}
  // number of keywords, ':' prefixed symbols in a list of arguments.
  int& npops() { return _npops;}
  // number of pops since the execute method called.
  int& nargskey() { return _nargskey; }
  // number of arguments following keywords.  Assumption of nargskey()<=nkey().
  int& nargspost() { return _nargspost; }
  // number of unevaluated ComValue objects input to a post-evaluation ComFunc.
  int& pedepth() { return _pedepth; }
  // post-evaluation depth: the nesting within blocks of post-evaluation commands,
  // within blocks of conditionally executing control commands.
  int& command_symid() { return _command_symid; }
  // symbol id associated with the ComFunc.
  
  friend ostream& operator << (ostream& s, const ComFuncState&);
  // print contents to ostream, brief or not depending on
  // associated ComTerp brief flag.

protected:

  int _nargs;
  int _nkeys;
  int _npops;
  int _nargskey;
  int _nargspost;
  int _pedepth;
  int _command_symid;
};
#endif /* !defined(_comfunc_h) */
