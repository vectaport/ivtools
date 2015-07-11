/*
 * Copyright (c) 2001,2006 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1994-1997 Vectaport Inc.
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

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wswitch"
#endif

#include <Attribute/aliterator.h>
#include <Attribute/attribute.h>
#include <Attribute/attrvalue.h>
#include <Attribute/attrlist.h>

#ifdef RESOURCE_COMPVIEW
#include <Unidraw/Components/component.h>
#include <Unidraw/Components/compview.h>
#endif

#include <iostream.h>
#if !defined(solaris)
#include <memory.h>
#endif
#include <stdio.h>
#include <string.h>
#include <cstdio>
using namespace std;

#ifdef LEAKCHECK
LeakChecker* AttributeValue::_leakchecker = nil;
#endif

/*****************************************************************************/

int* AttributeValue::_type_syms = nil;

AttributeValue::AttributeValue(ValueType valtype) {
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    type(valtype);
}

AttributeValue::AttributeValue(ValueType valtype, attr_value value) {
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    type(valtype);
    _v = value;
    ref_as_needed();
}

AttributeValue::AttributeValue(AttributeValue& sv) {
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    type(UnknownType);
    *this = sv;
}

AttributeValue::AttributeValue(AttributeValue* sv) {
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    type(UnknownType);
    *this = *sv;
    dup_as_needed();
}

AttributeValue::AttributeValue() {
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    type(UnknownType);
    _command_symid = -1;
}

AttributeValue::AttributeValue(unsigned char v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::UCharType;
    _v.ucharval = v;
}

AttributeValue::AttributeValue(char v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::CharType;
    _v.charval = v;
}

AttributeValue::AttributeValue(unsigned short v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::UShortType;
    _v.ushortval = v;
}

AttributeValue::AttributeValue(short v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::ShortType;
    _v.shortval = v;
}

AttributeValue::AttributeValue(unsigned int v, ValueType type) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = type;
    if ( type >= CharType && type <= UShortType ) {
      switch (type) {
      case CharType: 
	_v.charval = v;
	break;
      case UCharType:
	_v.ucharval = v;
	break;
      case ShortType: 
	_v.shortval = v;
	break;
      case UShortType:
	_v.ushortval = v;
	break;
      }
    } else 
      _v.dfunsval = v;
}

AttributeValue::AttributeValue(unsigned int kv, unsigned int kn, ValueType type) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = type;
    _v.keyval.keyid = kv;
    _v.keyval.keynarg = kn;
}

AttributeValue::AttributeValue(int v, ValueType type) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = type;
    if ( type >= CharType && type <= UShortType ) {
      switch (type) {
      case CharType: 
	_v.charval = v;
	break;
      case UCharType:
	_v.ucharval = v;
	break;
      case ShortType: 
	_v.shortval = v;
	break;
      case UShortType:
	_v.ushortval = v;
	break;
      }
    } else 
      _v.dfintval = v;
    ref_as_needed(); // when used as a StringType constructor
}

AttributeValue::AttributeValue(unsigned long v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::ULongType;
    _v.lnunsval = v;
}

AttributeValue::AttributeValue(long v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::LongType;
    _v.lnintval = v;
}

AttributeValue::AttributeValue(float v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::FloatType;
    _v.floatval = v;
}

AttributeValue::AttributeValue(double v) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::DoubleType;
    _v.doublval = v;
}

AttributeValue::AttributeValue(int classid, void* ptr) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::ObjectType;
    _v.objval.ptr = ptr;
    _v.objval.type = classid;
    _object_compview = false;
    if (classid==Attribute::class_symid())
      Resource::ref((Attribute*)ptr);
    else if (classid==AttributeList::class_symid())
      Resource::ref((AttributeList*)ptr);
}

AttributeValue::AttributeValue(ComponentView* view, int compid) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::ObjectType;
    _v.objval.ptr = view;
    _v.objval.type = compid;
    _object_compview = true;
    Resource::ref(view);
}

AttributeValue::AttributeValue(AttributeValueList* ptr) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::ArrayType;
    _v.arrayval.ptr = ptr;
    _v.arrayval.type = 0;
    Resource::ref(ptr);
}

AttributeValue::AttributeValue(void* comfuncptr, AttributeValueList* vallist) {
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::StreamType;
    _v.streamval.funcptr = comfuncptr;
    _v.streamval.listptr = vallist;
    Resource::ref(vallist);
}

AttributeValue::AttributeValue(const char* string) { 
#ifdef LEAKCHECK
    if(!_leakchecker) _leakchecker = new LeakChecker("AttributeValue");
    _leakchecker->create();
#endif
    clear();
    _type = AttributeValue::StringType;
    _v.dfintval = symbol_add((char*)string);
}

AttributeValue::~AttributeValue() {
#ifdef LEAKCHECK
    _leakchecker->destroy();
#endif
    unref_as_needed();
    type(UnknownType);
}

void AttributeValue::clear() {
    unsigned char* buf = (unsigned char*)(void*)&_v;
    for (int i=0; i<sizeof(double); i++) buf[i] = '\0';
    _state = 0;
}

AttributeValue& AttributeValue::operator= (const AttributeValue& sv) {
    boolean preserve_flag = same_list(sv);
    if (!preserve_flag) unref_as_needed();
    void* v1 = &_v;
    const void* v2 = &sv._v;
    memcpy(v1, v2, sizeof(_v));
    _type = sv._type;
    _command_symid = sv._command_symid;
    if (!preserve_flag) ref_as_needed();
    return *this;
}
    

AttributeValue::ValueType AttributeValue::type() const { return _type; }
void AttributeValue::type(AttributeValue::ValueType type) { _type = type; }

unsigned char& AttributeValue::uchar_ref() { return _v.ucharval; }
char& AttributeValue::char_ref() { return _v.charval; }
unsigned short& AttributeValue::ushort_ref() { return _v.ushortval; }
short& AttributeValue::short_ref() { return _v.shortval; }
unsigned int& AttributeValue::uint_ref() { return _v.dfunsval; }
boolean& AttributeValue::boolean_ref() { return _v.dfunsval; }
int& AttributeValue::int_ref() { return _v.dfintval; }
unsigned long& AttributeValue::ulong_ref() { return _v.lnunsval; }
long& AttributeValue::long_ref() { return _v.lnintval; }
float& AttributeValue::float_ref() { return _v.floatval; }
double& AttributeValue::double_ref() { return _v.doublval; }
unsigned int& AttributeValue::string_ref() { return _v.symval.symid; }
unsigned int& AttributeValue::symbol_ref() { return _v.symval.symid; }
void*& AttributeValue::obj_ref() { return _v.objval.ptr; }
unsigned int& AttributeValue::obj_type_ref() { return _v.objval.type; }
AttributeValueList*& AttributeValue::array_ref() { return _v.arrayval.ptr; }
unsigned int& AttributeValue::array_type_ref() { return _v.arrayval.type; }
AttributeValueList*& AttributeValue::list_ref() { return _v.arrayval.ptr; }
unsigned int& AttributeValue::list_type_ref() { return _v.arrayval.type; }
unsigned int& AttributeValue::keyid_ref() { return _v.keyval.keyid; }
unsigned int& AttributeValue::keynarg_ref() { return _v.keyval.keynarg; }

boolean AttributeValue::global_flag() { return is_symbol() && _v.symval.globalflag; }
void AttributeValue::global_flag(boolean flag) 
{ 
  if (is_symbol()) _v.symval.globalflag = flag; 
}

boolean AttributeValue::boolean_val() {
    switch (type()) {
    case AttributeValue::CharType:
	return (boolean) char_val();
    case AttributeValue::UCharType:
	return (boolean) uchar_val();
    case AttributeValue::ShortType:
	return (boolean) short_val();
    case AttributeValue::UShortType:
	return (boolean) ushort_val();
    case AttributeValue::IntType:
	return (boolean) int_val();
    case AttributeValue::UIntType:
	return (boolean) uint_val();
    case AttributeValue::LongType:
	return (boolean) long_val();
    case AttributeValue::ULongType:
	return (boolean) ulong_val();
    case AttributeValue::FloatType:
	return (boolean) float_val();
    case AttributeValue::DoubleType:
	return (boolean) double_val();
    case AttributeValue::BooleanType:
	return boolean_ref();
    case AttributeValue::SymbolType:
    case AttributeValue::StringType:
	return (boolean) int_val()!=-1;
    case AttributeValue::ObjectType:
        return (boolean) (unsigned long) obj_val();
    case AttributeValue::StreamType:
	return stream_mode() != 0;
    case AttributeValue::ListType:
        return array_val() != 0;
    default:
	return 0;
    }
}

unsigned char AttributeValue::uchar_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (unsigned char) char_val();
    case AttributeValue::UCharType:
	return uchar_ref();
    case AttributeValue::ShortType:
	return (unsigned char) short_val();
    case AttributeValue::UShortType:
	return (unsigned char) ushort_val();
    case AttributeValue::IntType:
	return (unsigned char) int_val();
    case AttributeValue::UIntType:
	return (unsigned char) uint_val();
    case AttributeValue::LongType:
	return (unsigned char) long_val();
    case AttributeValue::ULongType:
	return (unsigned char) ulong_val();
    case AttributeValue::FloatType:
	return (unsigned char) (unsigned char) float_val();
    case AttributeValue::DoubleType:
	return (unsigned char) (unsigned char) double_val();
    case AttributeValue::BooleanType:
	return (unsigned char) boolean_val();
    case AttributeValue::SymbolType:
	return (unsigned char) int_val();
    default:
	return '\0';
    }
}

char AttributeValue::char_val() {
    switch (type()) {
    case AttributeValue::CharType:
	return char_ref();
    case AttributeValue::UCharType:
	return (char) uchar_val();
    case AttributeValue::ShortType:
	return (char) short_val();
    case AttributeValue::UShortType:
	return (char) ushort_val();
    case AttributeValue::IntType:
	return (char) int_val();
    case AttributeValue::UIntType:
	return (char) uint_val();
    case AttributeValue::LongType:
	return (char) long_val();
    case AttributeValue::ULongType:
	return (char) ulong_val();
    case AttributeValue::FloatType:
	return (char) float_val();
    case AttributeValue::DoubleType:
	return (char) double_val();
    case AttributeValue::BooleanType:
	return (char) boolean_val();
    case AttributeValue::SymbolType:
	return (char) int_val();
    default:
	return '\0';
    }
}

unsigned short AttributeValue::ushort_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (unsigned short) char_val();
    case AttributeValue::UCharType:
	return (unsigned short) uchar_val();
    case AttributeValue::ShortType:
	return (unsigned short) short_val();
    case AttributeValue::UShortType:
	return ushort_ref();
    case AttributeValue::IntType:
	return (unsigned short) int_val();
    case AttributeValue::UIntType:
	return (unsigned short) uint_val();
    case AttributeValue::LongType:
	return (unsigned short) long_val();
    case AttributeValue::ULongType:
	return (unsigned short) ulong_val();
    case AttributeValue::FloatType:
	return (unsigned short) float_val();
    case AttributeValue::DoubleType:
	return (unsigned short) double_val();
    case AttributeValue::BooleanType:
	return (unsigned short) boolean_val();
    case AttributeValue::SymbolType:
	return (unsigned short) int_val();
    default:
	return 0;
    }
}

short AttributeValue::short_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (short) char_val();
    case AttributeValue::UCharType:
	return (short) uchar_val();
    case AttributeValue::ShortType:
	return short_ref();
    case AttributeValue::UShortType:
	return (short) ushort_val();
    case AttributeValue::IntType:
	return (short) int_val();
    case AttributeValue::UIntType:
	return (short) uint_val();
    case AttributeValue::LongType:
	return (short) long_val();
    case AttributeValue::ULongType:
	return (short) ulong_val();
    case AttributeValue::FloatType:
	return (short) float_val();
    case AttributeValue::DoubleType:
	return (short) double_val();
    case AttributeValue::BooleanType:
	return (short) boolean_val();
    case AttributeValue::SymbolType:
	return (short) int_val();
    default:
	return 0;
    }
}

unsigned int AttributeValue::uint_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (unsigned int) char_val();
    case AttributeValue::UCharType:
	return (unsigned int) uchar_val();
    case AttributeValue::ShortType:
	return (unsigned int) short_val();
    case AttributeValue::UShortType:
	return (unsigned int) ushort_val();
    case AttributeValue::IntType:
	return (unsigned int) int_val();
    case AttributeValue::UIntType:
	return uint_ref();
    case AttributeValue::LongType:
	return (unsigned int) long_val();
    case AttributeValue::ULongType:
	return (unsigned int) ulong_val();
    case AttributeValue::FloatType:
	return (unsigned int) float_val();
    case AttributeValue::DoubleType:
	return (unsigned int) double_val();
    case AttributeValue::BooleanType:
	return (unsigned int) boolean_val();
    case AttributeValue::SymbolType:
	return (unsigned int) int_val();
    case AttributeValue::ObjectType:
        return (unsigned int) (unsigned long) obj_val();
    default:
	return 0;
    }
}

int AttributeValue::int_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (int) char_val();
    case AttributeValue::UCharType:
	return (int) uchar_val();
    case AttributeValue::ShortType:
	return (int) short_val();
    case AttributeValue::UShortType:
	return (int) ushort_val();
    case AttributeValue::IntType:
	return int_ref();
    case AttributeValue::UIntType:
	return (int) uint_val();
    case AttributeValue::LongType:
	return (int) long_val();
    case AttributeValue::ULongType:
	return (int) ulong_val();
    case AttributeValue::FloatType:
	return (int) float_val();
    case AttributeValue::DoubleType:
	return (int) double_val();
    case AttributeValue::BooleanType:
	return (int) boolean_val();
    case AttributeValue::SymbolType:
	return int_ref();
    case AttributeValue::ObjectType:
        return (int) (long) obj_val();
    default:
	return 0;
    }
}

unsigned long AttributeValue::ulong_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (unsigned long) char_val();
    case AttributeValue::UCharType:
	return (unsigned long) uchar_val();
    case AttributeValue::ShortType:
	return (unsigned long) short_val();
    case AttributeValue::UShortType:
	return (unsigned long) ushort_val();
    case AttributeValue::IntType:
	return (unsigned long) int_val();
    case AttributeValue::UIntType:
	return (unsigned long) uint_val();
    case AttributeValue::LongType:
	return (unsigned long) long_val();
    case AttributeValue::ULongType:
	return ulong_ref();
    case AttributeValue::FloatType:
	return (unsigned long) float_val();
    case AttributeValue::DoubleType:
	return (unsigned long) double_val();
    case AttributeValue::BooleanType:
	return (unsigned long) boolean_val();
    case AttributeValue::SymbolType:
	return (unsigned long) int_val();
    case AttributeValue::ObjectType:
        return (unsigned long)obj_val();
    default:
	return 0L;
    }
}

long AttributeValue::long_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (long) char_val();
    case AttributeValue::UCharType:
	return (long) uchar_val();
    case AttributeValue::ShortType:
	return (long) short_val();
    case AttributeValue::UShortType:
	return (long) ushort_val();
    case AttributeValue::IntType:
	return (long) int_val();
    case AttributeValue::UIntType:
	return (long) uint_val();
    case AttributeValue::LongType:
	return long_ref();
    case AttributeValue::ULongType:
	return (long) ulong_val();
    case AttributeValue::FloatType:
	return (long) float_val();
    case AttributeValue::DoubleType:
	return (long) double_val();
    case AttributeValue::BooleanType:
	return (long) boolean_val();
    case AttributeValue::SymbolType:
	return (long) int_val();
    case AttributeValue::ObjectType:
        return (long)obj_val();
    default:
	return 0L;
    }
}

float AttributeValue::float_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (float) char_val();
    case AttributeValue::UCharType:
	return (float) uchar_val();
    case AttributeValue::ShortType:
	return (float) short_val();
    case AttributeValue::UShortType:
	return (float) ushort_val();
    case AttributeValue::IntType:
	return (float) int_val();
    case AttributeValue::UIntType:
	return (float) uint_val();
    case AttributeValue::LongType:
	return (float) long_val();
    case AttributeValue::ULongType:
	return (float) ulong_val();
    case AttributeValue::FloatType:
	return float_ref();
    case AttributeValue::DoubleType: 
	return (float) double_val();
    case AttributeValue::BooleanType:
	return (float) boolean_val();
    case AttributeValue::SymbolType:
	return (float) int_val();
    default:
	return 0.0;
    }
}

double AttributeValue::double_val() { 
    switch (type()) {
    case AttributeValue::CharType:
	return (double) char_val();
    case AttributeValue::UCharType:
	return (double) uchar_val();
    case AttributeValue::ShortType:
	return (double) short_val();
    case AttributeValue::UShortType:
	return (double) ushort_val();
    case AttributeValue::IntType:
	return (double) int_val();
    case AttributeValue::UIntType:
	return (double) uint_val();
    case AttributeValue::LongType:
	return (double) long_val();
    case AttributeValue::ULongType:
	return (double) ulong_val();
    case AttributeValue::FloatType:
	return (double) float_val();
    case AttributeValue::DoubleType: 
	return (double) double_ref();
    case AttributeValue::BooleanType:
	return (double) boolean_val();
    case AttributeValue::SymbolType:
	return (double) int_val();
    default:
	return 0.0;
    }
}

unsigned int AttributeValue::string_val() { 
    return string_ref();
}

unsigned int AttributeValue::symbol_val() { 
    return symbol_ref();
}

void* AttributeValue::obj_val() { 
	return is_object()||is_command() ? obj_ref() : nil;
}

unsigned int AttributeValue::obj_type_val() { 
    return _v.objval.type; 
}

unsigned int& AttributeValue::class_symid() {
    return _v.objval.type;
}

const char* AttributeValue::class_name() {
    return symbol_pntr(_v.objval.type);
}

AttributeValueList* AttributeValue::array_val() { 
	return array_ref();
}

unsigned int AttributeValue::array_type_val() { 
    return _v.arrayval.type; 
}

AttributeValueList* AttributeValue::list_val() { 
	return list_ref();
}

unsigned int AttributeValue::list_type_val() { 
    return _v.arrayval.type; 
}

unsigned int AttributeValue::keyid_val() { 
    return _v.keyval.keyid; 
}

unsigned int AttributeValue::keynarg_val() { 
    return _v.keyval.keynarg; 
}

const char* AttributeValue::string_ptr() {
    return symbol_pntr(string_val());
}

const char* AttributeValue::symbol_ptr() {
    return symbol_pntr(symbol_val());
}

int AttributeValue::array_len() {
    if (is_type(AttributeValue::ArrayType))
        return array_val()->Number();
    else
        return 0;
}

int AttributeValue::list_len() {
    if (is_type(AttributeValue::ArrayType))
        return array_val()->Number();
    else
        return 0;
}

int AttributeValue::command_symid() { return _command_symid; }
void AttributeValue::command_symid(int id, boolean alias) { 
  _command_symid = (alias ? -1 : 1) * id; }

const char* AttributeValue::command_name() {
    return symbol_pntr(_command_symid);
}

ostream& operator<< (ostream& out, const AttributeValue& sv) {
    AttributeValue* svp = (AttributeValue*)&sv;
    const char* title;
    const char* symbol;
    int counter;
#if 0
    switch( svp->type() )
	{
	case AttributeValue::KeywordType:
	    out << "Keyword (" << symbol_pntr( svp->symbol_ref() ) << 
		")"; 
	    break;
	    
	case AttributeValue::CommandType:
	    title = "Command (";
	    symbol = symbol_pntr( svp->symbol_ref() );
	    out << title << symbol;
	    counter = strlen(title) + strlen(symbol);
	    while( ++counter < 32 ) out << ' ';
	    out << ")";
	    break;
	    
	case AttributeValue::SymbolType:
	    out << "symbol (" << svp->symbol_ptr()  << ")";
	    break;
	    
	case AttributeValue::StringType:
	    out << "string (" << svp->string_ptr()  << ")";
	    break;
	    
	case AttributeValue::BooleanType:
	    out << "boolean (" << svp->boolean_ref() << ")";
	    break;
	    
	case AttributeValue::CharType:
	    out << "char (" << svp->char_ref() << ":" << (int)svp->char_ref() << ")";
	    break;
	    
	case AttributeValue::UCharType:
	    out << "uchar (" << svp->char_ref() << ":" << (int)svp->char_ref() << ")";
	    break;
	    
	case AttributeValue::IntType:
	    out << "int (" << svp->int_ref() << ")";
	    break;
	    
	case AttributeValue::UIntType:
	    if (svp->state()==AttributeValue::OctState) 
	      out << "uint (" << svp->uint_ref() << ")";
	    else if (svp->state()==AttributeValue::HexState) 
	      out << "uint (" << svp->uint_ref() << ")";
	    else
	      out << "uint (" << svp->uint_ref() << ")";
	    break;
	    
	case AttributeValue::LongType:
	    out << "Long (" << svp->long_ref() << ")";
	    break;
	    
	case AttributeValue::ULongType:
	    out << "ulong (" << svp->ulong_ref() << ")";
	    break;
	    
	case AttributeValue::FloatType:
	    out << "float (" << svp->float_ref() << ")";
	    break;
	    
	case AttributeValue::DoubleType:
	    out << "double (" << svp->double_ref() << ")";
	    //printf("%9.2f\n", svp->double_ref());
	    break;
	    
	case AttributeValue::EofType:
	    out << "eof";
	    break;
	    
	case AttributeValue::ArrayType:
	  {
	    out << "list of length " << svp->array_len();
	    ALIterator i;
	    AttributeValueList* avl = svp->array_val();
	    avl->First(i);
	    boolean first = true;
	    while (!avl->Done(i)) {
 	        out << "\n\t" << *avl->GetAttrVal(i);
	        avl->Next(i);
	    }
	  }
	    break;
	    
	case AttributeValue::BlankType:
	    break;
	    
	default:
	    break;
	}
#else
        switch(svp->type()) {
	case AttributeValue::KeywordType:
	  out << "Keyword (" << symbol_pntr( svp->symbol_ref() ) << 
	    ")"; 
	  break;
	  
	case AttributeValue::CommandType:
	  title = "Command (";
	  symbol = symbol_pntr( svp->command_symid() );
	  out << title << symbol;
	  counter = strlen(title) + strlen(symbol);
	  while( ++counter < 32 ) out << ' ';
	  out << ")";
	  break;
	  
	case AttributeValue::SymbolType:
	  out << svp->symbol_ptr();
	  break;

	case AttributeValue::StringType:
	  {
	    out << "\"";
	    const char *ptr = svp->string_ptr();
	    while (*ptr) {
	      switch (*ptr) {
	      case '\t' : out << "\\t"; break;
	      case '\n' : out << "\\n"; break;
	      default : out << *ptr;
	      };
	      ptr++;
	    }
	    out << "\"";
	  }
	  break;

	case AttributeValue::CharType:
	  out << svp->char_ref();
	  break;

	case AttributeValue::UCharType:
	  out << svp->char_ref();
	  break;
	  
	case AttributeValue::IntType:
	  out << svp->int_ref();
	  break;
	  
	case AttributeValue::UIntType:
	  if (svp->state()==AttributeValue::OctState)
	    out << "0" << std::oct << svp->uint_ref() << std::dec;
	  else if (svp->state()==AttributeValue::HexState)
	    out << "0x" << std::hex << svp->uint_ref() << std::dec;
	  else
	    out << svp->uint_ref();
	  break;

	case AttributeValue::BooleanType:
	  out << svp->uint_ref();
	  break;

	case AttributeValue::ShortType:
	  out << svp->short_ref();
	  break;

	case AttributeValue::UShortType:
	  if (svp->state()==AttributeValue::OctState)
	    out << "0" << std::oct << svp->ushort_ref() << std::dec;
	  else if (svp->state()==AttributeValue::HexState)
	    out << "0x" << std::hex << svp->ushort_ref() << std::dec;
	  else
	    out << svp->ushort_ref();
	  break;

	case AttributeValue::LongType:
	  out << svp->long_ref();
	  break;
	  
	case AttributeValue::ULongType:
	  if (svp->state()==AttributeValue::OctState)
	    out << "0" << std::oct << svp->ulong_ref() << std::dec;
	  else if (svp->state()==AttributeValue::HexState)
	    out << "0x" << std::hex << svp->ulong_ref() << std::dec;
	  else
	    out << svp->ulong_ref();
	  break;
	  
	case AttributeValue::FloatType:
#if __GNUG__<3
	  out.form("%.6f", svp->float_val());
#else
          {
	  const int bufsiz=256;
	  char buffer[bufsiz];
	  snprintf(buffer, bufsiz, "%.6f", svp->float_val());
	  out << buffer;
	  }
#endif
	  break;
	  
	case AttributeValue::DoubleType:
#if __GNUG__<3
	  out.form("%.6f", svp->double_val());
#else
	  {
	  const int bufsiz=256;
	  char buffer[bufsiz];
	  snprintf(buffer, bufsiz, "%.6f", svp->double_val());
	  out << buffer;
	  }
#endif
	  break;

	case AttributeValue::EofType:
	    out << "eof";
	    break;
	    
	case AttributeValue::ArrayType:
	  {
	    //out << "array of length " << svp->array_len();
	    ALIterator i;
	    AttributeValueList* avl = svp->array_val();
	    avl->First(i);
	    boolean first = true;
	    while (!avl->Done(i)) {
	      if (!first)
		out << ",";
	      out << *avl->GetAttrVal(i);
	      avl->Next(i);
	      first = false;
	    }
	  }
	    break;
	    
	case AttributeValue::BlankType:
	    break;
	    
	case AttributeValue::ObjectType:
	  out << "<" << symbol_pntr(svp->class_symid()) << ">";
	  break;

	case AttributeValue::StreamType:
	  out << "<stream:" << svp->stream_mode() << ">";
	  break;
	    
	default:
	  out << "nil";
	  break;
	}
#endif
    return out;
}

void AttributeValue::negate() { 
    switch (type()) {
    case AttributeValue::CharType:
        char_ref() = -char_val();
	return;
    case AttributeValue::UCharType:
        char_ref() = -uchar_val();
	type(CharType);
	return;
    case AttributeValue::ShortType:
	short_ref() = -short_val();
	return;
    case AttributeValue::UShortType:
        short_ref() = -ushort_val();
	type(ShortType);
	return;
    case AttributeValue::IntType:
        int_ref() = -int_val();
	return ;
    case AttributeValue::UIntType:
        int_ref() = -uint_val();
	type(IntType);
	return;
    case AttributeValue::LongType:
        long_ref() = -long_val();
	return;
    case AttributeValue::ULongType:
        long_ref() = -ulong_ref();
	type(LongType);
	return;
    case AttributeValue::FloatType:
        float_ref() = -float_val();
	return;
    case AttributeValue::DoubleType:
        double_ref() = -double_val();
	return;
    default:
	return;
    }
}

int AttributeValue::type_size(ValueType type) {
  switch (type) {
  case AttributeValue::UnknownType:
    return 0; 
  case AttributeValue::CharType:
    return sizeof(char);
  case AttributeValue::UCharType:
    return sizeof(unsigned char);
  case AttributeValue::ShortType:
    return sizeof(short);
  case AttributeValue::UShortType:
    return sizeof(unsigned short);
  case AttributeValue::IntType:
    return sizeof(int);
  case AttributeValue::UIntType:
    return sizeof(unsigned int);
  case AttributeValue::LongType:
    return sizeof(long);
  case AttributeValue::ULongType:
    return sizeof(unsigned long);
  case AttributeValue::FloatType:
    return sizeof(float);
  case AttributeValue::DoubleType:
    return sizeof(double);
  default:
    return 0;
  }
}

void AttributeValue::assignval (const AttributeValue& av) {
    boolean preserve_flag = same_list(av);
    if (!preserve_flag) unref_as_needed();
    void* v1 = &_v;
    const void* v2 = &av._v;
    memcpy(v1, v2, sizeof(_v));
    _type = av._type;
    _command_symid = av._command_symid;
    if (!preserve_flag) ref_as_needed();
}
    

boolean AttributeValue::is_attributelist() {
  return is_object() && class_symid() == AttributeList::class_symid();
}

boolean AttributeValue::is_attribute() {
  return is_object() && class_symid() == Attribute::class_symid();
}

void* AttributeValue::geta(int id) {
  if (is_object(id))
    return obj_val();
  else
    return nil;
}

int AttributeValue::type_symid() const {
  if (!_type_syms) {
    int i = 0;
    _type_syms = new int[((int)BlankType)+1];
    _type_syms[i++] = symbol_add("UnknownType");
    _type_syms[i++] = symbol_add("CharType");
    _type_syms[i++] = symbol_add("UCharType");
    _type_syms[i++] = symbol_add("ShortType");
    _type_syms[i++] = symbol_add("UShortType");
    _type_syms[i++] = symbol_add("IntType");
    _type_syms[i++] = symbol_add("UIntType");
    _type_syms[i++] = symbol_add("LongType");
    _type_syms[i++] = symbol_add("ULongType");
    _type_syms[i++] = symbol_add("FloatType");
    _type_syms[i++] = symbol_add("DoubleType");
    _type_syms[i++] = symbol_add("StringType");
    _type_syms[i++] = symbol_add("SymbolType");
    _type_syms[i++] = symbol_add("ListType");
    _type_syms[i++] = symbol_add("StreamType");
    _type_syms[i++] = symbol_add("CommandType");
    _type_syms[i++] = symbol_add("KeywordType");
    _type_syms[i++] = symbol_add("ObjectType");
    _type_syms[i++] = symbol_add("EofType");
    _type_syms[i++] = symbol_add("BooleanType");
    _type_syms[i++] = symbol_add("OperatorType");
    _type_syms[i++] = symbol_add("BlankType");
  }
  if (type()>=UnknownType && type()<=BlankType)
    return _type_syms[(int)type()];
  else
    return -1;
}


void AttributeValue::ref_as_needed() {
    if (_type == AttributeValue::ArrayType)
      Resource::ref(_v.arrayval.ptr);
    else if (_type == AttributeValue::StreamType)
      Resource::ref(_v.streamval.listptr);
    else if (_type == AttributeValue::StringType)
      symbol_reference(string_val());
    else if (_type == AttributeValue::SymbolType) // never to be unreferenced
      symbol_reference(symbol_val());
#ifdef RESOURCE_COMPVIEW
    else if (_type == AttributeValue::ObjectType) {
      if (object_compview())
	Resource::ref((ComponentView*)_v.objval.ptr);
      else if (obj_type_val()==AttributeList::class_symid()) 
	Resource::ref((AttributeList*)_v.objval.ptr);
      else if (obj_type_val()==Attribute::class_symid()) 
	Resource::ref((Attribute*)_v.objval.ptr);
    }
#endif
}

void AttributeValue::dup_as_needed() {
  if (_type == AttributeValue::ArrayType) {
    AttributeValueList* avl = _v.arrayval.ptr;
    _v.arrayval.ptr = new AttributeValueList(avl);
    Resource::ref(_v.arrayval.ptr);
    Resource::unref(avl);
  } else if (_type == AttributeValue::StreamType) {
    AttributeValueList* avl = _v.streamval.listptr;
    _v.streamval.listptr = new AttributeValueList(avl);
    Resource::ref(_v.streamval.listptr);
    Resource::unref(avl);
  } 
#ifdef RESOURCE_COMPVIEW
  else if (_type == AttributeValue::ObjectType) {
    if (object_compview()) {
      ComponentView* oldview = (ComponentView*)_v.objval.ptr;
      Component* subject = oldview->GetSubject();
      ComponentView* newview = oldview->Duplicate();
      newview->SetSubject(subject);
      subject->Attach(newview);
      _v.objval.ptr = newview;
      Resource::ref(newview);
      Resource::unref(oldview);
    }
    else if (obj_type_val()==AttributeList::class_symid()) {
      AttributeList* al = (AttributeList*)_v.objval.ptr;
      _v.objval.ptr = new AttributeList(al);
      Resource::ref((AttributeList*)_v.objval.ptr);
      Resource::unref((AttributeList*)al);
    }
    else if (obj_type_val()==Attribute::class_symid()) {
      Attribute* al = (Attribute*)_v.objval.ptr;
      _v.objval.ptr = (void*) new Attribute(*al);
      Resource::ref((Attribute*)_v.objval.ptr);
      Resource::unref((Attribute*)al);
    }
  }
#endif
}

void AttributeValue::unref_as_needed() {
  if (_type == AttributeValue::ArrayType) {
      Resource::unref(_v.arrayval.ptr);
  }
  else if (_type == AttributeValue::StreamType)
      Resource::unref(_v.streamval.listptr);
  else if (_type == AttributeValue::StringType)  // only StringType, never for SymbolType
       symbol_del(string_val());
#ifdef RESOURCE_COMPVIEW
  else if (_type == AttributeValue::ObjectType) {
    if (object_compview()) 
       Resource::unref((ComponentView*)_v.objval.ptr);
    else if (obj_type_val() == AttributeList::class_symid()) 
       Resource::unref((AttributeList*)_v.objval.ptr);
    else if (obj_type_val() == Attribute::class_symid()) 
       Resource::unref((Attribute*)_v.objval.ptr);
  }
#endif
}

const boolean AttributeValue::same_list(const AttributeValue& av) {
  if (_type == AttributeValue::ArrayType)
    return _v.arrayval.ptr == av._v.arrayval.ptr;
  else if (_type == AttributeValue::StreamType)
    return _v.streamval.listptr == av._v.streamval.listptr;
#ifdef RESOURCE_COMPVIEW
  else if (_type == AttributeValue::ObjectType)
    return _v.objval.ptr == av._v.objval.ptr && _object_compview == av._object_compview;
#endif
  else if (_type == AttributeValue::StringType)
    return _v.symval.symid == av._v.symval.symid;
  else
    return false;
}

void AttributeValue::stream_list(AttributeValueList* list) 
{ 
  if (is_stream()) {
    Resource::unref(_v.streamval.listptr); 
    _v.streamval.listptr = list; 
    if (!list) 
      stream_mode(0);
    else
      Resource::ref(list); 
  }
}

int AttributeValue::stream_mode() { 
  if (is_stream()) {
    if (!stream_list() || stream_list()->Number()==0)
      return 0;
    else
      return _stream_mode;
  } else
    return 0;
}

int AttributeValue::state() {
  if (!is_stream() && !is_object() && !is_command()) 
    return _state;
  else
    return -1;
}

void AttributeValue::state(int val) {
  if (!is_stream() && !is_object() && !is_command()) 
    _state = val;
}

boolean AttributeValue::is_object(int class_symid) { 
  if (!is_type(ObjectType)) return false;
  if (this->class_symid() == class_symid) return true;
  return false;
}

#if 0
void AttributeValue::object_compview(boolean flag) { 
  _object_compview = flag; 
  if(flag) Resource::ref((ComponentView*)_v.objval.ptr);
}
#endif


AttributeValue::AttributeValue(postfix_token* token) {
    clear();
    void* v1 = &_v;
    void* v2 = &token->v;
    memcpy(v1, v2, sizeof(_v));
    switch (token->type) {
    case TOK_STRING:  type(StringType); symbol_reference(string_val()); break;
    case TOK_CHAR:    type(CharType); break;
    case TOK_DFINT:   type(IntType); break;
    case TOK_DFUNS:   type(UIntType); break;
    case TOK_LNINT:   type(LongType); break;
    case TOK_LNUNS:   type(ULongType); break;
    case TOK_FLOAT:   type(FloatType); break;
    case TOK_DOUBLE:  type(DoubleType); break;
    case TOK_EOF:     type(EofType); break;
    case TOK_COMMAND: type(SymbolType); symbol_reference(symbol_val()); _v.symval.globalflag=0; break;
    case TOK_KEYWORD: type(KeywordType); break;
    case TOK_BLANK:   type(BlankType); break;
    default:          type(UnknownType); break;
    }
}

boolean AttributeValue::equal(AttributeValue& av) {

  boolean result;
  if (av.type()==AttributeValue::UnknownType && type()!=AttributeValue::UnknownType)
    result = false;
  
  else if (av.type()==AttributeValue::BlankType && type()!=AttributeValue::BlankType)
    result = false;
  
  else {
    switch (type()) {
    case AttributeValue::CharType:
      result = char_val() == av.char_val();
      break;
    case AttributeValue::UCharType:
      result = uchar_val() == av.uchar_val();
      break;
    case AttributeValue::ShortType:
      result = short_val() == av.short_val();
      break;
    case AttributeValue::UShortType:
      result = ushort_val() == av.ushort_val();
      break;
    case AttributeValue::IntType:
      result = int_val() == av.int_val();
      break;
    case AttributeValue::UIntType:
      result = uint_val() == av.uint_val();
      break; 
    case AttributeValue::LongType:
      result = long_val() == av.long_val();
      break;
    case AttributeValue::ULongType:
      result = ulong_val() == av.ulong_val();
      break;
    case AttributeValue::FloatType:
      result = float_val() == av.float_val();
      break;
    case AttributeValue::DoubleType:
      result = double_val() == av.double_val();
      break;
    case AttributeValue::StringType:
    case AttributeValue::SymbolType:
      result = strcmp(symbol_ptr(), av.symbol_ptr())==0;
      break;
    case AttributeValue::ArrayType: 
      result = av.type() == AttributeValue::ArrayType && 
        (array_val() == av.array_val() ||
         array_val()->Equal(av.array_val()));
      break;
    case AttributeValue::ObjectType:
      if (!object_compview())
        result = av.type() == AttributeValue::ObjectType && 
          obj_val() == av.obj_val() &&
          class_symid() == av.class_symid();
      else
        result = av.type() == AttributeValue::ObjectType && 
          class_symid() == av.class_symid() &&
          av.object_compview() &&
          ((ComponentView*)obj_val())->GetSubject() == 
          ((ComponentView*)av.obj_val())->GetSubject();
      break;
    default:
      result = 
        is_type(AttributeValue::UnknownType) && av.is_type(AttributeValue::UnknownType) ||
        is_type(AttributeValue::BlankType) && av.is_type(AttributeValue::BlankType);
      break;
    }
  }
  return result;
}
