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
 * ExportChooser -- select a file/command to export graphics to
 */

#ifndef export_chooser_h
#define export_chooser_h

#include <IVGlyph/enumform.h>
#include <IVGlyph/printchooser.h>

#include <InterViews/_enter.h>

class ExportChooser;
class ExportChooserImpl;
class ExportEnumEditor;
class String;
class WidgetKit;

class ExportChooser : public PrintChooser {
public:
    ExportChooser(
	const String& dir, WidgetKit*, Style*, const char** formats = nil,
	int nformats = 0, const char** commands = nil, OpenFileChooserAction* = nil,
	boolean execute_button = true, boolean by_pathname_button = true
    );

    void set_formats(const char** formats, int nformats, const char** commands = nil);

    virtual const char* format();
    virtual boolean idraw_format();
    virtual boolean postscript_format();
    virtual boolean svg_format();
    virtual boolean by_pathname_flag();
    virtual boolean execute_flag();
    virtual boolean by_pathname_flag_button();
    virtual boolean execute_flag_button();

};

class ExportChooserImpl : public PrintChooserImpl {
public:
    ExportChooserImpl(boolean execute_button, boolean by_pathname_button);

    virtual void free();
    virtual void build();

    void set_formats(const char** formats, int nformats, const char** commands = nil);
    virtual const char* format();
    virtual const char* command(const char* format);

    void to_printer_callback();
    void by_pathname_callback();

    friend class ExportChooser;
    friend class ExportEnumEditor;
protected:
    int _nformats;
    char** _formats;
    char** _commands;
    ObservableEnum* _obse;
    RadioEnumEditor* _editor;
    boolean _execute_flag;
    boolean _execute_flag_button;
    boolean _by_pathname_flag;
    boolean _by_pathname_flag_button;
};

class ExportEnumEditor : public RadioEnumEditor {
public:
    ExportEnumEditor(ObservableEnum* obs, char* labl, ExportChooserImpl* eci);

    void edit(String);
protected:
    void build();
    void buildbox();
    ExportChooserImpl* _eci;
};

declareActionCallback(ExportChooserImpl)

#include <InterViews/_leave.h>

#endif
