/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 1994,1995,1999 Vectaport Inc.
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
 * collection of stream functions
 */

#if !defined(_strmfunc_h)
#define _strmfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;
class ComValue;

#define STREAM_EXTERNAL 1
#define STREAM_INTERNAL 2
#define STREAM_NESTED   4

//: base class for ComTerp stream commands.
class StrmFunc : public ComFunc {
public:
    StrmFunc(ComTerp*);

    static void print_stream(std::ostream& out, AttributeValue& streamv);
};

//: stream command
class StreamFunc : public StrmFunc {
public:
    StreamFunc(ComTerp*);

    virtual void execute();
    void execute_literal();  // handle (val val ...) stream literal syntax
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "strm=%s(strm|list|attrlist|val|fileobj|pipeobj) -- copy stream or convert list (unary $$)"; }

    CLASS_SYMID("StreamFunc");

};

//: ~~ spread operator -- drain a stream (or list/val) and push its elements
//: as separate positional arguments of the enclosing command, so one command
//: runs once over all of them (var-arg expansion), rather than a list-in-one-
//: arg ($/list) or a per-element command replay (overdrive).  Post-eval so its
//: own stream operand is never overdriven; spreads() so it may push a runtime-
//: variable count.  Reuses the operand stack's normal 2x growth -- an infinite
//: stream runs away like any non-terminating program (no special cap).
class SpreadFunc : public StrmFunc {
public:
    SpreadFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual boolean spreads() { return true; }
    virtual const char* docstring() {
      return "~~ spread operator -- expand a stream (or list) into the positional args of the enclosing command"; }

    CLASS_SYMID("SpreadFunc");

};

//: info command for stream objects.
// attrlst=info(streamobj)      -- AttributeList describing a literal stream's
//                                 directory: func, ntoks, nremaining,
//                                 elemN_off/elemN_cnt..., nelem.  Non-literal
//                                 streams report (:mode :func).
// lst=info(streamobj :raw) -- the raw internal directory list, layout-
//                                 agnostic; the probe used by regression tests.
class InfoFunc : public StrmFunc {
public:
    InfoFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "attrlst|lst=%s(strm :raw) -- return internal list of a stream"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
        ":raw       return raw internal list directly",
        nil };
      return keys; }

    CLASS_SYMID("InfoFunc");

};

//: hidden func used by next command for stream command
class StreamNextFunc : public StrmFunc {
public:
    StreamNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "hidden func used by next command for stream command."; }

    CLASS_SYMID("StreamNextFunc");

};

//: ,, (concat) operator.
class ConcatFunc : public StrmFunc {
public:
    ConcatFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return ",, is the stream concat operator"; }

    CLASS_SYMID("ConcatFunc");

};

//: hidden func used by next command for ,, (concat) operator.
class ConcatNextFunc : public StrmFunc {
public:
    ConcatNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "hidden func used by next command for ,, (stream concat) operator."; }

    CLASS_SYMID("ConcatNextFunc");

};

//: ** (repeat) operator.
class RepeatFunc : public StrmFunc {
public:
    RepeatFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "** is the repeat operator"; }

};

//: .. (iterate) operator.
class IterateFunc : public StrmFunc {
public:
    IterateFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return ".. is the iterate operator"; }

};

//: next command from stream for ComTerp
class NextFunc : public StrmFunc {
public:
    NextFunc(ComTerp*);

    virtual void execute();
    static  void execute_impl(ComTerp*, ComValue& strmv, boolean skim);
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "val=%s(stream :skim) -- return next value from stream, don't recurse if :skim"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":skim      do not recurse into nested streams",
	nil
      };
      return keys;
    }

    static int next_depth() { return _next_depth; }
protected:
    static int _next_depth;

};

//: traverse stream command for ComTerp.
// cnt=each(strm) -- traverse stream returning its length
class EachFunc : public ComFunc {
public:
    EachFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "cnt=%s(strm) -- traverse stream returning its length"; }
};

//: stream filter command
class FilterFunc : public StrmFunc {
public:
    FilterFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "val=filter(strm classid) filter a stream for a given classid"; }

    CLASS_SYMID("FilterFunc");

};

//: hidden func used by next command for stream filter command
class FilterNextFunc : public StrmFunc {
public:
    FilterNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "hidden func used by next command for filter command"; }

    CLASS_SYMID("FilterNextFunc");

};
//: hidden func used by next command for stream literal (val val ...) syntax
class StreamLiteralNextFunc : public StrmFunc {
public:
    StreamLiteralNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "hidden func used by next command for stream literal"; }

    CLASS_SYMID("StreamLiteralNextFunc");
};

#endif /* !defined(_strmfunc_h) */
