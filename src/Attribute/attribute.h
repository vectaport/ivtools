/*
 * Copyright (c) 1996,1999 Vectaport Inc.
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

#ifndef _attribute_h
#define _attribute_h

#include <Attribute/classid.h>

class AttributeValue;
class AttributeList;

//: generic symbol/value pair.
// Attribute consists of a symbol, represented by its index into a symbol table,
// paired with a value, represented by an AttributeValue.  Memory for the
// AttributeValue is owned by the Attribute.
class Attribute {
public:
    Attribute(const char* name =0, AttributeValue* value =0);
    // construct an attribute by generating a symbol id for the 'name'
    // character string, and accepting a pointer to an externally allocated 'value'.
    Attribute(int symid, AttributeValue* value =0);
    // construct an attribute with a symbol id instead of a character string, 
    // and accepting a pointer to an externally allocated 'value'.

    Attribute(const Attribute&);
    // copy constructor.

    ~Attribute();
    // deallocate memory for internal AttributeValue.

    char* Name();
    // look up and return symbol string.
    void Value(AttributeValue*);
    // accept new pointer to an externally allocated AttributeValue,
    // deleting the old pointer.
    AttributeValue* Value();
    // return a pointer to the internal AttributeValue.
    int SymbolId();
    // return the id of the symbol in the symbol table.

protected:
    int symbolid;
    AttributeValue* valueptr;

friend AttributeList;

    CLASS_SYMID("Attribute"); 
};

#endif
