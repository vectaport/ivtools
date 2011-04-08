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

#include <IVGlyph/bdvalue.h>
#include <stdio.h>

/* class BoundedValue */

BoundedValue::BoundedValue() 
:Adjustable()
{
    scroll_incr_ = 0.0;
    page_incr_ = 0.0;
}

BoundedValue::BoundedValue(float lower, float upper,
			   float scroll, float page, float curvalue,
			   char* format, float* p)
:Adjustable()
{
    lower_ = lower;
    span_ = upper - lower;
    scroll_incr_ = scroll;
    page_incr_ = page;
    curvalue_ = curvalue;
    format_ = format;
    ptr = p;
}

BoundedValue::~BoundedValue() { }

void BoundedValue::lower_bound(float c) { lower_ = c; }
void BoundedValue::upper_bound(float c) { span_ = c - lower_; }

void BoundedValue::current_value(float value, DimensionName d) {
    curvalue_ = value;
    constrain(d, curvalue_);
    notify(Dimension_X);
    notify(Dimension_Y);
}

void BoundedValue::scroll_incr(float c) { scroll_incr_ = c; }
void BoundedValue::page_incr(float c) { page_incr_ = c; }

float BoundedValue::lower(DimensionName) const { return lower_; }
float BoundedValue::upper(DimensionName) const { return lower_ + span_; }
float BoundedValue::length(DimensionName) const { return span_; }
float BoundedValue::cur_lower(DimensionName) const { return curvalue_; }
float BoundedValue::cur_upper(DimensionName) const { return curvalue_; }
float BoundedValue::cur_length(DimensionName) const { return 0; }

void BoundedValue::scroll_to(DimensionName d, float position) {
    float p = position;
    constrain(d, p);
    if (p != curvalue_) {
	curvalue_ = p;
	notify(Dimension_X);
	notify(Dimension_Y);
    }
}

void BoundedValue::scroll_forward(DimensionName d) {
    scroll_to(d, curvalue_ + scroll_incr_);
}

void BoundedValue::scroll_backward(DimensionName d) {
    scroll_to(d, curvalue_ - scroll_incr_);
}

void BoundedValue::page_forward(DimensionName d) {
    scroll_to(d, curvalue_ + page_incr_);
}

void BoundedValue::page_backward(DimensionName d) {
    scroll_to(d, curvalue_ - page_incr_);
}

const char* BoundedValue::valuestring() {
    static char buf[20];
    sprintf(buf, format_, curvalue_);
    return buf;
}

void BoundedValue::accept() {
    if (ptr)
	*ptr = curvalue_;
}

/* class StrListValue */

StrListValue::StrListValue(StringList* sl, int cur, char** sp)
: BoundedValue(0, sl->count()-1, 1, 1, cur)
{
    list_ = sl;
    strptr = sp;
}

StrListValue::~StrListValue() {}

String StrListValue::current() {
    return list_->item((long)curvalue_);
}

const char* StrListValue::valuestring() {
    static char buf[40];
    sprintf(buf, "%s", current().string());
    return buf;
}

void StrListValue::accept() {
    if (strptr)
	strcpy(*strptr, current().string());
}
