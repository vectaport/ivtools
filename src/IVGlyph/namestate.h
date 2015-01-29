/*
 * Copyright (c) 1994 Vectaport Inc.
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

#ifndef namestates_h
#define namestates_h

#include <InterViews/monoglyph.h>
#include <InterViews/observe.h>
#include <Dispatch/iocallback.h>

class NameState;
class Patch;

class NameView : public MonoGlyph, public Observer {
public:
    NameView(NameState*);
    virtual ~NameView();

    virtual void update(Observable*);
  void blink_view(long =0, long =long(0.5 * 1000000));
  void stop_blinking();
protected:
  IOHandler* _blink_handler;
  int _blink_state;
  int _blink_in;
    Patch* _label;
    NameState* st1;
};

declareIOCallback(NameView)

class NameState : public Observable {
public:
    NameState(const char*);
    virtual ~NameState();

    const char* name();
    void name(const char*, boolean notif =true);
protected:
    char* _name;
};

#endif
