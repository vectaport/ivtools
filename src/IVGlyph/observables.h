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

#ifndef observables_h
#define observables_h

#include <InterViews/observe.h>

class String;
class StringList;

class AcceptingObservable : public Observable {
public:
    AcceptingObservable() {}

    virtual void accept() = 0;
protected:
};

class ObservableBoolean : public AcceptingObservable {
public:
    ObservableBoolean(boolean v =false, boolean* p =nil);
    ObservableBoolean(boolean* p);
    virtual ~ObservableBoolean();

    boolean value();
    void setvalue(boolean);
    void settrue();
    void setfalse();

    virtual void accept();
protected:
    boolean _value;
    boolean* ptr;
};

class ObservableEnum : public AcceptingObservable {
public:
    ObservableEnum(StringList*, int* ip = nil, char** sp =nil);
    ObservableEnum(StringList*, int* ip);
    virtual ~ObservableEnum();
    
    int maxvalue();
    String labelvalue(int);
    
    int intvalue();
    String labelvalue();
    int value(String);
    
    void setvalue(int);
    void setvalue(String);

    void prepend(const String&);
    void append(const String&);
    void insert(long index, const String&);
    virtual void remove(long index);
    boolean listchanged() { return changed_; }

    virtual void accept();
protected:
    int maxval_;
    int curval_;
    int* intptr;
    char** strptr;
    StringList* strings_;
    boolean changed_;
};

class ObservableText : public AcceptingObservable {
public:
    ObservableText(const char*, char** p =nil);
    ObservableText(char** p);
    virtual ~ObservableText();

    char* textvalue();
    void textvalue(const char*);

    virtual void accept();
protected:
    char* text;
    char** ptr;
};

#endif
