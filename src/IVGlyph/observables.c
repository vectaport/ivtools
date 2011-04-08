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

#include <IVGlyph/observables.h>
#include <IVGlyph/strlist.h>

#include <string.h>

/*****************************************************************************/

ObservableBoolean::ObservableBoolean(boolean v, boolean* p)
: AcceptingObservable()
{
    _value = v;
    ptr = p;
}

ObservableBoolean::ObservableBoolean(boolean* p)
: AcceptingObservable()
{
    _value = *p;
    ptr = p;
}

ObservableBoolean::~ObservableBoolean() {}

boolean ObservableBoolean::value() { return _value; }

void ObservableBoolean::setvalue(boolean v) {
    _value = v;
    notify();
}

void ObservableBoolean::settrue() {
    setvalue(true);
}

void ObservableBoolean::setfalse() {
    setvalue(false);
}

void ObservableBoolean::accept() {
    if (ptr)
	*ptr = _value;
}

/*****************************************************************************/

ObservableEnum::ObservableEnum(StringList* sl, int* ip, char** sp)
: AcceptingObservable()
{

    maxval_ = sl->count();
    strings_ = sl;
    curval_ = 0;
    changed_ = false;
    intptr = ip;
    strptr = sp;
}

ObservableEnum::ObservableEnum(StringList* sl, int* ip)
: AcceptingObservable()
{

    maxval_ = sl->count();
    strings_ = sl;
    curval_ = *ip;
    changed_ = false;
    intptr = ip;
    strptr = nil;
}

ObservableEnum::~ObservableEnum() {
}

int ObservableEnum::maxvalue() { return maxval_; }

String ObservableEnum::labelvalue(int n) {
    return strings_->item(n);
}

int ObservableEnum::intvalue() { return curval_; }

String ObservableEnum::labelvalue() { return strings_->item(curval_); }

int ObservableEnum::value(String s) {
    for (int i = 0; i < strings_->count(); i++) {
	if (s == strings_->item(i))
	    return i;
    }
    return -1;
}

void ObservableEnum::setvalue(int n) {
    if (n >= 0 && n < maxval_)
	curval_ = n;
    notify();
}

void ObservableEnum::setvalue(String lab) {
    for (int i = 0; i < maxval_; i++) {
	if (lab == strings_->item(i)) {
	    curval_ = i;
	    break;
	}
    }
    notify();
}

void ObservableEnum::prepend(const String& s) {
    strings_->prepend(s);
    maxval_ += 1;
    curval_ += 1;
    changed_ = true;
    notify();
    changed_ = false;
}

void ObservableEnum::append(const String& s) {
    strings_->append(s);
    maxval_ += 1;
    changed_ = true;
    notify();
    changed_ = false;
}

void ObservableEnum::insert(long index, const String& s) {
    strings_->insert(index, s);
    maxval_ += 1;
    if (index >= curval_)
	curval_ += 1;
    changed_ = true;
    notify();
    changed_ = false;

}

void ObservableEnum::remove(long index) {
    strings_->remove(index);
    maxval_ -= 1;
    if (curval_ == index) {
	if (curval_ > 0)
	    curval_ -= 1;
    }
    else if (curval_ > index)
	curval_ -= 1;
    changed_ = true;
    notify();
    changed_ = false;

}

void ObservableEnum::accept() {
    if (intptr)
	*intptr = curval_;
    if (strptr)
	strcpy(*strptr, strings_->item(curval_).string());
}

/*****************************************************************************/

ObservableText::ObservableText(const char* txt, char** p)
: AcceptingObservable()
{
    if (txt)
	text = strdup(txt);
    else
	text = nil;
    ptr = p;
}

ObservableText::ObservableText(char** p)
: AcceptingObservable()
{
    if (*p)
	text = strdup(*p);
    else
	text = nil;
    ptr = p;
}

ObservableText::~ObservableText() {
    if (text)
	delete [] text;
}

char* ObservableText::textvalue() {
    return text;
}

void ObservableText::textvalue(const char* txt) {
    if (text)
	delete [] text;
    text = strdup(txt);
    notify();
}

void ObservableText::accept() {
    if (ptr)
	strcpy(*ptr, text);
}
