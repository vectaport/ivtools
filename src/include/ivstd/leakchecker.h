/*
 * Copyright (c) 1998-1999 Vectaport Inc.
 * Copyright (c) 1997 Vectaport Inc., R.B. Kissh & Associates
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
#ifndef leak_checker_h
#define leak_checker_h

#include <stream.h>
using std::cerr;

//: utility for counting undestroyed instances of a class.
// To use create a static instance initialized with the class name, i.e.
// 'static LeakChecker checker("OverlayRaster")', then add a 'checker.create()'
// call to each constructor and a 'checker.destroy()' call to each
// destructor.  When the program is terminated the static instance of the
// leak checker will be destructed, and a count of un-destructed (alive)
// instances will be printed to stderr.
class LeakChecker {
public:
    void create() { _alive++; }
    // increment count of instances, to be called from constructors.
    void destroy() { _alive--; }
    // decrement count of instances, to be called from destructors.

    LeakChecker(const char* c) : _alive(0), _class(c) {}
    ~LeakChecker();

    int alive() {return _alive;}
private:
    int _alive;
    const char* _class;
};

inline LeakChecker::~LeakChecker() {
    cerr << "LEAKCHECKER: " << _class << ", " << _alive << "\n";
}

#endif
