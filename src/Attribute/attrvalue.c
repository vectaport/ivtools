/*
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

#include <Attribute/aliterator.h>
#include <Attribute/attribute.h>
#include <Attribute/attrvalue.h>
#include <Attribute/attrlist.h>

#include <iostream.h>
#if !defined(solaris)
#include <memory.h>
#endif
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

AttributeValue::AttributeValue(ValueType valtype) {
    type(valtype);
}

AttributeValue::AttributeValue(ValueType valtype, attr_value value) {
    type(valtype);
    _v = value;
}

AttributeValue::AttributeValue(AttributeValue& sv) {
    *this = sv;
    if (_type == AttributeValue::ArrayType)
      Resource::ref(_v.arrayval.ptr);
}

AttributeValue::AttributeValue() {
    type(UnknownType);
    _aggregate_type = UnknownType;
}

AttributeValue::AttributeValue(unsigned char v) { 
    _type = AttributeValue::UCharType;
    _v.ucharval = v;
}

AttributeValue::AttributeValue(char v) { 
    _type = AttributeValue::CharType;
    _v.charval = v;
}

AttributeValue::AttributeValue(unsigned short v) { 
    _type = AttributeValue::UShortType;
    _v.ushortval = v;
}

AttributeValue::AttributeValue(short v) { 
    _type = AttributeValue::ShortType;
    _v.shortval = v;
}

AttributeValue::AttributeValue(unsigned int v, ValueType type) { 
    _type = type;
    _v.dfunsval = v;
}

AttributeValue::AttributeValue(unsigned int kv, unsigned int kn, ValueType type) { 
    _type = type;
    _v.keyval.keyid = kv;
    _v.keyval.keynarg = kn;
}

AttributeValue::AttributeValue(int v, ValueType type) { 
    _type = type;
    _v.dfintval = v;
}

AttributeValue::AttributeValue(unsigned long v) { 
    _type = AttributeValue::ULongType;
    _v.lnunsval = v;
}

AttributeValue::AttributeValue(long v) { 
    _type = AttributeValue::LongType;
    _v.lnintval = v;
}

AttributeValue::AttributeValue(float v) { 
    _type = AttributeValue::FloatType;
    _v.floatval = v;
}

AttributeValue::AttributeValue(double v) { 
    _type = AttributeValue::DoubleType;
    _v.doublval = v;
}

AttributeValue::AttributeValue(int classid, void* ptr) { 
    _type = AttributeValue::ObjectType;
    _v.objval.ptr = ptr;
    _v.objval.type = classid;
}

AttributeValue::AttributeValue(AttributeValueList* ptr) { 
    _type = AttributeValue::ArrayType;
    _v.arrayval.ptr = ptr;
    _v.arrayval.type = 0;
    Resource::ref(ptr);
}

AttributeValue::AttributeValue(const char* string) { 
    _type = AttributeValue::StringType;
    _v.dfintval = symbol_add((char*)string);
}

AttributeValue::~AttributeValue() {
#if 0  // disable symbol reference counting
    if (_type == StringType || _type == SymbolType) 
        symbol_del(string_val());
    else 
#endif
    if (_type == ArrayType && _v.arrayval.ptr)
        Unref(_v.arrayval.ptr);
    type(UnknownType);
}

AttributeValue& AttributeValue::operator= (const AttributeValue& sv) {
    void* v1 = &_v;
    const void* v2 = &sv._v;
    memcpy(v1, v2, sizeof(double));
    _type = sv._type;
    _aggregate_type = sv._aggregate_type;
#if 0  // disable symbol reference counting
    if (_type == StringType || _type == SymbolType) {
        char* sptr = (char *)string_ptr();
	if (sptr) symbol_add(sptr);
    }
    else 
#endif
    if (_type == ArrayType && _v.arrayval.ptr)
        Resource::ref(_v.arrayval.ptr);
    return *this;
}
    

AttributeValue::ValueType AttributeValue::type() const { return _type; }
void AttributeValue::type(AttributeValue::ValueType type) { _type = type; }
AttributeValue::ValueType AttributeValue::aggregate_type() const { return _aggregate_type; }

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
unsigned int& AttributeValue::string_ref() { return _v.symbolid; }
unsigned int& AttributeValue::symbol_ref() { return _v.symbolid; }
void*& AttributeValue::obj_ref() { return _v.objval.ptr; }
unsigned int& AttributeValue::obj_type_ref() { return _v.objval.type; }
AttributeValueList*& AttributeValue::array_ref() { return _v.arrayval.ptr; }
unsigned int& AttributeValue::array_type_ref() { return _v.arrayval.type; }
unsigned int& AttributeValue::keyid_ref() { return _v.keyval.keyid; }
unsigned int& AttributeValue::keynarg_ref() { return _v.keyval.keynarg; }

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
	return (boolean) int_val();
    case AttributeValue::ObjectType:
	return (boolean) obj_val();
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
	return obj_ref();
}

unsigned int AttributeValue::obj_type_val() { 
    return _v.objval.type; 
}

unsigned int& AttributeValue::class_symid() {
    return _v.objval.type; 
}

AttributeValueList* AttributeValue::array_val() { 
	return array_ref();
}

unsigned int AttributeValue::array_type_val() { 
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

unsigned int AttributeValue::command_symid() { return _command_symid; }
void AttributeValue::command_symid(unsigned int id, boolean alias) { 
  _command_symid = (alias ? -1 : 1) * id; }

ostream& operator<< (ostream& out, const AttributeValue& sv) {
    AttributeValue* svp = (AttributeValue*)&sv;
    char* title;
    char* symbol;
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
	    out << "array of length " << svp->array_len();
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
	  symbol = symbol_pntr( svp->symbol_ref() );
	  out << title << symbol;
	  counter = strlen(title) + strlen(symbol);
	  while( ++counter < 32 ) out << ' ';
	  out << ")";
	  break;
	  
	case AttributeValue::SymbolType:
	  out << svp->symbol_ptr();
	  break;

	case AttributeValue::StringType:
	  out << "\"" << svp->string_ptr() << "\"";
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
	case AttributeValue::BooleanType:
	  out << svp->uint_ref();
	  break;

	case AttributeValue::ShortType:
	  out << svp->short_ref();
	  break;

	case AttributeValue::UShortType:
	  out << svp->ushort_ref();
	  break;

	case AttributeValue::LongType:
	  out << svp->long_ref();
	  break;
	  
	case AttributeValue::ULongType:
	  out << svp->ulong_ref();
	  break;
	  
	case AttributeValue::FloatType:
	  out.form("%.6f", svp->float_val());
	  break;
	  
	case AttributeValue::DoubleType:
	  out.form("%.6f", svp->double_val());
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

	default:
	  out << "Unknown type";
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
    void* v1 = &_v;
    const void* v2 = &av._v;
    memcpy(v1, v2, sizeof(double));
    _type = av._type;
    _aggregate_type = av._aggregate_type;
#if 0 // this end of reference counting disabled as well
    if (_type == StringType || _type == SymbolType) 
	symbol_add((char *)string_ptr());
    else 
#endif
    if (_type == ArrayType && _v.arrayval.ptr)
        Resource::ref(_v.arrayval.ptr);
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

