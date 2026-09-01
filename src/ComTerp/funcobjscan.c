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

#include <ComTerp/funcobjscan.h>
#include <ComTerp/postfixspan.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Attribute/attrvalue.h>

#include <limits.h>

/* Per-occurrence event, for the FIRST-mention/ever-written bookkeeping --
   distinct from the public Kind, which is the FINAL classification derived
   from these once the whole body has been walked. */
enum VarEvent { EvRead, EvWrite, EvReadThenWrite };

struct VarRecord {
    int symid;
    VarEvent first_event;
    boolean ever_written;
};

struct EscapeRecord {
    int symid;
    boolean is_global;
};

static VarRecord* find_or_add_var(VarRecord*& recs, int& n, int& cap, int symid, boolean* is_new) {
    for (int i = 0; i < n; i++) {
        if (recs[i].symid == symid) { *is_new = false; return &recs[i]; }
    }
    if (n == cap) {
        int newcap = cap ? cap * 2 : 8;
        VarRecord* newrecs = new VarRecord[newcap];
        for (int i = 0; i < n; i++) newrecs[i] = recs[i];
        delete [] recs;
        recs = newrecs;
        cap = newcap;
    }
    recs[n].symid = symid;
    n++;
    *is_new = true;
    return &recs[n-1];
}

static void note_escape(EscapeRecord*& recs, int& n, int& cap, int symid, boolean is_global) {
    for (int i = 0; i < n; i++) {
        if (recs[i].symid == symid) { recs[i].is_global = is_global; return; }
    }
    if (n == cap) {
        int newcap = cap ? cap * 2 : 8;
        EscapeRecord* newrecs = new EscapeRecord[newcap];
        for (int i = 0; i < n; i++) newrecs[i] = recs[i];
        delete [] recs;
        recs = newrecs;
        cap = newcap;
    }
    recs[n].symid = symid;
    recs[n].is_global = is_global;
    n++;
}

/* Records a plain-var occurrence's event -- called once per occurrence, in
   body order, so "first_event" naturally lands on whichever occurrence is
   seen first. */
static void note_event(VarRecord*& recs, int& n, int& cap, int symid, VarEvent ev) {
    boolean is_new;
    VarRecord* r = find_or_add_var(recs, n, cap, symid, &is_new);
    if (is_new) {
        r->first_event = ev;
        r->ever_written = false;   /* new VarRecord[] leaves this uninitialized */
    }
    if (ev == EvWrite || ev == EvReadThenWrite) r->ever_written = true;
}

static boolean symid_in_set(int* set, int n, int symid) {
    for (int i = 0; i < n; i++) if (set[i] == symid) return true;
    return false;
}

static void add_to_set(int*& set, int& n, int& cap, int symid) {
    if (symid_in_set(set, n, symid)) return;
    if (n == cap) {
        int newcap = cap ? cap * 2 : 8;
        int* newset = new int[newcap];
        for (int i = 0; i < n; i++) newset[i] = set[i];
        delete [] set;
        set = newset;
        cap = newcap;
    }
    set[n++] = symid;
}

/* True iff span is exactly one plain-var token -- the shape a symbol-level
   operand of assign/local/global must have to be classifiable at all (see
   funcobjscan.h: anything else, e.g. a dot-chain target, is out of scope
   and simply produces no event). */
static boolean span_is_plain_var(PostfixSpanWalk::Span span, boolean* is_plain_var) {
    return span.count == 1 && is_plain_var[span.start];
}

boolean* FuncObjVarScan::build_is_plain_var(ComTerp* comterp, postfix_token* toks, int ntoks) {
    boolean* is_plain_var = new boolean[ntoks];
    for (int i = 0; i < ntoks; i++) {
        /* nids<0 (HACKING.md's "Dot Operator Rhs" section) marks a bare
           identifier on the right of a dot -- an attribute-key literal
           like the "v" in "obj.v", never promoted to CommandType
           regardless of whether that name is also a registered command,
           but not an ordinary variable reference either. */
        if (toks[i].type == TOK_COMMAND && toks[i].nids >= 0) {
            ComValue sv;
            comterp->token_to_comvalue(&toks[i], &sv);
            is_plain_var[i] = sv.type() == ComValue::SymbolType;
        } else {
            is_plain_var[i] = false;
        }
    }
    return is_plain_var;
}

FuncObjVarScan::PositionalInfo FuncObjVarScan::scan_positionals(postfix_token* toks, int ntoks) {
    static int arg_symid = symbol_add("arg");
    static int narg_symid = symbol_add("narg");

    PositionalInfo info;
    info.count = -1;
    info.uses_narg = false;

    long maxidx = -1;           /* highest literal index seen: arg(0) -> 0 --
                                    long (not int) so maxidx+1 below can't
                                    overflow for a literal near INT_MAX */
    boolean saw_arg = false;
    boolean saw_nonliteral = false;

    PostfixSpanWalk walk;
    for (int i = 0; i < ntoks; i++) {
        walk.step(toks, i);
        if (toks[i].type != TOK_COMMAND) continue;
        int symid = toks[i].v.symbolid;

        if (symid == narg_symid) {
            /* narg() anywhere in the body reads as "this loops over a
               run of positionals bounded at call time," i.e. variadic --
               not resolvable to one fixed count regardless of any
               literal arg(n) indices also present. */
            info.uses_narg = true;
            continue;
        }

        if (symid == arg_symid && walk.consumed_count() == 1) {
            saw_arg = true;
            PostfixSpanWalk::Span operand = walk.consumed(0);
            if (operand.count == 1 &&
                (toks[operand.start].type == TOK_DFINT ||
                 toks[operand.start].type == TOK_LNINT)) {
                long idx = toks[operand.start].type == TOK_DFINT
                    ? (long)toks[operand.start].v.dfintval
                    : toks[operand.start].v.lnintval;
                if (idx > maxidx) maxidx = idx;
            } else {
                /* a computed index (arg(i), arg(i+1), ...) -- resolving
                   simple cases statically is future work (
                   point 2's "attempt, fall back to dynamic" allowance);
                   this first pass gives up gracefully instead of
                   guessing. */
                saw_nonliteral = true;
            }
        }
    }

    if (info.uses_narg || saw_nonliteral)
        info.count = -1;
    else if (saw_arg) {
        /* maxidx+1 overflowing (maxidx == LONG_MAX) is signed-integer UB,
           not just "unlikely" -- check before doing the addition rather
           than let it wrap and rely on that wrapping to coincidentally
           land back on the same -1 "can't be pinned down" sentinel. This is unreachable through today's literal
           parsing (values above INT_MAX don't currently survive intact --
           a separate, pre-existing bug), but the guard costs nothing
           and removes the UB regardless of whether any path can trigger
           it today. */
        if (maxidx == LONG_MAX)
            info.count = -1;
        else
            info.count = maxidx + 1;
    } else
        info.count = 0;      /* no arg(n) calls at all -- a niladic body */

    return info;
}

AttributeList* FuncObjVarScan::classify(postfix_token* toks, int ntoks, boolean* is_plain_var) {
    static int assign_symid = symbol_add("assign");
    static int local_symid = symbol_add("local");
    static int global_symid = symbol_add("global");
    static int dot_symid = symbol_add("dot");
    /* First operand is read-then-written in one occurrence -- see the capture classifier's
       plan: distinct from plain assign, whose first operand is a pure
       write (the old value is never read). */
    static int compound_assign_symids[] = {
        symbol_add("mod_assign"), symbol_add("mpy_assign"),
        symbol_add("add_assign"), symbol_add("sub_assign"),
        symbol_add("div_assign"), symbol_add("incr"),
        symbol_add("incr_after"), symbol_add("decr"),
        symbol_add("decr_after")
    };
    static int n_compound_assign = sizeof(compound_assign_symids) / sizeof(int);

    VarRecord* recs = nil;
    int nrecs = 0, recs_cap = 0;
    EscapeRecord* escapes = nil;
    int nescapes = 0, escapes_cap = 0;
    /* symbols used anywhere in the body as a dot-chain root (obj.field).
       DotFunc's attribute-write path falls through _alist, then local, then
       global to decide whether obj already exists or needs creating in the
       outer scope.  A capture pre-seeding al[obj] with a declaration-time
       snapshot would make that path see it as already present and stop short,
       breaking the documented bleed to outer scope.  So they are excluded from
       capture globally -- collected here, filtered at output. */
    int* dotroots = nil;
    int ndotroots = 0, dotroots_cap = 0;

    PostfixSpanWalk walk;
    for (int i = 0; i < ntoks; i++) {
        walk.step(toks, i);

        if (toks[i].type != TOK_COMMAND) continue;
        int symid = toks[i].v.symbolid;
        int nconsumed = walk.consumed_count();

        if ((symid == local_symid || symid == global_symid) && nconsumed == 1) {
            PostfixSpanWalk::Span arg = walk.consumed(0);
            if (span_is_plain_var(arg, is_plain_var)) {
                note_escape(escapes, nescapes, escapes_cap, toks[arg.start].v.symbolid,
                            symid == global_symid);
            }
            continue;
        }

        if (symid == dot_symid && nconsumed >= 1) {
            PostfixSpanWalk::Span root = walk.consumed(0);
            if (span_is_plain_var(root, is_plain_var)) {
                add_to_set(dotroots, ndotroots, dotroots_cap, toks[root.start].v.symbolid);
            }
            continue;
        }

        boolean is_compound_assign = false;
        for (int c = 0; c < n_compound_assign; c++) {
            if (symid == compound_assign_symids[c]) { is_compound_assign = true; break; }
        }

        for (int k = 0; k < nconsumed; k++) {
            PostfixSpanWalk::Span operand = walk.consumed(k);
            if (!span_is_plain_var(operand, is_plain_var)) continue;
            int varsymid = toks[operand.start].v.symbolid;

            if (k == 0 && symid == assign_symid) {
                note_event(recs, nrecs, recs_cap, varsymid, EvWrite);
            } else if (k == 0 && is_compound_assign) {
                note_event(recs, nrecs, recs_cap, varsymid, EvReadThenWrite);
            } else {
                note_event(recs, nrecs, recs_cap, varsymid, EvRead);
            }
        }
    }

    /* A plain-var token that never got consumed by anything -- e.g. a body
       that's just "x" -- is still an ordinary read: the body's own return
       value.  Only the walker's own final remaining() spans were truly
       never consumed by anything -- NOT every is_plain_var token, since a
       token consumed into an escape (local()/global(), handled above via
       `continue`, so it never reaches note_event) was consumed, just not
       into an ordinary read/write event.  Scanning all is_plain_var
       tokens here would wrongly re-count those as leftover reads. */
    for (int k = 0; k < walk.remaining_count(); k++) {
        PostfixSpanWalk::Span span = walk.remaining(k);
        if (!span_is_plain_var(span, is_plain_var)) continue;
        int symid = toks[span.start].v.symbolid;
        boolean is_new;
        find_or_add_var(recs, nrecs, recs_cap, symid, &is_new);
        if (is_new) {
            recs[nrecs-1].first_event = EvRead;
            recs[nrecs-1].ever_written = false;
        }
    }

    AttributeList* result = new AttributeList();

    for (int i = 0; i < nrecs; i++) {
        if (symid_in_set(dotroots, ndotroots, recs[i].symid)) continue;
        Kind kind;
        if (recs[i].first_event == EvWrite) {
            kind = WriteBeforeRead;
        } else if (recs[i].ever_written) {
            kind = ReadBeforeWrite;
        } else {
            kind = ReadOnly;
        }
        AttributeValue kindval((int)kind, AttributeValue::IntType);
        result->add_attr(recs[i].symid, kindval);
    }

    for (int i = 0; i < nescapes; i++) {
        boolean already_plain = false;
        for (int j = 0; j < nrecs; j++) {
            if (recs[j].symid == escapes[i].symid) { already_plain = true; break; }
        }
        if (already_plain) continue;
        Kind kind = escapes[i].is_global ? EscapingGlobal : EscapingLocal;
        AttributeValue kindval((int)kind, AttributeValue::IntType);
        result->add_attr(escapes[i].symid, kindval);
    }

    delete [] recs;
    delete [] escapes;
    delete [] dotroots;

    return result;
}

AttributeList* FuncObjVarScan::scan_defaults(ComTerp* comterp, postfix_token* toks, int ntoks, boolean* is_plain_var) {
    static int if_symid = symbol_add("if");
    static int eq_symid = symbol_add("eq");
    static int nil_symid = symbol_add("nil");
    static int then_symid = symbol_add("then");
    static int else_symid = symbol_add("else");

    AttributeList* result = new AttributeList();

    PostfixSpanWalk walk;
    for (int i = 0; i < ntoks; i++) {
        walk.step(toks, i);
        if (toks[i].type != TOK_COMMAND || (unsigned)toks[i].v.symbolid != (unsigned)if_symid) continue;
        /* only the plain 3-operand if(cond :then v :else v) shape --
           :until/:nilchk or any other keyword on this if() means it
           isn't this idiom at all */
        if (walk.consumed_count() != 3) continue;

        PostfixSpanWalk::Span condspan = walk.consumed(0);
        PostfixSpanWalk::Span branch1 = walk.consumed(1);
        PostfixSpanWalk::Span branch2 = walk.consumed(2);

        /* condition must be exactly "K==nil" or "nil==K" -- 3 tokens,
           last one eq, the other two bare single-token operands, one of
           them the literal nil command and the other a plain variable
           (the keyword this default belongs to) */
        if (condspan.count != 3) continue;
        int eqtok = condspan.start + 2;
        if (toks[eqtok].type != TOK_COMMAND || (unsigned)toks[eqtok].v.symbolid != (unsigned)eq_symid) continue;
        int t0 = condspan.start, t1 = condspan.start + 1;
        boolean t0_nil = toks[t0].type == TOK_COMMAND && (unsigned)toks[t0].v.symbolid == (unsigned)nil_symid;
        boolean t1_nil = toks[t1].type == TOK_COMMAND && (unsigned)toks[t1].v.symbolid == (unsigned)nil_symid;
        int keysym;
        if (t0_nil && is_plain_var[t1]) keysym = toks[t1].v.symbolid;
        else if (t1_nil && is_plain_var[t0]) keysym = toks[t0].v.symbolid;
        else continue;

        /* branch1/branch2 (source order) must each end in a KEYWORD
           token -- that's what identifies which is :then and which is
           :else (see funcobjscan.h's spanwalk comment: a keyword-tagged
           operand's span includes its trailing KEYWORD marker token) */
        PostfixSpanWalk::Span then_span, else_span;
        boolean have_then = false, have_else = false;
        PostfixSpanWalk::Span branches[2];
        branches[0] = branch1;
        branches[1] = branch2;
        for (int b = 0; b < 2; b++) {
            PostfixSpanWalk::Span sp = branches[b];
            if (sp.count < 1) continue;
            int last = sp.start + sp.count - 1;
            if (toks[last].type != TOK_KEYWORD) continue;
            if ((unsigned)toks[last].v.symbolid == (unsigned)then_symid) { then_span = sp; have_then = true; }
            else if ((unsigned)toks[last].v.symbolid == (unsigned)else_symid) { else_span = sp; have_else = true; }
        }
        if (!have_then || !have_else) continue;

        /* :else's value portion (span minus its trailing keyword token)
           must be exactly the bare keyword, unchanged -- confirms this
           if() really is the "return x as-is" idiom for THIS keysym,
           not some other, unrelated keyword-adjacent if() */
        if (else_span.count != 2) continue;
        if (!is_plain_var[else_span.start] || toks[else_span.start].v.symbolid != keysym) continue;

        /* :then's value portion must be exactly one literal token --
           give up gracefully (no default reported) on anything computed,
           same restraint scan_positionals uses for a non-literal arg(n)
           index */
        if (then_span.count != 2) continue;
        ComValue litval;
        comterp->token_to_comvalue(&toks[then_span.start], &litval);
        if (litval.is_type(AttributeValue::CommandType) || litval.is_type(AttributeValue::SymbolType)) continue;

        result->add_attr(keysym, litval);
    }

    return result;
}
