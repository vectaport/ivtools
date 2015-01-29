/*
 * Copyright (c) 1999 Vectaport Inc.
 * Copyright (c) 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
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
 */

#ifndef os_string_h
#define os_string_h

/*
 * String - simple (non-copying) string class
 */

#include <OS/enter-scope.h>

#include <iosfwd>

class String {
public:
#ifdef _DELTA_EXTENSIONS
#pragma __static_class
#endif
    String();
    String(const char*);
    String(const char*, int length);
    String(const String&);
    virtual ~String();

    const char* string() const;
    int length() const;

    virtual unsigned long hash() const;
    virtual String& operator =(const String&);
    virtual String& operator =(const char*);
    virtual boolean operator ==(const String&) const;
    virtual boolean operator ==(const char*) const;
    virtual boolean operator !=(const String&) const;
    virtual boolean operator !=(const char*) const;
    virtual boolean operator >(const String&) const;
    virtual boolean operator >(const char*) const;
    virtual boolean operator >=(const String&) const;
    virtual boolean operator >=(const char*) const;
    virtual boolean operator <(const String&) const;
    virtual boolean operator <(const char*) const;
    virtual boolean operator <=(const String&) const;
    virtual boolean operator <=(const char*) const;

    virtual boolean case_insensitive_equal(const String&) const;
    virtual boolean case_insensitive_equal(const char*) const;

    u_char operator [](int index) const;
    virtual String substr(int start, int length) const;
    String left(int length) const;
    String right(int start) const;

    virtual void set_to_substr(int start, int length);
    void set_to_left(int length);
    void set_to_right(int start);

    virtual int search(int start, u_char) const;
    int index(u_char) const;
    int rindex(u_char) const;

    virtual boolean convert(int&) const;
    virtual boolean convert(long&) const;
    virtual boolean convert(float&) const;
    virtual boolean convert(double&) const;

    virtual boolean null_terminated() const;

    virtual boolean contains(const char*, int start=0) const;

    String before(const char*) const;
    String from(const char*) const;

    int freq(const char* t) const;

    operator const char*() const;

    friend ostream& operator << (ostream& s, const String&);

protected:
    virtual void set_value(const char*);
    virtual void set_value(const char*, int);
private:
    const char* data_;
    int length_;
};

class CopyString : public String {
public:
#ifdef _DELTA_EXTENSIONS
#pragma __static_class
#endif
    CopyString();
    CopyString(const char*);
    CopyString(const char*, int length);
    CopyString(const String&);
    CopyString(const CopyString&);
    virtual ~CopyString();

    virtual String& operator =(const String&);
    virtual String& operator =(const char*);

    virtual boolean null_terminated() const;
protected:
    virtual void set_value(const char*);
    virtual void set_value(const char*, int);
private:
    void free();
};

class NullTerminatedString : public String {
public:
#ifdef _DELTA_EXTENSIONS
#pragma __static_class
#endif
    NullTerminatedString();
    NullTerminatedString(const String&);
    NullTerminatedString(const NullTerminatedString&);
    virtual ~NullTerminatedString();

    virtual String& operator =(const String&);
    virtual String& operator =(const char*);

    virtual boolean null_terminated() const;
private:
    boolean allocated_;

    void assign(const String&);
    void free();
};

inline const char* String::string() const { return data_; }
inline int String::length() const { return length_; }
inline u_char String::operator [](int index) const {
    return ((u_char*)data_)[index];
}

inline String String::left(int length) const { return substr(0, length); }
inline String String::right(int start) const { return substr(start, -1); }

inline void String::set_to_left(int length) { set_to_substr(0, length); }
inline void String::set_to_right(int start) { set_to_substr(start, -1); }

inline int String::index(u_char c) const { return search(0, c); }
inline int String::rindex(u_char c) const { return search(-1, c); }

extern char* strnew(const char*);   /* return a copy of the given string */

#endif
