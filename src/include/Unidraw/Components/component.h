/*
 * Copyright (c) 1990, 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Component - class of objects that are edited to 
 * form domain-specific drawings. Component subjects contain structural,
 * connectivity, constraint, and transfer function information.
 */

#ifndef unidraw_components_component_h
#define unidraw_components_component_h

#include <Unidraw/globals.h>

#ifndef UnidrawCommon
class Command;
#endif
class ComponentView;
class Iterator;
#ifndef UnidrawCommon
class StateVar;
class TransferFunct;
#endif
class UList;
#include <iosfwd>

//: base class for objects that model domain specific elements.
// <a href=../man3.1/Component.html>man page</a>
class Component {
public:
    virtual void Update();
    virtual void Notify();
#ifndef UnidrawCommon
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);
#endif

    virtual Component* GetParent();
    virtual Component* GetRoot();
#ifndef UnidrawCommon
    virtual StateVar* GetState(const char*);
    virtual TransferFunct* GetTransferFunct();
#endif

    virtual void First(Iterator&);
    virtual void Last(Iterator&);
    virtual void Next(Iterator&);
    virtual void Prev(Iterator&);
    virtual boolean Done(Iterator);

    virtual void Attach(ComponentView*);
    virtual void Detach(ComponentView*);
#ifndef UnidrawCommon
    ComponentView* Create(ClassId);
#endif

    virtual ~Component();
    virtual Component* Copy();
    virtual void Read(istream&);
    virtual void Write(ostream&);
    virtual ClassId GetClassId();
    virtual ClassId GetSubstId(const char*& delim);
    virtual boolean IsA(ClassId);

    static boolean use_unidraw() { return _use_unidraw; }
    static void use_unidraw(boolean flag) { _use_unidraw = flag; }

    UList* ViewList() { return _views; }
    ComponentView* View(UList*);
protected:
    Component();

    virtual void SetParent(Component* child, Component* parent);
protected:
    UList* _views;

    static boolean _use_unidraw;
};

#endif
