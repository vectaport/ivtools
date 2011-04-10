/*
 * Copyright (c) 1996-1997 Vectaport Inc.
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
 * AttributeList - a list of attributes
 */

#ifndef attr_list_h
#define attr_list_h

#include <OS/enter-scope.h>
#include <InterViews/resource.h>

#ifndef ALITERATOR
#define ALIterator _lib_iv(Iterator)
#define AList _lib_iv(UList)
#endif

class ALIterator;
class AList;
class istream;
class ostream;

class Attribute;
class AttributeValue;
class ParamStruct;

class AttributeList : public Resource {
public:
    AttributeList(AttributeList* = nil);
    virtual ~AttributeList();

    void add_attr(const char* name, AttributeValue& value); // copies value
    void add_attr(const char* name, AttributeValue* value); // uses value
    int add_attr(Attribute* attr);

public:
    void First(ALIterator&);
    void Last(ALIterator&);
    void Next(ALIterator&);
    void Prev(ALIterator&);
    boolean Done(ALIterator);
    boolean IsEmpty();
    int Number();

    void Append(Attribute*);
    void Prepend(Attribute*);
    void InsertAfter(ALIterator, Attribute*);
    void InsertBefore(ALIterator, Attribute*);
    void Remove(Attribute*);
    void Remove(ALIterator&);

    Attribute* GetAttr(const char*);
    Attribute* GetAttr(ALIterator);
    void SetAttr(Attribute*, ALIterator&);
    boolean Includes(Attribute*);

    Attribute* Attr(AList*);
    AList* Elem(ALIterator);

    friend ostream& operator << (ostream& s, const AttributeList&);

  void dump();

    AttributeValue* find(const char*);
    AttributeValue* find(int);

protected:
    AList* _alist;
    unsigned int _count;
};

class AttributeValueList : public Resource {
public:
    AttributeValueList(AttributeValueList* = nil);
    virtual ~AttributeValueList();

public:
    void First(ALIterator&);
    void Last(ALIterator&);
    void Next(ALIterator&);
    void Prev(ALIterator&);
    boolean Done(ALIterator);
    boolean IsEmpty();
    int Number();

    void Append(AttributeValue*);
    void Prepend(AttributeValue*);
    void InsertAfter(ALIterator, AttributeValue*);
    void InsertBefore(ALIterator, AttributeValue*);
    void Remove(AttributeValue*);
    void Remove(ALIterator&);

    AttributeValue* GetAttrVal(ALIterator);
    void SetAttrVal(AttributeValue*, ALIterator&);
    boolean Includes(AttributeValue*);

    AttributeValue* AttrVal(AList*);
    AList* Elem(ALIterator);

    friend ostream& operator << (ostream& s, const AttributeValueList&);

protected:
    AList* _alist;
    unsigned int _count;
};

#endif
