/*
 * Copyright (c) 1998,1999 Vectaport Inc.
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

#if !defined(framefunc_h)
#define _framefunc_h

#include <ComUnidraw/unifunc.h>

//: interpreter command to move current frame.
// moveframe([num] :abs) -- move frame by num or to num if :abs
class MoveFrameFunc : public UnidrawFunc {
public:
    MoveFrameFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([num] :abs) -- move frame by num or to num if :abs"; }
};

//: interpreter command to create new frame.
// createframe(:before) -- create and move to new frame
class CreateFrameFunc : public UnidrawFunc {
public:
    CreateFrameFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "%s(:before) -- create and move to new frame"; }
};

//: interpreter command to toggle the autonewframe flag
// autonewframe(:on :off) -- command to toggle the autonewframe flag
class AutoNewFrameFunc : public UnidrawFunc {
public:
    AutoNewFrameFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "%s(:on :off) -- command to toggle autonewframe"; }
};

#endif /* !defined(_framefunc_h) */
