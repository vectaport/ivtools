/*
 * Copyright (c) 2001 Scott E. Johnston
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
 * Implementation of AttributeList class.
 */


#include <Attribute/alist.h>
#include <Attribute/aliterator.h>
#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <Unidraw/globals.h>
#include <Unidraw/iterator.h>

#include <iostream.h>
#include <string.h>
#include <fstream>

#include <IV-2_6/_enter.h>

#ifdef LEAKCHECK
#include <leakchecker.h>
LeakChecker* AttributeValueList::_leakchecker = nil;
#endif


/*****************************************************************************/
using std::cerr;

int AttributeList::_symid = -1;

AttributeList::AttributeList (AttributeList* s) {
    _alist = new AList;
    _count = 0;
    if (s != nil) {
        ALIterator i;

        for (s->First(i); !s->Done(i); s->Next(i)) {
	    add_attr(new Attribute(*s->GetAttr(i)));
	}
    }
}

AttributeList::~AttributeList () { 
    if (_alist) {
        ALIterator i;
	for (First(i); !Done(i); Next(i)) {
	    delete GetAttr(i);
	}
	delete _alist; 
    }
}

void AttributeList::add_attr(const char* name, AttributeValue& value) {
  add_attr(name, new AttributeValue(value));
}

void AttributeList::add_attr(const char* name, AttributeValue* value) {
    Attribute* attr = new Attribute(name, value);
    if (add_attr(attr)) {
        attr->valueptr = nil;
	delete attr;
    }
}

void AttributeList::add_attr(int symid, AttributeValue& value) {
  add_attr(symid, new AttributeValue(value));
}

void AttributeList::add_attr(int symid, AttributeValue* value) {
    Attribute* attr = new Attribute(symid, value);
    if (add_attr(attr)) {
        attr->valueptr = nil;
	delete attr;
    }
}

void AttributeList::add_attribute(Attribute* attr) {
    if (add_attr(attr)) {
        attr->valueptr = nil;
	delete attr;
    }
}

int AttributeList::add_attr(Attribute* attr) {
    ALIterator i;
    for (First(i); !Done(i); Next(i)) {
	Attribute* old_attr = GetAttr(i);
	if (old_attr && attr->SymbolId() == old_attr->SymbolId()) {
	    old_attr->Value(attr->Value());
	    return -1;
	}
    }
    InsertBefore(i, attr);
    return 0;
}

Attribute* AttributeList::GetAttr (const char* n) {
    ALIterator i;
    for (First(i); !Done(i); Next(i)) {
	Attribute* attr = GetAttr(i);
	if (strcmp(n, attr->Name()) == 0)
	    return attr;
    }
    return nil;
}

Attribute* AttributeList::GetAttr (int symid) {
    ALIterator i;
    for (First(i); !Done(i); Next(i)) {
	Attribute* attr = GetAttr(i);
	if (symid == attr->SymbolId())
	    return attr;
    }
    return nil;
}

Attribute* AttributeList::Attr (AList* r) {
    return (Attribute*) (*r)();
}

AList* AttributeList::Elem (ALIterator i) { return (AList*) i.GetValue(); }

void AttributeList::Append (Attribute* v) {
    _alist->Append(new AList(v));
    ++_count;
}

void AttributeList::Prepend (Attribute* v) {
    _alist->Prepend(new AList(v));
    ++_count;
}

void AttributeList::InsertAfter (ALIterator i, Attribute* v) {
    Elem(i)->Prepend(new AList(v));
    ++_count;
}

void AttributeList::InsertBefore (ALIterator i, Attribute* v) {
    Elem(i)->Append(new AList(v));
    ++_count;
}

void AttributeList::Remove (ALIterator& i) {
    AList* doomed = Elem(i);

    Next(i);
    _alist->Remove(doomed);
    delete doomed;
    --_count;
}	
    
void AttributeList::Remove (Attribute* p) {
    AList* temp;

    if ((temp = _alist->Find(p)) != nil) {
	_alist->Remove(temp);
        delete temp;
	--_count;
    }
}

Attribute* AttributeList::GetAttr (ALIterator i) { return Attr(Elem(i)); }

void AttributeList::SetAttr (Attribute* gv, ALIterator& i) {
    i.SetValue(_alist->Find(gv));
}

void AttributeList::First (ALIterator& i) { i.SetValue(_alist->First()); }
void AttributeList::Last (ALIterator& i) { i.SetValue(_alist->Last()); }
void AttributeList::Next (ALIterator& i) { i.SetValue(Elem(i)->Next()); }
void AttributeList::Prev (ALIterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean AttributeList::Done (ALIterator i) { return Elem(i) == _alist->End(); }
int AttributeList::Number () { return _count; }

boolean AttributeList::Includes (Attribute* e) {
    return _alist->Find(e) != nil;
}

boolean AttributeList::IsEmpty () { return _alist->IsEmpty(); }

ostream& operator<< (ostream& out, const AttributeList& al) {

    AttributeList* attrlist = (AttributeList*)&al;
    ALIterator i;
    for (attrlist->First(i); !attrlist->Done(i); attrlist->Next(i)) {
	Attribute* attr = attrlist->GetAttr(i);
	out << " :" << attr->Name() << " ";

	AttributeValue* attrval = attr->Value();
#if 1
	out << *attrval;
#else
	char* string;
        switch(attr->Value()->type()) {
	    case AttributeValue::SymbolType:
	        out << attrval->symbol_ptr();
	        break;
	    case AttributeValue::StringType:
	      string = (char *) attrval->string_ptr();
	        out << "\"" << string << "\"";
	        break;
	    case AttributeValue::CharType:
	        out << "'" << attrval->char_ref() << "'";
	        break;
	    case AttributeValue::UCharType:
	        out << "'" << attrval->char_ref() << "'";
	        break;
	    case AttributeValue::IntType:
	        out << attrval->int_ref();
	        break;
	    case AttributeValue::UIntType:
	        out << attrval->uint_ref();
	        break;
	    case AttributeValue::ShortType:
	        out << attrval->short_ref();
	        break;
	    case AttributeValue::UShortType:
	        out << attrval->ushort_ref();
	        break;
	    case AttributeValue::LongType:
	        out << attrval->long_ref();
	        break;
	    case AttributeValue::ULongType:
	        out << attrval->ulong_ref();
	        break;
	    case AttributeValue::FloatType:
	        out.form("%.6", attrval->float_val());
	        break;
	    case AttributeValue::DoubleType:
	        out << attrval->double_ref();
	        break;
            default:
		out << "Unknown type";
	        break;
	}
#endif
    }
    return out;
}

void AttributeList::dump() {
  cerr << *this << "\n";
}

AttributeValue* AttributeList::find(const char* name) {
    int id = symbol_find((char *)name);
    return find(id);
}

AttributeValue* AttributeList::find(int symid) {
    if (symid==-1)
        return nil;
    ALIterator i;
    for (First(i); !Done(i); Next(i)) {
	Attribute* attr = GetAttr(i);
	if (attr->SymbolId() == symid) {
	    return attr->Value();
	}
    }
    return nil;
}

AttributeList* AttributeList::merge(AttributeList* al) {
  if (al) {
    ALIterator it;
    for( al->First(it); !al->Done(it); al->Next(it)) 
      add_attribute(new Attribute(*al->GetAttr(it)));
  }
  return this;
}

void AttributeList::clear() {
  ALIterator it;
  for( First(it); !Done(it); ) {
    Attribute* attr = GetAttr(it);
    Remove(it);
    delete attr;
  }
}

/*****************************************************************************/

AttributeValueList::AttributeValueList (AttributeValueList* s) {
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValueList");
    _leakchecker->create();
#endif
    _alist = new AList;
    _count = 0;
    if (s != nil) {
        ALIterator i;

        for (s->First(i); !s->Done(i); s->Next(i)) {
	    Append(new AttributeValue(s->GetAttrVal(i)));
	}
    }
    _nested_insert = false;
}

AttributeValueList::~AttributeValueList () { 
#ifdef LEAKCHECK
    _leakchecker->destroy();
#endif
    if (_alist) {
        ALIterator i;
	for (First(i); !Done(i); Next(i)) {
	    delete GetAttrVal(i);
	}
	delete _alist; 
    }
}

AttributeValue* AttributeValueList::AttrVal (AList* r) {
    return (AttributeValue*) (*r)();
}

AList* AttributeValueList::Elem (ALIterator i) { return (AList*) i.GetValue(); }

void AttributeValueList::Append (AttributeValue* v) {
    _alist->Append(new AList(v));
    ++_count;
}

void AttributeValueList::Prepend (AttributeValue* v) {
    _alist->Prepend(new AList(v));
    ++_count;
}

void AttributeValueList::InsertAfter (ALIterator i, AttributeValue* v) {
    Elem(i)->Prepend(new AList(v));
    ++_count;
}

void AttributeValueList::InsertBefore (ALIterator i, AttributeValue* v) {
    Elem(i)->Append(new AList(v));
    ++_count;
}

void AttributeValueList::Remove (ALIterator& i) {
    AList* doomed = Elem(i);

    Next(i);
    _alist->Remove(doomed);
    delete doomed;
    --_count;
}	
    
void AttributeValueList::Remove (AttributeValue* p) {
    AList* temp;

    if ((temp = _alist->Find(p)) != nil) {
	_alist->Remove(temp);
        delete temp;
	--_count;
    }
}

AttributeValue* AttributeValueList::GetAttrVal (ALIterator i) { return AttrVal
(Elem(i)); }

void AttributeValueList::SetAttrVal (AttributeValue* gv, ALIterator& i) {
    i.SetValue(_alist->Find(gv));
}

void AttributeValueList::First (ALIterator& i) { i.SetValue(_alist->First()); }
void AttributeValueList::Last (ALIterator& i) { i.SetValue(_alist->Last()); }
void AttributeValueList::Next (ALIterator& i) { i.SetValue(Elem(i)->Next()); }
void AttributeValueList::Prev (ALIterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean AttributeValueList::Done (ALIterator i) { return Elem(i) == _alist->End(); }
int AttributeValueList::Number () { return _count; }

boolean AttributeValueList::Includes (AttributeValue* e) {
    return _alist->Find(e) != nil;
}

boolean AttributeValueList::IsEmpty () { return _alist->IsEmpty(); }

ostream& operator<< (ostream& out, const AttributeValueList& al) {

    AttributeValueList* attrlist = (AttributeValueList*)&al;
    ALIterator i;
    for (attrlist->First(i); !attrlist->Done(i);) {
	AttributeValue* attrval = attrlist->GetAttrVal(i);

	char* string;
        switch(attrval->type()) {
	    case AttributeValue::SymbolType:
	        out << attrval->symbol_ptr();
	        break;
	    case AttributeValue::StringType:
	      string = (char *) attrval->string_ptr();
	        out << "\"" << string << "\"";
	        break;
	    case AttributeValue::CharType:
	        out << attrval->char_ref();
	        break;
	    case AttributeValue::UCharType:
	        out << attrval->char_ref();
	        break;
	    case AttributeValue::IntType:
	        out << attrval->int_ref();
	        break;
	    case AttributeValue::UIntType:
	        out << attrval->uint_ref();
	        break;
	    case AttributeValue::LongType:
	        out << attrval->long_ref();
	        break;
	    case AttributeValue::ULongType:
	        out << attrval->ulong_ref();
	        break;
	    case AttributeValue::FloatType:
	        out << attrval->float_ref();
	        break;
	    case AttributeValue::DoubleType:
	        out << attrval->double_ref();
	        break;
	    case AttributeValue::BooleanType:
	        out << attrval->boolean_ref();
	        break;
	    case AttributeValue::ArrayType:
  	        out << "{";
	        out << *attrval->array_ref();
  	        out << "}";
	        break;
            default:
		out << "nil";
	        break;
	}

	attrlist->Next(i);
	if (!attrlist->Done(i))  out << ",";
	
    }
    return out;
}


void AttributeValueList::clear() {
  ALIterator it;
  for( First(it); !Done(it); ) {
    AttributeValue* av = GetAttrVal(it);
    Remove(it);
    delete av;
  }
}

AttributeValue* AttributeValueList::Get(unsigned int index) {
  if (Number()<=index) return nil;
  Iterator it;
  First(it);
  for (int i=0; i<index; i++) Next(it);
  return GetAttrVal(it);
}

AttributeValue* AttributeValueList::Set(unsigned int index, AttributeValue* av) {
  if (Number()<=index) {
    Iterator it;
    Last(it);
    int padding = index-Number();
    for (int i=0; i<padding; i++) Append(new AttributeValue());
    Append(av);
    return nil;
  }
  else {
    Iterator it;
    First(it);
    for (int i=0; i<index; i++) Next(it);
    AttributeValue* oldv = Replace(it, av);
    return oldv;
  }
}

AttributeValue* AttributeValueList::Replace (ALIterator& i, AttributeValue* av) {
    AList* doomed = Elem(i);
    AttributeValue* removed = GetAttrVal(i);
    Next(i);
    _alist->Remove(doomed);
    delete doomed;
    Elem(i)->Append(new AList(av));
    return removed;
}	
    
