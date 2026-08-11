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

#include <ComTerp/postfixspan.h>
#include <string.h>

PostfixSpanWalk::PostfixSpanWalk() {
    _capacity = 16;
    _stack = new Entry[_capacity];
    _size = 0;

    _consumed_capacity = 8;
    _consumed = new Span[_consumed_capacity];
    _consumed_count = 0;
}

PostfixSpanWalk::~PostfixSpanWalk() {
    delete [] _stack;
    delete [] _consumed;
}

void PostfixSpanWalk::push(Span span, int bound_value_count) {
    if (_size == _capacity) {
        int newcap = _capacity * 2;
        Entry* newstack = new Entry[newcap];
        memcpy(newstack, _stack, sizeof(Entry) * _size);
        delete [] _stack;
        _stack = newstack;
        _capacity = newcap;
    }
    _stack[_size].span = span;
    _stack[_size].bound_value_count = bound_value_count;
    _size++;
}

void PostfixSpanWalk::ensure_consumed_capacity(int n) {
    if (n <= _consumed_capacity) return;
    int newcap = _consumed_capacity;
    while (newcap < n) newcap *= 2;
    Span* newconsumed = new Span[newcap];
    memcpy(newconsumed, _consumed, sizeof(Span) * _consumed_count);
    delete [] _consumed;
    _consumed = newconsumed;
    _consumed_capacity = newcap;
}

/* Pops the top n stack entries into _consumed[0..n), oldest/deepest (i.e.
   earliest source position) first -- so _consumed reads in source order,
   the same order the tokens originally appeared in. */
void PostfixSpanWalk::pop_into_consumed(int n) {
    ensure_consumed_capacity(n);
    for (int k = n - 1; k >= 0; k--) {
        _size--;
        _consumed[k] = _stack[_size].span;
    }
    _consumed_count = n;
}

void PostfixSpanWalk::step(postfix_token* toks, int i) {
    unsigned type = toks[i].type;

    if (type == TOK_BLANK) {
        /* A matched-parens boundary around a pure grouping expression, e.g.
           the ")" closing "(a+b)" in "(a+b)*c" -- NOT a value of its own.
           The live evaluator (comterp.c:709,805,858, all three checking
           is_blank()) never pushes one either; whatever's already on the
           stack from the group's own content is what a following command
           should see. Pushing a leaf span here would orphan that content
           and make the enclosing command consume the wrong operand. */
        return;
    }

    if (type == TOK_KEYWORD) {
        /* A keyword marker's own narg is 0 (bare flag) or 1 (keyword+value)
           -- the count of already-pushed values it binds, per
           doc/FUNC-AND-ARGS-DESIGN.md and dotfunc.c's "knarg is 0 or 1 by
           construction". Combine marker + bound value (if any) into one
           span so a command consuming this keyword-group sees it as a
           single operand, matching comterp.c's own keyword-then-positional
           pop order (comterp.c:584-608). */
        int n = toks[i].narg;
        int start = i;
        if (n > 0) {
            pop_into_consumed(n);
            start = _consumed[0].start;
        }
        push(Span{start, i - start + 1}, n);
        return;
    }

    if (type == TOK_COMMAND) {
        int nkey = toks[i].nkey;
        int narg = toks[i].narg;

        /* Keywords are pushed last (positionals-first-then-keywords is a
           parser-enforced invariant, doc/POSTFIX-INDEXING.md's "Trailing
           positionals" section), so they're popped first here. Each
           sub-group is copied out to its own exactly-sized heap array
           (not a fixed-size local one -- narg/nkey come straight from the
           parser and are not bounded by any compile-time constant) before
           the next pop_into_consumed call reuses _consumed's low indices. */
        int keyword_val_total = 0;
        Span* keygroups = nkey > 0 ? new Span[nkey] : nil;
        int nkeygroups = 0;
        if (nkey > 0) {
            pop_into_consumed(nkey);
            for (int k = 0; k < nkey; k++) {
                keygroups[nkeygroups++] = _consumed[k];
            }
            /* bound_value_count of each popped keyword-group entry was on
               the stack Entry, not the Span -- recover it by re-deriving
               narg-for-that-keyword from the token at its span's last
               index (the keyword marker itself is always the last token
               in its own combined span). */
            for (int k = 0; k < nkey; k++) {
                int markeridx = keygroups[k].start + keygroups[k].count - 1;
                keyword_val_total += toks[markeridx].narg;
            }
        }

        int plain = narg - keyword_val_total;
        if (plain < 0) plain = 0;
        Span* posgroups = plain > 0 ? new Span[plain] : nil;
        int nposgroups = 0;
        if (plain > 0) {
            pop_into_consumed(plain);
            for (int k = 0; k < plain; k++) {
                posgroups[nposgroups++] = _consumed[k];
            }
        }

        /* Public consumed() order: plain positionals first, then keyword
           groups -- source order, per the same parser invariant above.
           Ensure room for BOTH groups combined before writing -- the two
           pop_into_consumed calls above each only guaranteed capacity for
           their own individual count, not the sum, so this call is not
           redundant with those. */
        ensure_consumed_capacity(nposgroups + nkeygroups);
        _consumed_count = 0;
        int start = i;
        for (int k = 0; k < nposgroups; k++) {
            if (_consumed_count == 0 || posgroups[k].start < start) start = posgroups[k].start;
            _consumed[_consumed_count++] = posgroups[k];
        }
        for (int k = 0; k < nkeygroups; k++) {
            if (_consumed_count == 0 || keygroups[k].start < start) start = keygroups[k].start;
            _consumed[_consumed_count++] = keygroups[k];
        }
        if (nposgroups == 0 && nkeygroups == 0) start = i;

        delete [] keygroups;
        delete [] posgroups;

        push(Span{start, i - start + 1}, 0);
        return;
    }

    /* Leaf: a literal value token (int, string, etc) or TOK_BLANK -- just
       itself, no operands. */
    push(Span{i, 1}, 0);
}
