/*
 * Copyright (c) IET Inc.
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
 * ComTerp is a command interpreter derived from the Parser
 */

#if !defined(_comterp_h)
#define _comterp_h

#include <ComTerp/parser.h>

#include <OS/table.h>
declareTable(ComValueTable,int,void*)


class AttributeList;
class AttributeValue;
class ComFunc;
class ComFuncState;
class ComTerpState;
class ComValue;
#include <iosfwd>

class ComterpHandler;

//: extendable command interpreter.
// ComTerp is an extendable command interpreter with a simple C-like expression
// syntax and support for commands with fixed-location and keyword-prefixed 
// arguments.  The underlying architecture of this interpreter is patterned
// after Fischer and LeBlanc's "Crafting a Compiler with C", with their pipeline
// of scanner --> parser --> code_conversion --> code_generation retrofitted into 
// one of scanner --> parser --> code_conversion --> interpreter.
// <p>
// You use a ComTerp by first constructing one with a default set of commands
// and operators, then add any commands (derived ComFunc objects) needed for a
// particular application, then start it running on either stdin or 
// some alternate input file described by arguments to the constructor.
// To embed a ComTerp in another application you'll probably want to use
// a ComTerpServ, which has extensions to ComTerp for passing input and output
// strings from/to the interpreter by buffers.
class ComTerp : public Parser {
public:
    ComTerp();
    // construct with default set of ComFunc objects.
    ComTerp(const char* path);
    // construct and run on commands read from 'path'.
    ComTerp(void*, char*(*)(char*,int,void*), int(*)(void*), int(*)(void*));
    // construct with IO system based on a void* pointer and three function
    // pointers to receive that void* as their first argument, with
    // signatures that match fgets, feof, and ferror.
    ~ComTerp();

    void init();
    // initialize memory for the regular stack, the ComFuncState stack,
    // and the symbol tables.

    boolean read_expr();
    // read expression from the input, return true if all ok.
    boolean eof();
    // return true when end-of-file found on the input.

    virtual int eval_expr(boolean nested=false);
    // evaluate topmost expression on the stack, with a flag to
    // indicate if this call to eval_expr() is nested inside
    // another, so that initialization doesn't get repeated.
    virtual int eval_expr(ComValue* pfvals, int npfvals);
    // evaluate postfix expression stored in ComValue objects.
    virtual int post_eval_expr(int tokcnt, int offtop, int pedepth);
    // copy unevaluated expression to the stack and evaluate.

    int print_stack_top() const;
    // print the top of the stack to stdout.
    int print_stack_top(ostream& out) const;
    // print the top of the stack to an ostream.
    int print_stack() const;
    // print the entire stack to stdout.
    int stack_height() { return _stack_top+1; }
    // return current height of the stack.
    boolean brief() const;
    // return brief mode flag.
    void brief(boolean flag) { _brief = flag; }
    // set brief mode flag.

    int add_command(const char* name, ComFunc*, const char* alias = nil);
    // add a derived ComFunc to be known by 'name'.
    void list_commands(ostream& out, boolean sorted = false);
    // print an optionally sorted list of commands to an ostream.
    int* get_commands(int &ncommands, boolean sorted = false);
    // return an optionally sorted list of command names.

    ComValue& pop_stack(boolean lookupsym=true);
    // return a reference (on the stack) to what was the top of the stack,
    // if 'lookupsym' is false, don't look up ComValue objects in 
    // the local or global symbol table to replace a symbol, just
    // return the symbol itself.
    ComValue& lookup_symval(ComValue&);
    // look up a ComValue associated with a symbol (specified in the
    // input ComValue) in the local or global symbol tables.
    AttributeValue* lookup_symval(ComValue*);
    // look up a pointer to an AttributeValue associated with a symbol
    // (specified in the input ComValue) in the local or global symbol
    // tables.  Do not alter the input ComValue.
    ComValue& lookup_symval(int symid);
    // look up a ComValue associated with a symbol (specified with a
    // symbol id) in the local or global symbol tables.

    ComValue& stack_top(int n=0);
    // return reference to top of the stack, offset by 'n' (usually negative).
    ComValue& pop_symbol();
    // return a reference (on the stack) to what was the top of the stack,
    void push_stack(ComValue&);
    // copy the state of a ComValue onto the stack, incrementing the
    // reference count of any AttributeValueList, otherwise replicating data.
    void push_stack(AttributeValue&);
    // copy the state of an AttributeValue into a ComValue on the stack, 
    // incrementing the reference count of any AttributeValueList, 
    // otherwise replicating data.
    boolean stack_empty();

    ComValue& expr_top(int n=0);
    // top of currently evaluating expression buffer.

    static ComTerp& instance();
    // instance of ComTerp for global use.

    void quit(boolean quitflag=true);
    // set flag that will cause a quit to happen as soon as possible.
    void quitflag(boolean flag);
    // set flag that will cause a quit to happen as soon as possible.
    boolean quitflag();
    // return value of flag that will cause a quit to happen as soon as possible.
    virtual void exit(int status=0);
    // call _exit().

    virtual int run(boolean one_expr=false, boolean nested=false);
    // run interpreter until end-of-file or quit command, unless 
    // 'one_expr' is true.  Leave 'one_expr' false when using a ComTerpServ.
    // Return Value:  -1 if eof, 0 if normal operation, 1 if 
    // partial expression parsed, 2 if no result computed, 3 if error in parsing.  
    // 'nested' indicates contents of stack should be preserved.

    virtual int runfile(const char* filename);
    // run interpreter on contents of 'filename'.
    void add_defaults();
    // add default commands (ComFunc objects).

    ComValueTable* localtable() const { return _localtable; }
    // local symbol table associated with an individual ComTerp.
    ComValueTable* globaltable() const 
      { if (!_globaltable) _globaltable = new ComValueTable(100); return _globaltable; }
    // global symbol table associated with every ComTerp.
    ComValue* localvalue(int symid);
    // value associated with a symbol id in the local symbol table.
    ComValue* globalvalue(int symid);
    // value associated with a symbol id in the global symbol table.
    ComValue* eithervalue(int symid, boolean globalfirst=false);
    // value associated with a symbol id in either symbol table.

    const char* errmsg() { return _errbuf; }
    // current error message buffer.

    void set_attributes(AttributeList*);
    // set AttributeList to be used as an additional local symbol table.
    AttributeList* get_attributes();
    // return pointer to AttributeList being used as an additional 
    // local symbol table.

    void handler(ComterpHandler* h );
    // set handler for invoking ComFunc execute methods.
    ComterpHandler* handler();
    // return pointer to handler that can read_expressions from
    // a connection and interpret them.

    void disable_prompt();
    // disable '>' prompting sent in response to an unfinished input expression.
    void enable_prompt();
    // enable '>' prompting sent in response to an unfinished input expression.

    unsigned int& pfnum() { return _pfnum; }
    // number of arguments in the input postfix buffer of a tokenized 
    // tree like expression ready to be evaluated, but not yet
    // converted to ComValue objects.

    virtual boolean is_serv() { return false; } 
    // flag to test if ComTerp or ComTerpServ

    void func_for_next_expr(ComFunc* func);
    // set ComFunc to use on subsequent expression
    ComFunc* func_for_next_expr();
    // get ComFunc to use on subsequent expression

    void val_for_next_func(ComValue& val);
    // set ComValue to pass to subequent command
    ComValue& val_for_next_func();
    // get ComValue to pass to subequent command
    void clr_val_for_next_func();
    // clear out ComValue to pass to subequent command

    unsigned int& linenum() { return _linenum; }
    // count of lines processed

    ComTerpState* top_servstate();
    // return pointer to top state on ComTerpServ state stack

    void push_servstate();
    // push ComTerpServ state for later retrieval

    void pop_servstate();
    // pop ComTerpServ state that was saved earlier

    void trace_mode(int mode) { _trace_mode = mode; }
    // set trace mode

    int trace_mode() { return _trace_mode; }
    // return trace mode

    int& npause() { return _npause; }
    // return (reference to) number of pauses

    int& stepflag() { return _stepflag; }
    // return flag that controls stepwise execution

protected:
    void incr_stack();
    void incr_stack(int n);
    void decr_stack(int n=1);

    boolean skip_func(ComValue* topval, int& offset, int offlimit);
    boolean skip_key(ComValue* topval, int& offset, int offlimit, int& argcnt);
    boolean skip_arg(ComValue* topval, int& offset, int offlimit, int& argcnt);

    void push_stack(postfix_token*);
    void token_to_comvalue(postfix_token*, ComValue*);
    const ComValue* stack(unsigned int &top) const;
    void load_sub_expr();
    void load_postfix(postfix_token*, int toklen, int tokoff);
    void eval_expr_internals(int pedepth=0);

    ComFuncState* top_funcstate();
    // return top ComFuncState on stack
    virtual void push_funcstate(ComFuncState& funcstate);
    // push ComFuncState onto stack
    virtual void pop_funcstate();
    // pop ComFuncState off stack

    ComValue* _stack;  // stack of multi-value objects, central to the interpreter
    int _stack_top; // current top of stack, -1 indicates empty
    unsigned int _stack_siz; // current maximum stack size
    boolean _quitflag; // flag that can be set to terminate interpreter
    char* _errbuf; // buffer used for rendering error messages
    char* _errbuf2; // ancillary buffer for rendering error messages
    int _pfoff; // current offset in _pfbuf
    boolean _brief; // when used to produce ComValue output
    boolean _just_reset; // flag that gets set after call to ::reset_stack()
    boolean _defaults_added; // flag for base set of commands added 

    ComValueTable* _localtable; // per interpreter symbol table
    static ComValueTable* _globaltable; // interpreter shared symbol table
    AttributeList* _alist; // extends symbol tables with names in an AttributeList

    ComFuncState* _fsstack;  // stack of func-status (nargs/nkeys/...) 
    int _fsstack_top;
    unsigned int _fsstack_siz;

    ComTerpState* _ctsstack;  // stack of ComTerpServ state
    int _ctsstack_top;
    unsigned int _ctsstack_siz;

    ComValue* _pfcomvals; 
    // postfix buffer of ComValue's converted from postfix_token

    static ComTerp* _instance;
    // default instance of a ComTerp

    ComterpHandler* _handler;
    // I/O handler for this ComTerp.

    ComFunc* _func_for_next_expr;
    // ComFunc to run on next expression

    ComValue* _val_for_next_func;
    // ComValue to pass to next command

    int _trace_mode;
    // trace mode

    int _npause;
    // depth of pause

    int _stepflag;
    // true if single-stepping interpreter

    friend class ComFunc;
    friend class ComterpHandler;
    friend class ComTerpIOHandler;
};

//: object for holding ComTerp state
// object that holds the state of a ComTerp
// which allows for nested and recursive use of a singular ComTerp
class ComTerpState {
public:
  ComTerpState() {}
  ComTerpState(ComTerpState& ctss) { *this = ctss; }
  // copy constructor.

  postfix_token*& pfbuf() { return _pfbuf; }
  int& pfnum() { return _pfnum; }
  int& pfoff() { return _pfoff; }
  int& bufptr() { return _bufptr; }
  int& bufsiz() { return _bufsiz; }
  int& linenum() { return _linenum; }
  //  int& just_reset() { return _just_reset; }
  char*& buffer() { return _buffer; }
  ComValue*& pfcomvals() { return _pfcomvals; }
  infuncptr& infunc() { return _infunc; }
  eoffuncptr& eoffunc() { return _eoffunc; }
  errfuncptr& errfunc() { return _errfunc; }
  void*& inptr() { return _inptr; }

protected:

  postfix_token* _pfbuf;
  int _pfnum;
  int _pfoff;
  int _bufptr;
  int _linenum;
  // int _just_reset;
  char* _buffer;
  int _bufsiz;
  ComValue* _pfcomvals;
  infuncptr _infunc;
  eoffuncptr _eoffunc;
  errfuncptr _errfunc;
  void* _inptr;

};
#endif /* !defined(_comterp_h) */
