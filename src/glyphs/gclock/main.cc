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

#include <InterViews/enter-scope.h>
#include <Dispatch/dispatcher.h>
#include <Dispatch/iocallback.h>
#include <Time/Time_.h>
#include <Time/Date.h>
#include <Time/obstime.h>
#include <Time/timeglyph.h>
#include <IV-look/kit.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/session.h>
#include <InterViews/window.h>

class UpdatableTime : public ObservableTime {
public:
    UpdatableTime(Time* th = nil) 
    : ObservableTime(th) {}
    void tic_handler(long, long);
protected:
};

declareIOCallback(UpdatableTime)
implementIOCallback(UpdatableTime)

static OptionDesc options[] = {
    { nil }
};

static IOHandler* tic;

void UpdatableTime::tic_handler(long, long) {
    Time* th = new Time();
    time(th);
    delete th;
    Dispatcher::instance().startTimer(1, 0, tic);
}

int main(int argc, char** argv) {
    Session* session = new Session("gclock", argc, argv, options);
    WidgetKit& kit = *WidgetKit::instance();

    UpdatableTime* utime = new UpdatableTime();
    tic = new IOCallback(UpdatableTime)(
	utime, &UpdatableTime::tic_handler);
    TimeGlyph* tg = new TimeGlyph(session->style(), utime);
    Dispatcher::instance().startTimer(1, 0, tic);
    ApplicationWindow* win = new ApplicationWindow(new Background(
	tg, kit.background()));
    return session->run_window(win);
}
