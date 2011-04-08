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

#ifndef obstime_h
#define obstime_h

#include <InterViews/observe.h>
#include <Time/Time.h>

class ObservableTime : public Observable {
public:
    ObservableTime(Time* = nil);
    virtual ~ObservableTime();

    void time(Time*);
    void time(Time&);
    Time* time();

    void addsecond(long);
    void addminute(long);
    void addhour(long);
    void addday(long);

    void plussec() { addsecond(1); }
    void minussec() { addsecond(-1); }
    void plusmin() { addminute(1); }
    void minusmin() { addminute(-1); }
    void plushour() { addhour(1); }
    void minushour() { addhour(-1); }
    void plusday() { addday(1); }
    void minusday() { addday(-1); }
    
protected:
    Time* th_;
};

#endif
