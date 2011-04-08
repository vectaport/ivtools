/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

#ifndef enumform_h
#define enumform_h

#include <InterViews/monoglyph.h>
#include <IVGlyph/observables.h>
#include <InterViews/patch.h>
#include <IVGlyph/strlist.h>

class Button;
class Deck;
class InputHandler;
class Menu;
class Macro;
class PolyGlyph;
class TelltaleGroup;

class EnumObserver : public MonoGlyph, public Observer {
public:
    EnumObserver(ObservableEnum*, char* labl, Coord minwid = 150);
    virtual ~EnumObserver();

    virtual void update(Observable*);
protected:
    Coord minw;
    ObservableEnum* _obs;
    Patch* _view;
};

class MenuEnumEditor : public Patch {
public:
    MenuEnumEditor(ObservableEnum* obs, Macro* m = nil);
    virtual ~MenuEnumEditor();

    void edit(String);
protected:
    MenuEnumEditor();
    Glyph* buildmenu();
    void build();
    Menu* _menu;
    ObservableEnum* _obs;
    EnumObserver* _enumobs;
    Macro* _macro;
};

class RadioEnumEditor : public Patch, public Observer {
public:
    RadioEnumEditor(ObservableEnum* obs, char* labl);
    virtual ~RadioEnumEditor();

    void edit(String);
    virtual void update(Observable*);
protected:
    RadioEnumEditor();
    void buildbox();
    void build();
    PolyGlyph* _buttonbox;
    PolyGlyph* mainglyph;
    TelltaleGroup* _group;
    ObservableEnum* _obs;
    char* lab;
};

class CycleEnumEditor : public MonoGlyph, public Observer {
public:
    CycleEnumEditor(ObservableEnum* obs, char* labl);
    virtual ~CycleEnumEditor();

    void cycle();
    void bkcycle();
    void up();
    void down();
    virtual void update(Observable*);
protected:
    Deck* _values;
    Patch* _view;
    ObservableEnum* _obs;
};

/*
 * EnumAction denoted by an object and member function to call on the object.
 */

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __EnumActionCallback(T) T##_EnumActionCallback
#define EnumActionCallback(T) __EnumActionCallback(T)
#define __EnumActionMemberFunction(T) T##_EnumActionMemberFunction
#define EnumActionMemberFunction(T) __EnumActionMemberFunction(T)
#else
#define __EnumActionCallback(T) T/**/_EnumActionCallback
#define EnumActionCallback(T) __EnumActionCallback(T)
#define __EnumActionMemberFunction(T) T/**/_EnumActionMemberFunction
#define EnumActionMemberFunction(T) __EnumActionMemberFunction(T)
#endif

#define declareEnumActionCallback(T) \
typedef void (T::*EnumActionMemberFunction(T))(String); \
class EnumActionCallback(T) : public Action { \
public: \
    EnumActionCallback(T)(T*, EnumActionMemberFunction(T), const String&); \
\
    virtual void execute(); \
private: \
    T* obj_; \
    EnumActionMemberFunction(T) func_; \
    String i; \
};

#define implementEnumActionCallback(T) \
EnumActionCallback(T)::EnumActionCallback(T)(T* obj, EnumActionMemberFunction(T) func, const String& ival) { \
    obj_ = obj; \
    func_ = func; \
    i = ival; \
} \
\
void EnumActionCallback(T)::execute() { (obj_->*func_)(i); }

#endif
