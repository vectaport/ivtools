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

#ifndef textform_h
#define textform_h

#include <InterViews/monoglyph.h>
#include <IVGlyph/observables.h>

class CharFieldEditor;
class FieldEditor;
class InputHandler;
class Patch;

class ObsTextEditor : public MonoGlyph, public Observer {
public:
    ObsTextEditor(ObservableText*, char* labl =nil, int width =125);
    virtual ~ObsTextEditor();

    InputHandler* focusable() const;
    virtual void update(Observable*);
protected:
    virtual void accept_editor(FieldEditor*);
    virtual void cancel_editor(FieldEditor*);
    CharFieldEditor* _editor;
    ObservableText* _obs;
};

class TextObserver : public MonoGlyph, public Observer {
public:
    TextObserver(ObservableText*, char* labl, int max=30);
    virtual ~TextObserver();

    virtual void update(Observable*);
protected:
    ObservableText* _obs;
    Patch* _view;
};

#endif
