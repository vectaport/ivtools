/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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
 * ComValue is the general object for interpreter values stored on the
 * stack and associated with global symbols.
 */

#if !defined(_comvalue_h)
#define _comvalue_h

#include <ComTerp/_comterp.h>
#include <Attribute/attrvalue.h>

class ComTerp;

class ComValue : public AttributeValue {
public:
    ComValue(ComValue&);
    ComValue(AttributeValue&);
    ComValue(postfix_token* token);
    ComValue(ValueType type);
    ComValue();

    ComValue(char);
    ComValue(unsigned char);
    ComValue(short);
    ComValue(unsigned short);
    ComValue(int, ValueType=IntType);
    ComValue(unsigned int, ValueType=IntType);
    ComValue(unsigned int, unsigned int, ValueType=KeywordType);
    ComValue(long);
    ComValue(unsigned long);
    ComValue(float);
    ComValue(double);
    ComValue(int classid, void*);
    ComValue(AttributeValueList*);
    ComValue(const char*);

    virtual ~ComValue();

    ComValue& operator= (const ComValue&);
    void assignval (const ComValue&);

    int narg() const;
    int nkey() const;
    int nids() const;

    boolean unknown() { return ComValue::UnknownType == type(); }
    boolean null() { return unknown(); }

    void* geta(int id); 

    friend ostream& operator << (ostream& s, const ComValue&);

    static void comterp(const ComTerp* comterp) { _comterp = comterp; }
    static const ComTerp* comterp() { return _comterp; }

    static ComValue& nullval();
    static ComValue& trueval();
    static ComValue& falseval();
    static ComValue& blankval();
    static ComValue& unkval();
    static ComValue& oneval();
protected:
    int _narg;
    int _nkey;
    int _nids;

    static const ComTerp* _comterp;
    static ComValue _nullval;
    static ComValue _trueval;
    static ComValue _falseval;
    static ComValue _blankval;
    static ComValue _unkval;
    static ComValue _oneval;
};

#endif /* !defined(_comvalue_h) */
