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

#include <InterViews/enter-scope.h>
#include <Time/Time_.h>
#include <Time/Date.h>
#include <Time/timeglyph.h>
#include <IV-look/kit.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/session.h>
#include <InterViews/window.h>

static OptionDesc options[] = {
    { nil }
};

int main(int argc, char** argv) {
    Session* session = new Session("timestamp", argc, argv, options);
    WidgetKit& kit = *WidgetKit::instance();

    ApplicationWindow* win = new ApplicationWindow(new Background(
	new TimeGlyph(session->style(), nil, true),
	kit.background()));
    return session->run_window(win);
}
