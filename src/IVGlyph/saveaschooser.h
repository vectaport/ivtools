/*
 * Copyright (c) 1995-1996 Vectaport Inc.
 * Copyright (c) 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
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
 * SaveAsChooser -- select a file to save graphic scripts to
 */

#ifndef saveas_chooser_h
#define saveas_chooser_h

#include <IVGlyph/ofilechooser.h>

#include <InterViews/_enter.h>

class SaveAsChooser;
class SaveAsChooserImpl;
class String;
class WidgetKit;

class SaveAsChooser : public OpenFileChooser {
public:
    SaveAsChooser(
	const String& dir, WidgetKit*, Style*, OpenFileChooserAction* = nil,
	boolean gs_button = true, boolean pts_button = true,
	boolean pic_button = false
    );
    SaveAsChooser( Style* );

    virtual boolean saveas_chooser();

    virtual boolean gs_compacted();
    virtual boolean pts_compacted();
    virtual boolean pic_compacted();

    virtual boolean gs_button();
    virtual boolean pts_button();
    virtual boolean pic_button();
};

class SaveAsChooserImpl : public OpenFileChooserImpl {
public:
    friend class SaveAsChooser;

    SaveAsChooserImpl(boolean gs_button, boolean pts_button, boolean pic_button);
    virtual void build();

    void gs_callback();
    void pts_callback();
    void pic_callback();

    boolean _gs_compacted;
    boolean _pts_compacted;
    boolean _pic_compacted;

    boolean _gs_button;
    boolean _pts_button;
    boolean _pic_button;
};

declareActionCallback(SaveAsChooserImpl)
declareFieldEditorCallback(SaveAsChooserImpl)

#include <InterViews/_leave.h>

#endif
