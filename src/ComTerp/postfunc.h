/*
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
// val=for(initexpr whileexpr [nextexpr] :body expr) -- for loop.
class ForFunc : public ComFunc {
public:
    ForFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "val=%s(initexpr whileexpr [nextexpr] :body expr) -- for loop"; }
};

//: while-loop command for ComTerp.
// val=while([testexpr] :until :body expr ) -- while loop.
class WhileFunc : public ComFunc {
public:
    WhileFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "val=%s([testexpr] :until :body expr ) -- while loop"; }
};

#endif /* !defined(_postfunc_h) */
