/*
 * Copyright (c) 1998 R.B. Kissh & Associates
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

#ifndef xt_session_h
#define xt_session_h

#include <InterViews/session.h>

// is there any other way to tell it what an AppContext is?
#include "Xd.h"
#include <X11/Intrinsic.h>
#include "Xud.h"

// ### ugly
#include "xtintrinsic.h"

class XtSession : public Session {
public:
  XtSession(
    const char*, int& argc, char** argv, const OptionDesc* = nil,
    const PropertyData* = nil, XtAppContext = nil, Display* = nil
  );

  XtAppContext get() const;
  void set(XtAppContext);
  virtual void loopHook();

  virtual int run();
  virtual void read(Event&, boolean (*test)() = nil);

  void xt_init();

  virtual void workProc();
  static XtfBoolean doWorkProc(XtPointer);

protected:
  XtAppContext _app_context;
};

#endif
