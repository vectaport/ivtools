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
#define STREAM_SPREAD   8  // ~~ tag: drain into the enclosing call's positionals

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

//: ~~ spread operator -- expand a stream/list/attrlist held by variable into the
//: arguments of the enclosing call (list/stream elements -> positionals, attrlist
//: -> keywords), so one command runs once over all of them, rather than a
//: list-in-one-arg ($/list) or a per-element command replay (overdrive).
//: SpreadFunc itself only TAGS its operand (STREAM_SPREAD) and pushes exactly ONE
//: value -- the multi-value expansion happens later in eval_expr_internals,
//: upstream of the command/funcobj dispatch, which drains the tagged stream in
//: place.  Post-eval so its own stream operand is never overdriven.  No cap: an
//: infinite stream runs away like any non-terminating program.
class SpreadFunc : public StrmFunc {
public:
    SpreadFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "~~ spread operator -- expand a stream (or list) into the positional args of the enclosing command"; }

    CLASS_SYMID("SpreadFunc");

};

//: echo -- the inverse of ~~.  Returns its evaluated args in ~~-passable form:
//: positionals in a list, keywords as one single-attribute attrlist per keyword
//: at the TAIL of that list (list order thus preserves keyword order), or -- when
//: there are no positionals -- a bare multi-attribute attrlist.  So the round-trip
//: identity echo(~~echo(x)) == echo(x) holds for positional-only, keyword-only,
//: and mixed calls.
class EchoFunc : public ComFunc {
public:
    EchoFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "val=%s(arg[,arg...] [:key val...]) -- return evaluated args in ~~-passable form (positional list with tail attrlist singletons, or a bare attrlist)"; }
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

//: %% (replay) operator.
// The streaming counterpart to ** (repeat).  %% cycles a WHOLE stream through N
// full passes: A%%3 -> A's elements, three times over.  It must be post_eval so
// it receives the whole stream operand rather than being broadcast per-element
// (a non-post-eval binary op with one stream operand is vectorized element-wise
// -- that broadcast is exactly how ** repeats each element).  Like ConcatFunc,
// setup builds a stream driven by a separate next-func (ReplayNextFunc).  Runs a
// $$-copy of the source to exhaustion, then a fresh copy for the next pass, N
// times -- each pass an independent cursor, the source itself never consumed.
// Together with ** this yields cross-product streams: (A**3, B%%3).
class ReplayFunc : public StrmFunc {
public:
    ReplayFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "%% is the stream-replay operator (cycle a stream N times)"; }
};

//: hidden func used by next command for %% (replay) operator.
class ReplayNextFunc : public StrmFunc {
public:
    ReplayNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "hidden func used by next command for %% (stream replay) operator."; }
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
