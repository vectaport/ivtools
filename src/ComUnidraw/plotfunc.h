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

#if !defined(_plotfunc_h)
#define _plotfunc_h

#include <ComUnidraw/unifunc.h>

//: command to plot a barchart in comdraw using plotmtv/pstoedit
// barplot([var_str value_float] [...] :title str :xtitle str :ytitle str :valtitle str :newview) -- display a barplot
class BarPlotFunc : public UnidrawFunc {
public:
    BarPlotFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([var_str value_float] [...] :title str :xtitle str :ytitle str :valtitle str :newview) -- display a barplot"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":title str             plot title",
	":xtitle str            x axis title",
	":ytitle str            y axis title",
	":valtitle str          value title",
	":newview               create new viewer",
	nil
      };
      return keys;
    }
};

#endif /* !defined(_plotfunc_h) */
