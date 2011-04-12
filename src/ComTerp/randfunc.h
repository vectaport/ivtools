/*
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
 * collection of random number generating functions
 */

#if !defined(_randfunc_h)
#define _randfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;

//: random-number command for ComTerp.
// val=rand([minval,maxval]) -- return random number between 0 and 1 or minval,maxval.
class RandFunc : public ComFunc {
public:
    RandFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s([minval,maxval]) -- return random number between 0 and 1 or minval,maxval"; }

    static double drand(double minval, double maxval);

};

//: command to seed ComTerp random number generator.
// srand(seedval) -- seed random number generator.
class SRandFunc : public ComFunc {
public:
    SRandFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(seedval) -- seed random number generator"; }

};

#endif /* !defined(_randfunc_h) */
