/*
 * Copyright (c) 1994 Vectaport Inc.
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
 * Copyright (c) 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */
/* derived from idemo/main.c */

#ifndef bdvalue_h
#define bdvalue_h

#include <InterViews/adjust.h>
#include <IVGlyph/strlist.h>

class BoundedValue : public Adjustable {
protected:
    BoundedValue();
public:
    BoundedValue(float lower, float upper, float scroll, float page,
		 float curvalue, char* format ="%.6f", float* p =nil);
    virtual ~BoundedValue();

    virtual void lower_bound(float);
    virtual void upper_bound(float);
    virtual void current_value(float, DimensionName d = Dimension_X);
    virtual void scroll_incr(float);
    virtual void page_incr(float);

    virtual float lower(DimensionName) const;
    virtual float upper(DimensionName) const;
    virtual float length(DimensionName) const;
    virtual float cur_lower(DimensionName) const;
    virtual float cur_upper(DimensionName) const;
    virtual float cur_length(DimensionName) const;

    virtual void scroll_to(DimensionName, float position);
    virtual void scroll_forward(DimensionName);
    virtual void scroll_backward(DimensionName);
    virtual void page_forward(DimensionName);
    virtual void page_backward(DimensionName);

    void scrollfwdX() { scroll_forward(Dimension_X); }
    void scrollbwdX() { scroll_backward(Dimension_X); }

    virtual const char* valuestring();
    const char* format() { return format_; }

    virtual void accept();
protected:
    float curvalue_;
    float lower_;
    float span_;
    float scroll_incr_;
    float page_incr_;
    float* ptr;
    char* format_;
};

class StrListValue : public BoundedValue {
public:
    StrListValue(StringList*, int cur=0, char** sp =nil);
    virtual ~StrListValue();
    String current();
    virtual const char* valuestring();

    virtual void accept();
protected:
    StringList* list_;
    char** strptr;
};

#endif
