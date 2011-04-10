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

/*
 * Copyright (c) 1993 2001 S.A.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * 2001 S.A. may not be used in any advertising or publicity relating to the
 * software without the specific, prior written permission of 2001 S.A.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL 2001 S.A. BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef xt_window_h
#define xt_window_h

#include "xtintrinsic.h"

#include <InterViews/window.h>

// ### good place?
typedef int XCoord;

struct _GlyphWidgetRec;

class WidgetWindow : public ManagedWindow {
public:
    WidgetWindow(_GlyphWidgetRec* w, Glyph* g);

    virtual ~WidgetWindow();

    virtual void map();
    virtual void set_attributes();

    virtual void compute_geometry();
    virtual void bind();
    virtual void do_map();

    virtual void xt_initialize(Widget req, ArgList, Cardinal*);
    virtual void xt_realize(XtValueMask* mask, XSetWindowAttributes* attr);
    virtual void xt_expose(XEvent*, Region);
    virtual XtGeometryResult xt_query_geometry(
	XtWidgetGeometry*, XtWidgetGeometry*
    );
    virtual XtfBoolean xt_set_values(Widget, Widget, ArgList, Cardinal*);
    virtual void xt_destroy();

protected:

    struct _GlyphWidgetRec* widget_;
    static void xt_structure_event(Widget, XtPointer, XEvent*, XtfBoolean*);

    static void xt_events(Widget, XtPointer, XEvent*, XtfBoolean*);
};
    

void initWidget(Widget, Glyph*);


#endif
