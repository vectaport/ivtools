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

#ifndef bdtable_h
#define bdtable_h

#include <IVGlyph/bdvalue.h>
#include <IVGlyph/enumform.h>

#include <InterViews/observe.h>
#include <InterViews/patch.h>

#include <OS/list.h>

declarePtrList(BoundedValueList,BoundedValue)

class PolyGlyph;

class BoundedValueTable : public ObservableEnum, public Observer {
public:
    BoundedValueTable(StringList*, BoundedValueList*);
    virtual ~BoundedValueTable();

    virtual void update(Observable*);
    BoundedValue* bdvalue(int);
    void prepend(const String&, BoundedValue*);
    void append(const String&, BoundedValue*);
    void insert(long index, const String&, BoundedValue*);
    virtual void remove(long index);
protected:
    BoundedValueList* _values;
};

class BoundedValueTableEditor : public Patch, public Observer {
public:
    BoundedValueTableEditor(BoundedValueTable* bvt, char* labl);
    virtual ~BoundedValueTableEditor();

    void edit(String);
    virtual void update(Observable*);
protected:
    void build();
    char* _lab;
    BoundedValueTable* _obs;
    PolyGlyph* _labelbox;
    PolyGlyph* _editbox;
    PolyGlyph* _mainglyph;
};


#endif
