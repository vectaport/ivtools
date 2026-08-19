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

#if !defined(_funcobjscan_h)
#define _funcobjscan_h

#include <ComUtil/comterp.h>

class AttributeList;
class ComTerp;

// FuncObjVarScan: classifies every distinct symbol referenced in a FuncObj
// body's token span into #170's taxonomy -- read-only, read-before-write,
// write-before-read, or escaping (local()/global()).  Used by #310 (func()
// closures via declaration-time capture) and #170 (:help contract
// derivation) -- one classifier, two consumers.
//
// Built on PostfixSpanWalk (postfixspan.h) for the underlying traversal.
// Does no symbol-table lookup itself and needs no ComTerp access -- the
// caller (a ComFunc, a friend of ComTerp, e.g. FuncObjFunc::execute) has
// already resolved each token's symbol-vs-command status once via the
// existing ComTerp::token_to_comvalue and passes the result in as
// is_plain_var[], so this stays a pure structural pass with no
// side effects and no dependency on live interpreter state.
class FuncObjVarScan {
public:
    enum Kind { ReadOnly, ReadBeforeWrite, WriteBeforeRead, EscapingLocal, EscapingGlobal };

    // is_plain_var[i] must be true iff toks[i] is a bare, zero-arg symbol
    // reference (TOK_COMMAND, narg==0, nkey==0) that resolves to an
    // ordinary variable rather than a registered command -- i.e. what
    // ComTerp::token_to_comvalue leaves as ComValue::SymbolType instead of
    // promoting to CommandType.  Every other token (real commands,
    // literals, keywords) must be false here.
    //
    // Returns an AttributeList mapping each referenced variable's symid (as
    // attribute name) to its Kind (stored as an IntType AttributeValue).
    // Caller owns the returned list.
    static AttributeList* classify(postfix_token* toks, int ntoks, boolean* is_plain_var);

    // Builds a fresh is_plain_var[] array for classify() above, resolving
    // each token the same way ordinary evaluation would (token_to_comvalue
    // is public specifically for this, see its own comment in comterp.h).
    // Shared by #310's declaration-time capture (postfunc.c) and #170's
    // :help fire-time analysis (comterp.c) so both go through one
    // implementation of the nids<0 dot-rhs exclusion (HACKING.md's "Dot
    // Operator Rhs" section) rather than two copies drifting apart. Caller
    // owns the returned array (delete []).
    static boolean* build_is_plain_var(ComTerp* comterp, postfix_token* toks, int ntoks);

    // Positional arg(n) usage, derived separately from classify() above --
    // that's keyword-symbol classification only, with no notion of arg(n)
    // at all.
    struct PositionalInfo {
        long count;        // -1 means "count could not be pinned down statically";
                            // long (not int) so a literal index near INT_MAX
                            // doesn't overflow computing count = maxidx + 1
                            // (Greptile, PR #337)
        boolean uses_narg; // true if narg() appears anywhere in the body --
                            // treated as a signal the body is variadic
                            // (loops over an arg(n) run bounded by narg()),
                            // so count is forced to -1 regardless of any
                            // literal arg(n) indices also found.
    };

    // Scans for arg(n) calls, deriving the positional count from the
    // highest literal index referenced (max constant index + 1).  A
    // non-literal index (e.g. arg(i)) makes the count unresolvable the same
    // way narg() usage does -- see #170 phase 1 point 2's "attempt simple
    // computed n, fall back to dynamic" allowance; this first pass only
    // resolves literal indices, computed-index resolution is future work.
    static PositionalInfo scan_positionals(postfix_token* toks, int ntoks);

    // #336 (staged from #170's "Future" section, "Positional optionality"):
    // recognizes the canonical "unsupplied keyword defaults to nil" idiom --
    // if(x==nil :then DEFAULT :else x) -- and extracts DEFAULT where it's a
    // single literal token. Only this one, well-known shape is matched
    // (condition is exactly "x==nil"/"nil==x", the :else branch is exactly
    // the bare keyword unchanged, the :then branch is exactly one literal
    // token); anything more elaborate -- a computed default, extra
    // keywords on the if(), a differently-shaped condition -- is silently
    // skipped rather than guessed at, the same "attempt simple cases, give
    // up gracefully" restraint scan_positionals uses for computed arg(n)
    // indices. Needs ComTerp access (token_to_comvalue) to turn the
    // literal token into a real ComValue, unlike classify()/
    // scan_positionals() above.
    //
    // Returns an AttributeList mapping each keyword's symid (only those
    // with a recognized default) to its default ComValue. Always non-nil,
    // possibly empty. Caller owns the returned list.
    static AttributeList* scan_defaults(ComTerp* comterp, postfix_token* toks, int ntoks, boolean* is_plain_var);
};

#endif /* !defined(_funcobjscan_h) */
