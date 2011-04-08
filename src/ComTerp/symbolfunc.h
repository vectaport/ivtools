/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1998,1999 Vectaport Inc.
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
 * collection of symbol functions
 */

#if !defined(_symbolfunc_h)
#define _symbolfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;

//: symbol id command for ComTerp.
// int|lst=symid(symbol [symbol ...]) -- return id(s) associated with symbol(s)
class SymIdFunc : public ComFunc {
public:
    SymIdFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "int|lst=%s(symbol [symbol ...]) -- return id(s) associated with symbol(s)"; }
};


//: symbol command for ComTerp.
// sym|lst=symbol(symid [symid ...]) -- return symbol(s) associated with integer id(s)
class SymbolFunc : public ComFunc {
public:
    SymbolFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() { 
      return "sym|lst=%s(symid [symid ...]) -- return symbol(s) associated with integer id(s)"; }
};

//: lookup symbol value command for ComTerp.
// val|lst=symval(symbol_var [symbol_var ...]) -- return value(s) associated with symbol variable(s)
class SymValFunc : public ComFunc {
public:
    SymValFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() { 
      return "val|lst=%s(symbol_var [symbol_var ...]) -- return value(s) associated with symbol variables(s)"; }
};

//: create symbol command for ComTerp.
// sym|lst=symadd(symbol [symbol ...]) -- create symbol(s) and return without lookup
class SymAddFunc : public ComFunc {
public:
    SymAddFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "sym|lst=%s(symbol [symbol ...]) -- create symbol(s) and return without lookup"; }
};

//: command to split a symbol or string into a list of character objects
// lst=split(symbol|string) -- split symbol or string into list of characters.
class SplitStrFunc : public ComFunc {
public:
    SplitStrFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() { 
      return "lst=%s(symbol|string) -- split symbol or string into list of characters"; }
};

//: command to join list of characters into a string object
// str=join(clist) -- join list of characters into string
class JoinStrFunc : public ComFunc {
public:
    JoinStrFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() { 
      return "str=%s(clist) -- join list of characters into string"; }
};


//: command to make assign a global variable
// val=global(symbol)|global(symbol)=val -- make symbol global
class GlobalSymbolFunc : public ComFunc {
public:
    GlobalSymbolFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() { 
      return "sym=%s(symbol)|global(symbol)=val -- make symbol global"; }
};


#endif /* !defined(_symbolfunc_h) */
