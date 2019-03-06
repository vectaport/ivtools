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

#ifndef timeglyph_h
#define timeglyph_h

#include <InterViews/monoglyph.h>
#include <InterViews/observe.h>

class BoundedValue;
class StrListValue;
class Style;
class ObservableTime;
class DragValuator;

class TimeGlyph : public MonoGlyph, public Observer {
public:
    TimeGlyph(Style*, ObservableTime* = nil, boolean editable = false);

    void time(Time*);
    virtual void update(Observable*);
protected:
    void addtimesdelta();
    void subtimesdelta();
    void updatevalues();
    ObservableTime* time_;
    StrListValue* wdayvalue;
    StrListValue* monthvalue;
    BoundedValue* mdayvalue;
    BoundedValue* yearvalue;
    BoundedValue* hourvalue;
    BoundedValue* minutevalue;
    BoundedValue* secondvalue;
    StrListValue* deltavalue;
    BoundedValue* timesvalue;
    DragValuator* wdayv;
    DragValuator* monthv;
    DragValuator* mdayv;
    DragValuator* yearv;
    DragValuator* hourv;
    DragValuator* minutev;
    DragValuator* secondv;
};

#endif
