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
 * collection of statistical functions
 */

#if !defined(_statfunc_h)
#define _statfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;

//: sum-of-values command for ComTerp.
// val=sum(val1[,val2[,...,valn]]) -- return sum of values.
class SumFunc : public ComFunc {
public:
    SumFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s(val1[,val2[,...,valn]]) -- return sum of values"; }

protected:
    boolean _meanfunc;

};

//: mean-of-values command for ComTerp.
// val=mean(val1[,val2[,...,valn]]) -- return mean of values.
class MeanFunc : public SumFunc {
public:
    MeanFunc(ComTerp*);

    virtual const char* docstring() { 
      return "val=%s(val1[,val2[,...,valn]]) -- return mean of values"; }

};

//: variance-of-values command for ComTerp.
// val=var(val1[,val2[,...,valn]]) -- return variance of values.
class VarFunc : public ComFunc {
public:
    VarFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s(val1[,val2[,...,valn]]) -- return variance of values"; }

protected:
    boolean _stddevfunc;

};

//: standard-deviation-of-values command for ComTerp.
// val=stddev(val1[,val2[,...,valn]]) -- return standard deviation of values.
class StdDevFunc : public VarFunc {
public:
    StdDevFunc(ComTerp*);

    virtual const char* docstring() { 
      return "val=%s(val1[,val2[,...,valn]]) -- return standard deviation of values"; }

};

#endif /* !defined(_statfunc_h) */
