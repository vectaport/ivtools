/*
 * Copyright (c) 2026 Scott E. Johnston
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

#if !defined(_postfixspan_h)
#define _postfixspan_h

#include <ComUtil/comterp.h>
#include <ComTerp/comvalue.h>

// PostfixSpanWalk: a single forward (left-to-right) pass over an already
// detached, forward-ordered postfix_token buffer -- the kind
// ComTerp::copy_post_eval_expr/copy_stack_arg_post_eval produce (a FuncObj
// body, a stream-literal's copied token region, etc).
//
// ComTerp's own skip_func/skip_key/skip_arg (comterp.c) are the arity-aware
// traversal for the *live* _pfbuf, walked backward from a stack-relative
// anchor -- they don't apply to a buffer that's already been copied out.
// This is the forward-buffer analog: arity comes straight off each token's
// own narg()/nkey(), so no ComTerp instance or live stack is needed at all.
//
// Usage: call step() once per token index, 0..ntoks-1, in order.  After
// stepping a command token (nids>=0, i.e. TOK_COMMAND), consumed_count()/
// consumed(k) report the operand spans it just popped, in source order
// (k==0 is the first source-order operand -- the lvalue position for an
// assign-family command).  After the whole buffer has been walked,
// remaining_count()/remaining(k) report what's left on the stack: one span
// per top-level ';'-sequenced statement result.
class PostfixSpanWalk {
public:
    struct Span { int start; int count; };

    PostfixSpanWalk();
    ~PostfixSpanWalk();

    void step(postfix_token* toks, int i);
    // Same algorithm as step(postfix_token*, int) above, read live off a
    // ComValue buffer (_pfcomvals) instead of a detached postfix_token copy
    // -- a hand-mirrored duplicate pending consolidation.
    void step(ComValue* vals, int i);

    void seed(Span span) { push(span, 0); }
    // prime the walk with a span already known by other means (e.g. a
    // command's own just-pushed result), so step() can be called starting
    // partway through a buffer instead of from token 0 -- lets a caller
    // ask "what consumes this span next" without re-walking everything
    // before it.

    int consumed_count() const { return _consumed_count; }
    Span consumed(int k) const { return _consumed[k]; }

    int remaining_count() const { return _size; }
    Span remaining(int k) const { return _stack[k].span; }

protected:
    void push(Span span, int bound_value_count);
    void ensure_consumed_capacity(int n);
    void pop_into_consumed(int n);

    struct Entry { Span span; int bound_value_count; };

    Entry* _stack;
    int _size;
    int _capacity;

    Span* _consumed;
    int _consumed_count;
    int _consumed_capacity;
};

// find_next_eager_parent: the command or keyword marker that consumes the
// span at seed_idx, found by walking buf forward from start (inclusive) with
// a PostfixSpanWalk seeded at seed_idx -- skipping whole sibling subtrees
// along the way rather than checking only the next token.  Returns that
// token's index, or -1 if nothing in [start, bufsiz) consumes it (seed_idx
// is the top of its statement).  Restricted to the purely-eager case: the
// caller's own seed_idx must be valid, which pfoff()-1 is only when the
// caller isn't itself running inside another post-eval command's own
// operand span.
int find_next_eager_parent(ComValue* buf, int bufsiz, int start, int seed_idx);

#endif /* !defined(_postfixspan_h) */
