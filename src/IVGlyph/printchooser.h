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
 * PrintChooser -- select a file or printer to send postscript to
 */

#ifndef print_chooser_h
#define print_chooser_h

#include <IVGlyph/ofilechooser.h>

#include <InterViews/_enter.h>

class PrintChooser;
class PrintChooserImpl;
class String;
class WidgetKit;

class PrintChooser : public OpenFileChooser {
public:
    PrintChooser(
	const String& dir, WidgetKit*, Style*, OpenFileChooserAction* = nil
    );
    PrintChooser( Style* );

    virtual boolean to_printer();
    virtual boolean idraw_format();
};

class PrintChooserImpl : public OpenFileChooserImpl {
public:
    friend class PrintChooser;

    PrintChooserImpl();
    virtual void build();

    virtual void accept_editor(FieldEditor*);
    virtual void cancel_editor(FieldEditor*);

    void to_printer_callback();
    void idraw_format_callback();

    boolean _to_printer;
    boolean _idraw_format;
};

declareActionCallback(PrintChooserImpl)
declareFieldEditorCallback(PrintChooserImpl)

#include <InterViews/_leave.h>

#endif
