/*
 * Copyright (c) 1994,1999 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 * OverlaySelection - manages a set of selected components in an overlay style.
 */

#ifndef ov_selection_h
#define ov_selection_h

#include <Unidraw/selection.h>

class OverlayView;
class OverlayViewer;

//: derived Selection object
// specialized Selection object with support for highlighting by
// changing graphic state (as opposed to only using tic mark handles).
// Also adds ability to globally disable tic mark handles.
class OverlaySelection : public Selection {
public:
    OverlaySelection(OverlaySelection* = nil);
    OverlaySelection(Selection*);

    virtual void Show(Viewer* = nil);	
    virtual void Hide(Viewer* = nil);
    virtual void Update(Viewer* = nil); 
    virtual void Clear(Viewer* = nil);

    virtual void RepairClear(Viewer*, boolean);

    virtual void Exclusive(Selection*);
    virtual void Merge(Selection*);    

    virtual void ShowHandles(Viewer* = nil);
    virtual void HideHandles(Viewer* = nil);

    virtual OverlayViewer* ShowHighlights(Viewer* = nil);
    virtual OverlayViewer* HideHighlights(Viewer* = nil);

    void EnableHandles();
    void DisableHandles();
    boolean HandlesEnabled();
    boolean HandlesDisabled();

    OverlayView* GetView(Iterator);
    void SetView(OverlayView*, Iterator&);

    OverlaySelection* ViewsWithin(IntCoord l, IntCoord b, IntCoord r, IntCoord t);

    virtual void Reserve() { return; }
    // for use of derived classes

protected:
    boolean _clear_to_repair;
    boolean _handles_disabled;
};

#endif
