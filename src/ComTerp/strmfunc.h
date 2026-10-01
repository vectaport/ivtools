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
#define STREAM_FUNCOBJ 16  // packed callee is a FuncObj, not a ComFunc
#define STREAM_RING    32  // string-backed ring FIFO -- see FeedFunc/RingNextFunc

//: base class for ComTerp stream commands.
class StrmFunc : public ComFunc {
public:
    StrmFunc(ComTerp*);

    static void print_stream(std::ostream& out, AttributeValue& streamv);

    /* true if val is the bquoted `EOS symbol -- the nested-stream delimiter
       convention (see feed()); checked by identity, not by resolution, so
       it works the same whether or not EOS happens to be bound to anything. */
    static boolean is_delimiter(ComValue& val);
};

//: stream command
class StreamFunc : public StrmFunc {
public:
    StreamFunc(ComTerp*);

    virtual void execute();
    void execute_literal();  // handle (val val ...) stream literal syntax
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "strm=%s(strm|list|attrlist|str|val|fileobj|pipeobj) -- copy stream or convert list (unary $$); a string streams its characters, a bquoted one is carried whole"; }


protected:
    /* build+push a stream from an ALREADY-evaluated operand.  Factored out
       of execute() so a subclass that must peek at its operand's value
       before deciding how to handle it (GrStreamFunc::execute(), which
       checks object_compview()) can hand that already-fired ComValue
       straight in here instead of re-firing the original (post_eval)
       argument expression a second time -- the same peek-once-then-thread-
       the-value-through discipline GrDotFunc::execute() documents. */
    void push_stream_from_value(ComValue& operand1);

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
      return "val=%s(arg [arg [...]] [:key val...]) -- return evaluated args in ~~-passable form (positional list with tail attrlist singletons, or a bare attrlist)"; }
};

//: info command for opaque Obj types.
// attrlst=info(streamobj)  -- AttributeList describing a literal stream's
//                             directory: func, ntoks, nremaining,
//                             elemN_off/elemN_cnt..., nelem.  Non-literal
//                             streams report (:mode :func).
// lst=info(streamobj :raw) -- the raw internal directory list, which is
//                             layout-agnostic.
// attrlst=info(lst)        -- (:count :coloned :nested :cutoff) for a
//                             plain list; :nested is the list's own
//                             nested_insert() flag (governs whether ','
//                             and ':' mutate it in place or nest it),
//                             :cutoff is its own print-elision cutoff
//                             (max_out(), -1 meaning "inherit cutoff()").
// attrlst=info(attrlst)    -- (:sealed :count).
// attrlst=info(funcname)   -- a bare, unfired func by name: (:ntoks :nspans :posteval).
// attrlst=info(fileobj)    -- (:filename :mode :open).
// attrlst=info(pipeobj)    -- (:command :pid).
// attrlst=info(sockobj)    -- (:host :port).
// list=info(dateobj|timeobj) -- the coloned year:mon:day[:hr:min:sec...]
//                             list printOn() already renders as text, but
//                             as a live value -- printOn() has no other way
//                             to hand it back out.
class InfoFunc : public StrmFunc {
public:
    InfoFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "attrlst=%s(obj [:raw]) -- inspect an opaque value's internal facts"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
        ":raw       for a stream, return its raw internal list directly",
        nil };
      return keys; }


};

//: hidden func used by next command for stream command
class StreamNextFunc : public StrmFunc {
public:
    StreamNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "hidden func used by next command for stream command."; }


};

//: cursor over a string's characters, used by next() for a string-backed stream.
class StringNextFunc : public StrmFunc {
public:
    StringNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "hidden func used by next command for a string-backed stream."; }

    CLASS_SYMID_HIDDEN("StringNextFunc");

};

//: ,, (concat) operator.
class ConcatFunc : public StrmFunc {
public:
    ConcatFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return ",, is the stream concat operator"; }


};

//: hidden func used by next command for ,, (concat) operator.
class ConcatNextFunc : public StrmFunc {
public:
    ConcatNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "hidden func used by next command for ,, (stream concat) operator."; }


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

//: next command from stream for ComTerp; also * (unary prefix next) operator.
class NextFunc : public StrmFunc {
public:
    NextFunc(ComTerp*);

    virtual void execute();
    static  void execute_impl(ComTerp*, ComValue& strmv);
    /* drains an at()-produced streamed-lvalue (idxstream, is_stream()&&
       lhs_assign()) against a value source, writing each pulled source
       value via at(...:set...); rhsval is read once per destination if
       rhs_is_stream, else broadcast unchanged.  Returns the write count,
       or -1 if the streamed target isn't list/string-shaped (e.g. an
       attrlist) -- a hard abort, not a partial count, matching the
       non-streamed al@n=val case's nil.  Shared by AssignFunc's
       `lst@lo:hi=val` and NextFunc's own streamed-var zipper so the one
       pull-then-check-exhaustion loop isn't reimplemented per caller. */
    static  int  zip_assign_stream(ComTerp*, ComValue& idxstream,
				    ComValue* rhsval, boolean rhs_is_stream,
				    int linenum);
    /* completes a single at()-produced [list,idx] pair's write (pairv,
       is_array()&&lhs_assign()) by re-driving at() with :set, returning
       the written result.  Shared by AssignFunc's scalar `lst@N=val` and
       NextFunc's own single-target zipper case. */
    static  ComValue write_at_pair(ComTerp*, ComValue& pairv, ComValue& writeval);
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      /* %1$s (not plain %s) reused twice: helpfunc.c passes only one
         substitution argument, and a second bare %s would read past it */
      return "val=%1$s(stream [var]) -- return next value from stream\n\
*s is unary-prefix sugar for %1$s(s)\n\
with var, also assigns the pulled value (including nil) to that variable\n\
var may also be a settable expression: a streamed at() (r@lo:hi) zip-writes\n\
each pulled value and returns the write count; a scalar at() (r@n) or a\n\
dot() (al.field) write the one pulled value in place and still return it"; }

    static int next_depth() { return _next_depth; }
protected:
    /* the non-symbol, lhs-eligible var dispatch (streamed at(), scalar
       at(), dot()) -- split out of execute() so its own argument-evaluation
       order (var before stream, required for the lhs postfix-buffer flag
       to take effect) stays isolated from the plain-symbol path's original
       order. */
    void execute_var_dispatch(ComValue& streamv, ComValue& varname, int linenum);
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


};

//: hidden func used by next command for stream filter command
class FilterNextFunc : public StrmFunc {
public:
    FilterNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "hidden func used by next command for filter command"; }


};
//: hidden func used by next command for stream literal (val val ...) syntax
class StreamLiteralNextFunc : public StrmFunc {
public:
    StreamLiteralNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "hidden func used by next command for stream literal"; }

};

//: command to build or append to a FIFO stream, the write-end
//: complement to next().  A bare (unprotected) string first argument
//: builds a fixed-capacity ring FIFO over that string's own bytes
//: instead of a growable one -- see RingNextFunc.
// fifo=feed([fifo] [val ...] :raw :noring) -- build or append to a FIFO stream
class FeedFunc : public ComFunc {
public:
    FeedFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "fifo=%s([fifo] [val ...] :raw :noring) -- build or append to a FIFO stream; a bare string argument becomes a fixed-capacity ring over its own bytes, a bquoted one is stored whole"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":raw       store a stream or string argument whole instead of taking",
	"           it apart -- applies to every argument in the call, where a",
	"           bquoted value protects just itself",
	":noring    for a new string-backed FIFO, refuse a push once the buffer",
	"           fills rather than wrapping to reclaim drained space",
	nil
      };
      return keys;
    }

};

//: command to re-grain a stream: pull up to n elements into an indexed list,
//: one block per next(), so a script loop pays interpretation once per block
//: instead of once per element.
// lst=chunk(strm n) -- convert a stream into a stream of n-element lists
class ChunkFunc : public ComFunc {
public:
    ChunkFunc(ComTerp*);

    virtual void execute();
    /* post_eval so a stream argument arrives whole: a non-post-eval command
       with a stream arg is overdriven, which would lift chunk over the very
       stream it is meant to re-grain. */
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "strm=%s(strm n) -- re-grain a stream into a stream of n-element lists, so a script loop pays interpretation once per block instead of once per element"; }

};

//: hidden func used by next command for chunk-built block streams
class ChunkNextFunc : public StrmFunc {
public:
    ChunkNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "hidden func used by next command for chunk-built block streams"; }

};

//: hidden func used by next command for feed-built FIFO streams
class FeedNextFunc : public StrmFunc {
public:
    FeedNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "hidden func used by next command for feed-built FIFO streams"; }

};

//: hidden func used by next command for a string-backed ring FIFO
//: (feed(str) -- see FeedFunc).  The stream's avl carries
//: [0]=buf [1]=head [2]=tail [3]=count [4]=wrap(0|1), all bytes in
//: buf's own storage -- see FeedFunc::execute() for the write side.
class RingNextFunc : public StrmFunc {
public:
    RingNextFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "hidden func used by next command for a string-backed ring FIFO"; }

};

#endif /* !defined(_strmfunc_h) */
