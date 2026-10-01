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
#include <ComTerp/comvalue.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

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
        /* A matched-parens boundary (e.g. the ")" closing "(a+b)") is not a value
           on its own; a leaf span here would orphan the group's content, confusing the consumer */
        return;
    }

    if (type == TOK_KEYWORD) {
        /* A keyword marker's narg is 0 (bare flag) or 1 (keyword+value); combine
           marker + bound value (if any) into one span, consumed as a single
           operand. consumed_count() reflects only this call's own operand(s)
           -- a bare flag has none, so it is cleared here independently of
           any count a sibling keyword's step() may have left on it. */
        int n = toks[i].narg;
        int start = i;
        if (n > 0) {
            pop_into_consumed(n);
            start = _consumed[0].start;
        } else {
            _consumed_count = 0;
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
               recover it from the marker token at each span's last index */
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

void postfix_flatten_into(postfix_token* toks, int ntoks, AttributeValueList* avl) {
    for (int i = 0; i < ntoks; i++) {
        ComValue tv(&toks[i]);
        if (toks[i].type == TOK_KEYWORD && tv.narg() <= avl->Number()) {
            AttributeList* al = new AttributeList();
            ComValue keyval;
            if (tv.narg() == 1) {
                AttributeValue* prev = avl->Get(avl->Number()-1);
                keyval = ComValue(*prev);
                avl->Remove(prev);
            } else {
                AttributeValueList* sub = new AttributeValueList();
                for (int k = 0; k < tv.narg(); k++) {
                    ComValue subelt(*avl->Get(avl->Number()-tv.narg()+k));
                    sub->Append(new AttributeValue(subelt));
                }
                for (int k = 0; k < tv.narg(); k++)
                    avl->Remove(avl->Get(avl->Number()-1));
                keyval = ComValue(sub);
            }
            al->add_attr(tv.symbol_val(), keyval);
            ComValue alval(AttributeList::class_symid(), al);
            avl->Append(new AttributeValue(alval));
        } else
            avl->Append(new AttributeValue(tv));
    }
}

void postfix_nest_into(postfix_token* toks, int ntoks, AttributeValueList* avl) {
    PostfixSpanWalk walker;
    ComValue* built = new ComValue[ntoks];
    for (int i = 0; i < ntoks; i++) {
        walker.step(toks, i);
        int ncons = walker.consumed_count();
        ComValue tv(&toks[i]);
        if (toks[i].type == TOK_KEYWORD) {
            AttributeList* al = new AttributeList();
            ComValue keyval;
            if (ncons == 1) {
                PostfixSpanWalk::Span s = walker.consumed(0);
                keyval = built[s.start + s.count - 1];
            } else
                keyval = ComValue(new AttributeValueList());
            al->add_attr(tv.symbol_val(), keyval);
            built[i] = ComValue(AttributeList::class_symid(), al);
        } else if (toks[i].type == TOK_COMMAND && ncons > 0) {
            AttributeValueList* node = new AttributeValueList();
            node->Append(new AttributeValue(tv));
            for (int k = 0; k < ncons; k++) {
                PostfixSpanWalk::Span s = walker.consumed(k);
                ComValue opnd(built[s.start + s.count - 1]);
                node->Append(new AttributeValue(opnd));
            }
            built[i] = ComValue(node);
        } else
            /* a literal, TOK_BLANK, or a bare command reference (a
               variable read, narg==nkey==0) is its own subtree */
            built[i] = tv;
    }
    for (int k = 0; k < walker.remaining_count(); k++) {
        PostfixSpanWalk::Span s = walker.remaining(k);
        ComValue result(built[s.start + s.count - 1]);
        avl->Append(new AttributeValue(result));
    }
    delete [] built;
}
