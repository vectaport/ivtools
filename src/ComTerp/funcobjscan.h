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
// body's token span as read-only, read-before-write, write-before-read, or
// escaping (local()/global()).  One classifier, two consumers: declaration-
// time capture for func() closures, and help-contract derivation.
//
// Built on PostfixSpanWalk for the traversal.  It does no symbol-table lookup
// and needs no ComTerp access: the caller has already resolved each token's
// symbol-vs-command status via ComTerp::token_to_comvalue and passes the
// result in as is_plain_var[], so this stays a pure structural pass.
class FuncObjVarScan {
public:
    enum Kind { ReadOnly, ReadBeforeWrite, WriteBeforeRead, EscapingLocal, EscapingGlobal };

    // is_plain_var[i] must be true iff toks[i] is a bare, zero-arg symbol
    // reference (TOK_COMMAND, narg==0, nkey==0) resolving to an ordinary
    // variable rather than a registered command -- what token_to_comvalue
    // leaves as SymbolType instead of promoting to CommandType.  Every other
    // token must be false.
    //
    // Returns an AttributeList mapping each variable's symid to its Kind, as
    // an IntType value.  Caller owns the list.
    static AttributeList* classify(postfix_token* toks, int ntoks, boolean* is_plain_var);

    // builds a fresh is_plain_var[] for classify(), resolving each token the
    // way ordinary evaluation would.  Shared by declaration-time capture and
    // help-contract analysis so both use one implementation of the nids<0
    // dot-rhs exclusion rather than two that can drift.  Caller owns the
    // returned array (delete []).
    static boolean* build_is_plain_var(ComTerp* comterp, postfix_token* toks, int ntoks);

    // Positional arg(n) usage, derived separately from classify() above --
    // that's keyword-symbol classification only, with no notion of arg(n)
    // at all.
    struct PositionalInfo {
        long count;        // -1 means "count could not be pinned down statically";
                            // long, not int, so a literal index near INT_MAX
                            // does not overflow computing maxidx + 1
        boolean uses_narg; // true if narg() appears anywhere in the body --
                            // treated as a signal the body is variadic
                            // (loops over an arg(n) run bounded by narg()),
                            // so count is forced to -1 regardless of any
                            // literal arg(n) indices also found.
    };

    // scans for arg(n) calls, deriving the positional count from the highest
    // literal index referenced.  A non-literal index (arg(i)) makes the count
    // unresolvable, the same as narg() usage does; only literal indices are
    // resolved here.
    static PositionalInfo scan_positionals(postfix_token* toks, int ntoks);

    // recognizes the canonical "unsupplied keyword defaults to nil" idiom,
    // if(x==nil :then DEFAULT :else x), and extracts DEFAULT where it is a
    // single literal token.  Only that exact shape is matched -- a computed
    // default, extra keywords on the if(), a differently shaped condition are
    // skipped rather than guessed at.  Needs ComTerp access to turn the
    // literal token into a ComValue, unlike the two above.
    //
    // Returns an AttributeList mapping each keyword's symid to its default,
    // for those with a recognized one.  Always non-nil, possibly empty.
    // Caller owns the list.
    static AttributeList* scan_defaults(ComTerp* comterp, postfix_token* toks, int ntoks, boolean* is_plain_var);
};

#endif /* !defined(_funcobjscan_h) */
