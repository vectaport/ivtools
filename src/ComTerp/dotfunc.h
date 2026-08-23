/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
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
 * dot func supports a compound variable
 */

#if !defined(_dotfunc_h)
#define _dotfunc_h

#include <ComTerp/numfunc.h>
#include <string>

//: . (dot) operator, for compound variables | dotlst=dot(name) -- construct empty dottted pair list.
// obj.method(args) also fires a FuncObj-valued attribute self-bound to
// obj, evaluating args in the caller's own scope first -- see execute().
class DotFunc : public ComFunc {
public:
    DotFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "%s (.) makes compound variables | dotlst=dot(name) -- construct empty dotted pair list"; }


protected:
    /* Peek both raw args, firing arg 0 if it's an unfired nested command
       reference (e.g. the inner dot of node.left.val, or at(grid(:table))
       in general).  Split out of execute() so a post_eval-aware subclass
       (GrDotFunc, which needs to inspect/rewrite before_part -- unwrapping
       a ComponentView -- before the shared dispatch below runs) can call
       this itself instead of re-peeking the stack, which would either see
       stale post_eval() args or double-fire arg 0.  Must run before
       reset_stack(). */
    void peek_and_fire(ComValue& before_part, ComValue& after_raw, int& after_nids,
			std::string& before_expr_text, std::string& after_expr_text);
    /* Everything after peek_and_fire(): the before/after validity checks,
       attrlist lookup-or-create, and the actual dispatch (attribute
       fetch, or obj.method(args) self-bound firing). force_named_field,
       default false, lets a caller that already knows after_raw is a
       genuine field symbol (DotStreamNextFunc::execute(), #304) skip the
       nargs()>1 gate that distinguishes "al.field" from the rare 1-arg
       dot(name) form -- nargs() there reflects THIS invocation (always 1,
       the driving stream itself), not the original dot-expression's own
       arg count, so it can't be relied on for a synthetic per-pull call. */
    void execute_core(ComValue before_part, ComValue after_raw, int after_nids,
		       const std::string& before_expr_text, const std::string& after_expr_text,
		       boolean force_named_field = false);
    /* Get/set the debug-expr flag at runtime via a :dbg keyword --
       intentionally not in DotFunc's public docstring above: a malformed-
       dot warning already shows both sides' resolved values unconditionally
       (see execute_core), so this only adds a raw postfix-token dump on
       top of that for tracking down something the resolved value alone
       doesn't explain -- a fallback for a future maintainer chasing a
       stranger case, not a supported end-user feature. Returns true if
       this call WAS a :dbg request (already handled -- result pushed,
       stack reset, caller should return immediately); false for an
       ordinary dot expression, which the caller should dispatch normally.
       A subclass that overrides execute() entirely instead of calling
       DotFunc::execute() (GrDotFunc, which needs to unwrap a ComponentView
       before the shared dispatch runs) must call this itself at its own
       top, or dot(:dbg true) silently never reaches it and always
       misfires as a malformed dot expression instead. */
    boolean check_dbg_keyword();
};

//: hidden func used by next() to drive a lazy (stream).field access (#304).
// Holds, in its own stream's stream_list(): [0] the underlying before-
// stream, [1] the fixed after-dot field symbol. Each pull advances the
// before-stream one element and re-runs DotFunc's own dispatch
// (execute_core) on that element, exactly reproducing what an ordinary,
// non-streaming bare field access would do for it -- same Attribute
// wrapping, same auto-vivify-if-missing behavior, no separate field-
// lookup logic duplicated here. A DotFunc subclass (not StrmFunc, unlike
// its next-func siblings) specifically so it can call the protected
// execute_core() it inherits, the same way GrDotFunc already does.
class DotStreamNextFunc : public DotFunc {
public:
    DotStreamNextFunc(ComTerp*);

    virtual void execute();
    /* Override DotFunc's post_eval()==true back to false: this is
       invoked directly via exec(1,0) by the "next" stream-driving
       mechanism (NextFunc::execute_impl and friends), the same ordinary
       calling convention every other *NextFunc sibling uses -- not
       through real post-eval dispatch, so there's no valid postfix-
       buffer bookmark for stack_arg_post()'s offset arithmetic to read.
       Left at DotFunc's inherited true, stack_arg(0) silently read
       garbage memory instead (confirmed via lldb: a SIGBUS inside the
       ComValue this constructed from that garbage). */
    virtual boolean post_eval() { return false; }
    virtual const char* docstring() {
      return "hidden func used by next command for (stream).field dot access (#304)."; }

};

//: name returns name field of a dotted pair
//  attrname(attribute) returns name field of a dotted pair
class DotNameFunc : public ComFunc {
public:
    DotNameFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "%s(attribute) returns name field of a dotted pair (also accepts a single-entry attrlist, e.g. al@n)"; }
};

//: value returns value field of a dotted pair
// attrval(attribute) returns value field of a dotted pair
class DotValFunc : public ComFunc {
public:
    DotValFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "%s(attribute) returns value field of a dotted pair (also accepts a single-entry attrlist, e.g. al@n)"; }
};
#endif /* !defined(_dotfunc_h) */

