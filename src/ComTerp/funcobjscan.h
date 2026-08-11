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
};

#endif /* !defined(_funcobjscan_h) */
