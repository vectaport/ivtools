/*
 * Copyright (c) 2020 Scott E. Johnston
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
 * ComKit - object to construct objects for a ComEditor
 */

#ifndef com_kit_h
#define com_kit_h

#include <OverlayUnidraw/ovkit.h>

#include <InterViews/_enter.h>

class ComKit : public OverlayKit {
public:
    ComKit();
    virtual ~ComKit();
    virtual MenuItem* MakeEditMenu();

    static ComKit* Instance();
    // default instance of ComKit.

protected:
    static ComKit* _comkit;
};

#include <OverlayUnidraw/ovdoer.h>
#include <InterViews/action.h>

declareActionCallback(ComKit)

#include <InterViews/_leave.h>

#endif
