/*
 * Copyright (c) 1994-1998 Vectaport Inc.
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

#if !defined(_attrvalue_h)
#define _attrvalue_h

#include <stdlib.h>
#include <OS/enter-scope.h>

extern "C" {
    int symbol_add(char*);
    int symbol_del(int);
    int symbol_find(char*);
    char* symbol_pntr(int);
}

class AttributeValueList;
class ostream;

/* Union for data storage */
typedef union attr_value_union
{
      char              charval;
      unsigned char     ucharval;
      short             shortval;
      unsigned short    ushortval;
      int               dfintval;
      unsigned int      dfunsval;
      long              lnintval;
      unsigned long     lnunsval;
      float             floatval;
      double            doublval;
      unsigned int      symbolid;
      struct {
	void *ptr;
	unsigned int type;
        }               objval;
      struct {
	AttributeValueList *ptr;
	unsigned int type;
        }               arrayval;
  struct {
    unsigned int keyid;
    unsigned int keynarg;
  } keyval;
} attr_value;

typedef const char* const_char_ptr;

class AttributeValue {
public:
    enum ValueType { UnknownType, CharType, UCharType, ShortType, UShortType, 
		     IntType, UIntType, LongType, ULongType, FloatType, DoubleType, 
                     StringType, SymbolType, ArrayType, StreamType, CommandType, KeywordType, 
                     ObjectType, EofType, BooleanType, OperatorType, BlankType };

    AttributeValue(ValueType type);
    AttributeValue(ValueType type, attr_value value);
    AttributeValue(AttributeValue&);
    AttributeValue();

    AttributeValue(char);
    AttributeValue(unsigned char);
    AttributeValue(short);
    AttributeValue(unsigned short);
    AttributeValue(int, ValueType);
    AttributeValue(unsigned int, ValueType);
    AttributeValue(unsigned int, unsigned int, ValueType=KeywordType);
    AttributeValue(long);
    AttributeValue(unsigned long);
    AttributeValue(float);
    AttributeValue(double);
    AttributeValue(int classid, void*);
    AttributeValue(AttributeValueList*);
    AttributeValue(const char*);

    virtual ~AttributeValue();

    AttributeValue& operator= (const AttributeValue&);

    ValueType type() const;
    void type(ValueType);
    ValueType aggregate_type() const;
    int type_size() { return type_size(type()); }
    static int type_size(ValueType);

    char& char_ref();
    unsigned char& uchar_ref();
    short& short_ref();
    unsigned short& ushort_ref();
    int& int_ref();
    unsigned int& uint_ref();
    boolean& boolean_ref();
    long& long_ref();
    unsigned long& ulong_ref();
    float& float_ref();
    double& double_ref();
    unsigned int& string_ref();
    unsigned int& symbol_ref();
    void*& obj_ref();
    unsigned int& obj_type_ref();
    AttributeValueList*& array_ref();
    unsigned int& array_type_ref();
    unsigned int& keyid_ref();
    unsigned int& keynarg_ref();

    char char_val();
    unsigned char uchar_val();
    short short_val();
    unsigned short ushort_val();
    int int_val();
    unsigned int uint_val();
    boolean boolean_val();
    long long_val();
    unsigned long ulong_val();
    float float_val();
    double double_val();
    unsigned int string_val();
    unsigned int symbol_val();
    void* obj_val();
    unsigned int obj_type_val();
    AttributeValueList* array_val();
    unsigned int array_type_val();
    unsigned int keyid_val();
    unsigned int keynarg_val();

    const char* string_ptr();
    const char* symbol_ptr();
    int array_len();

    unsigned int command_symid();
    void command_symid(unsigned int);

    void negate();

    boolean is_true() { return type() != UnknownType && boolean_val(); }
    boolean is_false() { return !is_true(); }
    boolean is_type(ValueType t) { return type() == t; }

    boolean is_char() { return is_char(type()); }
    boolean is_short() { return is_short(type()); }
    boolean is_int() { return is_int(type()); }
    boolean is_long() { return is_long(type()); }
    boolean is_float() { return is_float(type()); }
    boolean is_double() { return is_double(type()); }
    boolean is_num() { return is_char() || is_short() || is_int() ||
			 is_long() || is_float() || is_double(); }

    static boolean is_char(ValueType t) 
      { return t==CharType || t==UCharType; }
    static boolean is_short(ValueType t)
      { return t==ShortType || t==UShortType; }
    static boolean is_int(ValueType t)
      { return t==IntType || t==UIntType; }
    static boolean is_long(ValueType t) 
      { return t==LongType || t==ULongType; }
    static boolean is_float(ValueType t)
      { return t==FloatType; }
    static boolean is_double(ValueType t)
      { return t==DoubleType; }

    friend ostream& operator << (ostream& s, const AttributeValue&);

    void* value_ptr() { return &_v; }

protected:
    ValueType _type;
    attr_value _v;
    union { 
      ValueType _aggregate_type;
      unsigned int _command_symid;
    };

};

typedef class AttributeValue _AV; /* for quick casting in debugger */

#endif /* !defined(_attrvalue_h) */
