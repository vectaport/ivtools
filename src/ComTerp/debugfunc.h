/*
 * Copyright (c) 2000 Vectaport Inc, IET Inc.
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
 * collection of debug functions
 */

#if !defined(_debugfunc_h)
#define _debugfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;
class ComValue;

//: command for toggling or setting trace mode
// val=trace([flag] :val) -- toggle or set trace mode
class ComterpTraceFunc : public ComFunc {
public:
    ComterpTraceFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s([flag] :val) -- toggle or set trace mode"; }
};

//: command to pause script execution until C/R
// pause -- pause script execution until C/R
class ComterpPauseFunc : public ComFunc {
public:
    ComterpPauseFunc(ComTerp*);
    virtual void execute();
    virtual void execute_body(ComValue&);
    virtual const char* docstring() { 
	return "%s -- pause script execution until C/R"; }
    virtual boolean stepfunc() { return false; }

};

//: command to toggle step script execution 
// pause -- toggle stepwise script execution
class ComterpStepFunc : public ComterpPauseFunc {
public:
    ComterpStepFunc(ComTerp*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s -- toggle stepwise script execution"; }
    virtual boolean stepfunc() { return true; }
};

//: command to toggle step script execution 
// pause -- toggle stepwise script execution
class ComterpStackHeightFunc : public ComFunc {
public:
    ComterpStackHeightFunc(ComTerp*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s -- return stack height for debug purposes"; }
};

#endif /* !defined(_debugfunc_h) */
