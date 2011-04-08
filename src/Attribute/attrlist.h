/*
 * Copyright (c) 1996-1999 Vectaport Inc.
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
#include <Attribute/classid.h>

#ifndef ALITERATOR
#define ALIterator _lib_iv(Iterator)
#define AList _lib_iv(UList)
#endif

class ALIterator;
class AList;
#include <iosfwd>

class Attribute;
class AttributeValue;
class ParamStruct;

//: list of Attribute objects, i.e. a property list.
// An AttributeList is derived from Resource, so it is a reference-counted
// object that can be freely shared between other objects.
//
// An AttributeList assumes responsibility for the memory of its member
// Attribute objects, which in turn assume responsibility for the memory
// of their member AttributeValue objects.
class AttributeList : public Resource {
public:
    AttributeList(AttributeList* = nil);
    // construct with optional AttributeList to copy.
    virtual ~AttributeList();
    // do not call directly.  Frees memory of associated Attribute objects.

    void add_attr(const char* name, AttributeValue& value); 
    // add attribute by making copy of an AttributeValue.
    void add_attr(const char* name, AttributeValue* value); 
    // add attribute by using pointer to AttributeValue, assuming responsibility
    void add_attr(int symid, AttributeValue& value); 
    // add attribute by making copy of an AttributeValue.
    void add_attr(int symid, AttributeValue* value); 
    // add attribute by using pointer to AttributeValue, assuming responsibility
    // for the memory.

    void add_attribute(Attribute* attr);
    // add complete Attribute object to the list, accepting responsibility
    // for the memory of the Attribute object. 

    void First(ALIterator&);
    // set iterator to point to first Attribute in list.
    void Last(ALIterator&);
    // set iterator to point to last Attribute in list.
    void Next(ALIterator&);
    // set iterator to point to next Attribute in list.
    void Prev(ALIterator&);
    // set iterator to point to previous Attribute in list.
    boolean Done(ALIterator);
    // return true if iterator is pointing off the end of the list.
    // works for forward and backward traversals.
    boolean IsEmpty();
    // true if no Attribute objects in list.
    int Number();
    // number of Attribute objects in list.

    Attribute* GetAttr(const char*);
    // get attribute by name.
    Attribute* GetAttr(int symid);
    // get attribute by symbol id.
    Attribute* GetAttr(ALIterator);
    // get attribute pointed to by iterator.
    void SetAttr(Attribute*, ALIterator&);
    // set attribute pointed to by iterator.
    boolean Includes(Attribute*);
    // check if list includes Attribute by pointer-comparison.
    void Remove(Attribute*);
    // remove Attribute from list, returning responsibility for freeing the
    // associated memory.

    AList* Elem(ALIterator);
    // return AList (UList) pointed to by ALIterator (Iterator).
    Attribute* Attr(AList*);
    // return attribute pointed to by AList (UList).

    AttributeList* merge(AttributeList*);
    // merge the contents of another AttributeList into this one,
    // replicating the AttributeValue as needed.

protected:
    void Append(Attribute*);
    // append Attribute to end of list.  Could cause duplicates.
    void Prepend(Attribute*);
    // append Attribute to front of list.  Could cause duplicates.
    void InsertAfter(ALIterator, Attribute*);
    // append Attribute after position pointed by iterator.  Could cause duplicates.
    void InsertBefore(ALIterator, Attribute*);
    // append Attribute before position pointed by iterator.  Could cause duplicates.
    void Remove(ALIterator&);
    // remove Attribute pointed to by iterator from the list, 
    // returning responsibility for freeing the associated memory.
    // This requires saving a pointer to the Attribute before calling this method.


public:
    friend ostream& operator << (ostream& s, const AttributeList&);
    // print list to ostream.

    void dump();
    // utility method to call ostream output method.

    AttributeValue* find(const char*);
    // find AttributeValue by symbol.
    AttributeValue* find(int symid);
    // find AttributeValue by symbol id.


protected:
    int add_attr(Attribute* attr);
    // add attribute, returning 0 if new, -1 if it already existed.
    // When -1 is returned you need to clear the valueptr of 'attr' before 
    // deleting it.  That's why this is protected.

    AList* _alist;
    unsigned int _count;

    CLASS_SYMID("AttributeList");
};

//: list of AttributeValue objects.
// An AttributeValueList is derived from Resource, so it is a reference-counted
// object that can be freely shared between other objects.
//
// An AttributeValueList assumes responsibility for the memory of its member
// AttributeValue objects.
class AttributeValueList : public Resource {
public:
    AttributeValueList(AttributeValueList* = nil);
    // construct with optional AttributeValueList to copy.
    virtual ~AttributeValueList();
    // do not call directly.  Frees memory of associated AttributeValue objects.

public:
    void First(ALIterator&);
    // set iterator to point to first AttributeValue in list.
    void Last(ALIterator&);
    // set iterator to point to last AttributeValue in list.
    void Next(ALIterator&);
    // set iterator to point to next AttributeValue in list.
    void Prev(ALIterator&);
    // set iterator to point to previous AttributeValue in list.
    boolean Done(ALIterator);
    // return true if iterator is pointing off the end of the list.
    // works for forward and backward traversals.
    boolean IsEmpty();
    // true if no AttributeValue objects in list.
    int Number();
    // number of AttributeValue objects in list.

    void Append(AttributeValue*);
    // append AttributeValue to end of list.
    void Prepend(AttributeValue*);
    // append AttributeValue to front of list.
    void InsertAfter(ALIterator, AttributeValue*);
    // insert AttributeValue after position pointed to by iterator.
    void InsertBefore(ALIterator, AttributeValue*);
    // insert AttributeValue before position pointed to by iterator.
    void Remove(AttributeValue*);
    // remove AttributeValue from list, returning responsibility for freeing the
    // associated memory.

    AttributeValue* GetAttrVal(ALIterator);
    // get AttributeValue pointed to by iterator.
    void SetAttrVal(AttributeValue*, ALIterator&);
    // set AttributeValue pointed to by iterator.
    boolean Includes(AttributeValue*);
    // check if list includes AttributeValue by pointer-comparison.

    AList* Elem(ALIterator); 
    // return AList (UList) pointed to by ALIterator (Iterator).
    AttributeValue* AttrVal(AList*);
    // return AttributeValue pointed to by AList (UList).

    friend ostream& operator << (ostream& s, const AttributeValueList&);
    // print list to ostream.

protected:
    void Remove(ALIterator&);
    // remove AttributeValue pointed to by iterator from the list, 
    // returning responsibility for freeing the associated memory.
    // This requires saving a pointer to the AttributeValue before calling this method.

    AList* _alist;
    unsigned int _count;
};

#endif
