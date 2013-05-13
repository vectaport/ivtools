/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 1998,1999 Vectaport Inc.
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

#if !defined(_postfunc_h)
#define _postfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;

//: echo postfix output of parser.
// postfix(arg1 [arg2 [arg3 ... [argn]]]) -- echo unevaluated postfix arguments
// (with [narg|nkey] after defined commands, {narg|nkey} after undefined commands
// (narg) after keys).
class PostFixFunc : public ComFunc {
public:
    PostFixFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "%s(arg1 [arg2 [arg3 ... [argn]]]) -- echo unevaluated postfix arguments\n(with [narg|nkey] after defined commands, {narg|nkey} after undefined commands\n(narg) after keys)"; }
};

//: post-evaluate command for ComTerp.
// arr=posteval(arg1 [arg2 [arg3 ... [argn]]]) -- post-evaluate every fixed argument 
// (until nil) then return array.
class PostEvalFunc : public ComFunc {
public:
    PostEvalFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "arr=%s(arg1 [arg2 [arg3 ... [argn]]]) -- post-evaluate every fixed argument (until nil) then return array"; }
};

//: if-then-else command for ComTerp.
// val=if(testexpr :then expr :else expr) -- evaluate testexpr and execute the 
// :then expression if true, the :else expression if false.
class IfThenElseFunc : public ComFunc {
public:
    IfThenElseFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "val=%s(testexpr :then expr :else expr) -- evaluate testexpr and\nexecute the :then expression if true, the :else expression if false."; }
};

//: for-loop command for ComTerp.
// val=for(initexpr whileexpr [nextexpr [bodyexpr]] :body expr) -- for loop.
class ForFunc : public ComFunc {
public:
    ForFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "val=%s(initexpr whileexpr [nextexpr [bodyexpr]] :body expr) -- for loop"; }
};

//: while-loop command for ComTerp.
// val=while([testexpr [bodyexpr]] :nilchk :until :body expr ) -- while loop.
class WhileFunc : public ComFunc {
public:
    WhileFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "val=%s([testexpr [bodyexpr]] :nilchk :until :body expr ) -- while loop"; }
};

//: ; (sequence) operator.
class SeqFunc : public ComFunc {
public:
    SeqFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "; is the sequencing operator"; }

    static boolean continueflag() { return _continueflag; }
    static void continueflag(boolean flag) { _continueflag = flag; }
    static boolean breakflag() { return _breakflag; }
    static void breakflag(boolean flag) { _breakflag = flag; }

protected:
    static boolean _continueflag;
    static boolean _breakflag;
};

//: continue command
class ContinueFunc : public ComFunc {
public:
    ContinueFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s -- skip to next iteration of for or while loop"; }

};

//: break command
class BreakFunc : public ComFunc {
public:
    BreakFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s -- break out of for or while loop"; }

};

//: switch command for ComTerp.
// switch(val key-body-pairs) -- switch statement (:casen for pos., :case_n for neg., otherwise :symbol)
class SwitchFunc : public ComFunc {
public:
    SwitchFunc(ComTerp*);

    virtual boolean post_eval() { return true; }
    virtual void execute();
    virtual const char* docstring() { 
      return "switch(val key-body-pairs) -- switch statement (:casen for pos., :case_n for neg., otherwise :symbol)"; }

};

class FuncObj {
 public:
  FuncObj(postfix_token* toks, int ntoks); 
  virtual ~FuncObj();

  postfix_token* toks() { return _toks; }
  int ntoks() { return _ntoks; }

  CLASS_SYMID("FuncObj");

 protected:
  postfix_token* _toks;
  int _ntoks;
};
  
//: create token buffer object
// funcobj=func(body) -- encapsulate a body of commands into an executable object
class FuncObjFunc : public ComFunc {
public:
    FuncObjFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "funcobj=%s(body :echo) -- encapsulate a body of commands into an executable object"; }
};

#endif /* !defined(_postfunc_h) */
