/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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
 * ComFunc is a simple action callback pointed to by a ComCom
 * (or possibly any other external function binding mechanism)
 */

#if !defined(_comfunc_h)
#define _comfunc_h

#include <stdlib.h>
#include <OS/types.h>
#include <ComTerp/comvalue.h>

class ComTerp;
class ComTerpServ;

class ComFunc {
public:
    ComFunc(ComTerp*);
    virtual ~ComFunc() {}

    virtual void execute() = 0;

    void argcnts(int narg, int nkey, int nargskey=0);
    int nargs() {return _nargs;}

    int nargsfixed() {return _nargs - _nargskey;}
    int nkeys() {return _nkeys;}
    int nargskey() {return _nargskey;}

    ComValue& pop_stack();
    ComValue& pop_symbol();
    void push_stack(ComValue&);
    void reset_stack();

    ComValue& stack_arg(int n, boolean symbol=false, 
			ComValue& dflt=ComValue::nullval());
    ComValue& stack_key(int id, boolean symbol=false, 
			ComValue& dflt=ComValue::trueval(), boolean always=false);
    ComValue& stack_dotname(int n);

    ComValue& lookup_symval(ComValue&);
    void assign_symval(int id, ComValue*);

    ComTerp* comterp() { return _comterp; }
    ComTerpServ* comterpserv() { return (ComTerpServ*)_comterp; }

    virtual boolean lazy_eval() { return false; }
    virtual const char* docstring() { return "%s: no docstring method defined"; }
    static int bintest(const char* name);
    static boolean bincheck(const char* name);

protected:
    ComTerp* _comterp;
    int _nargs;
    int _nkeys;
    int _nargskey;
    int _npops;
};

#endif /* !defined(_comfunc_h) */
