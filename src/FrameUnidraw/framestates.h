/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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

#ifndef framestates_h
#define framestates_h

#include <IVGlyph/namestate.h>

class FrameNumberState : public NameState {
public:
    FrameNumberState(int =0, const char* desc=nil, int usebg =1);

    int number();
    void number(int, boolean notif =true);

    // same as number methods
    int framenumber(); 
    void framenumber(int, boolean notif =true);
protected:
    int _number;
    char* _desc;
    char buf[256];
    int _usebg;
};

class FrameListState : public NameState {
public:
    FrameListState(int =1);

    int framenumber();
    void framenumber(int, boolean notif =true);
protected:
    int _framenumber;
    char buf[256];
};

#endif
