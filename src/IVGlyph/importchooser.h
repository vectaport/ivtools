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
 * ImportChooser -- select a file or importer to send postscript to
 */

#ifndef import_chooser_h
#define import_chooser_h

#include <IVGlyph/ofilechooser.h>

#include <InterViews/_enter.h>

class Button;
class ImportChooser;
class ImportChooserImpl;
class String;
class WidgetKit;

class ImportChooser : public OpenFileChooser {
public:
    ImportChooser(
	const String& dir, WidgetKit*, Style*, OpenFileChooserAction* = nil,
	boolean centered_bttn = true, boolean by_pathname_bttn = true,
	boolean from_command_bttn = true, boolean auto_convert_bttn = false
    );
#if 0
    ImportChooser( Style* );
#endif
    virtual ~ImportChooser();

    virtual boolean centered();
    virtual boolean by_pathname();
    virtual boolean from_command();
    virtual boolean auto_convert();

    void set_centered(boolean);
    void set_by_pathname(boolean);
    void set_from_command(boolean);
    void set_auto_convert(boolean);

    static ImportChooser& instance();
    static void instance(ImportChooser*);

protected:
    static ImportChooser* instance_;
};

class ImportChooserImpl : public OpenFileChooserImpl {
public:
    friend class ImportChooser;

    ImportChooserImpl();
    virtual void build();

    virtual void accept_editor(FieldEditor*);
    virtual void cancel_editor(FieldEditor*);

    void centered_callback();
    void by_pathname_callback();
    void from_command_callback();
    void auto_convert_callback();

    boolean _centered;
    boolean _by_pathname;
    boolean _from_command;
    boolean _auto_convert;
    Button* _cbutton;
    Button* _fbutton;
    Button* _mbutton;
    Button* _abutton;
    Action* _centered_action;
    Action* _by_pathname_action;
    Action* _from_command_action;
    Action* _auto_convert_action;
};

declareActionCallback(ImportChooserImpl)
declareFieldEditorCallback(ImportChooserImpl)

#include <InterViews/_leave.h>

#endif
