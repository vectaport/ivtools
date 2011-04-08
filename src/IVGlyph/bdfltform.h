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

#ifndef bdfltform_h
#define bdfltform_h

#include <InterViews/monoglyph.h>
#include <InterViews/observe.h>

class BoundedValue;
class InputHandler;
class Patch;
class Valuator;

class BoundedValueEditor : public MonoGlyph {
public:
    BoundedValueEditor(BoundedValue*, char* labl =nil, boolean scr =true);
    virtual ~BoundedValueEditor();

    InputHandler* focusable();
    void accept();
protected:
    Valuator* val;
};

class BoundedValueObserver : public MonoGlyph, public Observer {
public:
    BoundedValueObserver(BoundedValue*, char* labl);
    virtual ~BoundedValueObserver();

    virtual void update(Observable*);
protected:
    Patch* _view;
    BoundedValue* _value;
};

class MeterObserver : public MonoGlyph, public Observer {
public:
    MeterObserver(BoundedValue*, char* label = "", boolean int_display = true);
    virtual ~MeterObserver();

    virtual void update(Observable*);

protected:
    Patch* _view;
    BoundedValue* _value;
    boolean _int_display;
};

#endif
