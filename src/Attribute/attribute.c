/*
 * Copyright (c) 1996 Vectaport Inc.
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

#include <Attribute/attribute.h>
#include <Attribute/attrvalue.h>

/*****************************************************************************/

Attribute::Attribute(const char* name, AttributeValue* value) {
    if (name)
	symbolid = symbol_add((char *)name);
    else
	symbolid = -1;
    valueptr = value;
}

Attribute::Attribute(int symid, AttributeValue* value) {
  symbolid = symid;
  valueptr = value;
}

Attribute::Attribute(const Attribute& attr) {
    symbolid = attr.symbolid;
    if (symbolid != -1) // for reference count
	symbol_add(symbol_pntr(symbolid));
    valueptr = new AttributeValue(*attr.valueptr);
}

Attribute::~Attribute() {
    if (0 && symbolid != -1)  // need to rewrite symbol table
	symbol_del(symbolid);
    delete valueptr;
}

char* Attribute::Name() {
    return symbol_pntr(symbolid);
}

void Attribute::Value(AttributeValue* value) {
    delete valueptr;
    valueptr = value;
}

AttributeValue* Attribute::Value() {
    return valueptr;
}

int Attribute::SymbolId() {
    return symbolid;
}
