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
   the same order the tokens originally appeared in.  A walk that started
   mid-buffer (seed(), rather than token 0) can be asked to pop more than
   it has ever tracked -- e.g. print("fmt" 20:23:44)'s format string,
   pushed before a walk seeded at the colon chain's own result -- which
   isn't an error, just an operand outside this walk's tracked window;
   stand in a sentinel (start<0) that can never match a real span rather
   than reading _stack out of bounds. */
void PostfixSpanWalk::pop_into_consumed(int n) {
    ensure_consumed_capacity(n);
    for (int k = n - 1; k >= 0; k--) {
        if (_size > 0) {
            _size--;
            _consumed[k] = _stack[_size].span;
        } else {
            _consumed[k] = Span{-1, 0};
        }
    }
    _consumed_count = n;
}

void PostfixSpanWalk::step(postfix_token* toks, int i) {
    unsigned type = toks[i].type;

    if (type == TOK_BLANK) {
        /* A matched-parens boundary (e.g. the ")" closing "(a+b)") is not a value
           on its own; a leaf span here would orphan the group's content, confusing the consumer */
        return;
    }

    if (type == TOK_KEYWORD) {
        /* A keyword marker's narg is 0 (bare flag) or 1 (keyword+value); combine
           marker + bound value (if any) into one span, consumed as a single operand */
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

        /* Keywords push last (positionals then keywords), so they pop
           first; copy each sub-group out before pop_into_consumed reuses _consumed's low indices */
        int keyword_val_total = 0;
        Span* keygroups = nkey > 0 ? new Span[nkey] : nil;
        int nkeygroups = 0;
        if (nkey > 0) {
            pop_into_consumed(nkey);
            for (int k = 0; k < nkey; k++) {
                keygroups[nkeygroups++] = _consumed[k];
            }
            /* bound_value_count lived on the stack Entry, not the Span --
               recover it from the marker token at each span's last index;
               a sentinel group (count==0, from an untracked pop) contributes
               nothing and has no marker token to index */
            for (int k = 0; k < nkey; k++) {
                if (keygroups[k].count <= 0) continue;
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

        /* Public consumed() order: plain positionals first, then keyword groups;
           reserve room for BOTH: each earlier pop_into_consumed call sized only for its own count */
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

    /* Leaf: a literal value token (int, string, etc) or TOK_BLANK --
       just itself, no operands */
    push(Span{i, 1}, 0);
}

/* PROTOTYPE mirror of step(postfix_token*, int) above, reading a live
   ComValue buffer instead -- see the .h comment; consolidate once proven. */
void PostfixSpanWalk::step(ComValue* vals, int i) {
    ComValue& val = vals[i];

    if (val.is_type(ComValue::BlankType)) {
        return;
    }

    if (val.is_type(ComValue::KeywordType)) {
        int n = val.narg();
        int start = i;
        if (n > 0) {
            pop_into_consumed(n);
            start = _consumed[0].start;
        }
        push(Span{start, i - start + 1}, n);
        return;
    }

    if (val.is_type(ComValue::CommandType)) {
        int nkey = val.nkey();
        int narg = val.narg();

        int keyword_val_total = 0;
        Span* keygroups = nkey > 0 ? new Span[nkey] : nil;
        int nkeygroups = 0;
        if (nkey > 0) {
            pop_into_consumed(nkey);
            for (int k = 0; k < nkey; k++) {
                keygroups[nkeygroups++] = _consumed[k];
            }
            for (int k = 0; k < nkey; k++) {
                if (keygroups[k].count <= 0) continue;
                int markeridx = keygroups[k].start + keygroups[k].count - 1;
                keyword_val_total += vals[markeridx].narg();
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

    /* Leaf: a literal value (int, string, etc) -- just itself, no operands */
    push(Span{i, 1}, 0);
}
