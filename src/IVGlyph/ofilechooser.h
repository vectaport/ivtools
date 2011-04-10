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
 * OpenFileChooser -- select a file
 */

#ifndef open_file_chooser_h
#define open_file_chooser_h

#include <InterViews/dialog.h>
#include <InterViews/resource.h>
#include <OS/directory.h>
#include <InterViews/action.h>
#include <IV-look/field.h>

#include <InterViews/_enter.h>

class Action;
class Directory;
class FieldEditor;
class FieldEditorAction;
class FileBrowser;
class Glyph;
class ObservableText;
class OpenFileChooser;
class OpenFileChooserImpl;
class String;
class TextObserver;
class TransientWindow;
class WidgetKit;

class OpenFileChooserAction : public Resource {
protected:
    OpenFileChooserAction();
    virtual ~OpenFileChooserAction();
public:
    virtual void execute(OpenFileChooser*, boolean accept);
};

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define OpenFileChooserCallback(T) T##_OpenFileChooserCallback
#define OpenFileChooserMemberFunction(T) T##_OpenFileChooserMemberFunction
#else
#define OpenFileChooserCallback(T) T/**/_OpenFileChooserCallback
#define OpenFileChooserMemberFunction(T) T/**/_OpenFileChooserMemberFunction
#endif

#define declareOpenFileChooserCallback(T) \
typedef void (T::*OpenFileChooserMemberFunction(T))(OpenFileChooser*, boolean); \
class OpenFileChooserCallback(T) : public OpenFileChooserAction { \
public: \
    OpenFileChooserCallback(T)(T*, OpenFileChooserMemberFunction(T)); \
    virtual ~OpenFileChooserCallback(T)(); \
\
    virtual void execute(OpenFileChooser*, boolean accept); \
private: \
    T* obj_; \
    OpenFileChooserMemberFunction(T) func_; \
};

#define implementOpenFileChooserCallback(T) \
OpenFileChooserCallback(T)::OpenFileChooserCallback(T)( \
    T* obj, OpenFileChooserMemberFunction(T) func \
) { \
    obj_ = obj; \
    func_ = func; \
} \
\
OpenFileChooserCallback(T)::~OpenFileChooserCallback(T)() { } \
\
void OpenFileChooserCallback(T)::execute(OpenFileChooser* f, boolean accept) { \
    OpenFileChooserMemberFunction(T) pf = func_; \
    (obj_->*pf)(f, accept); \
}

class OpenFileChooser : public Dialog {
public:
    OpenFileChooser(
	const String& dir, WidgetKit*, Style*, OpenFileChooserAction* = nil
    );
    OpenFileChooser( Style* );
    virtual ~OpenFileChooser();

    virtual const String* selected() const;
    virtual void reread();
    virtual void dismiss(boolean);

    virtual boolean post_for_aligned(Window*, float xalign, float yalign);
    void unmap();

    virtual boolean saveas_chooser();
    TransientWindow* twindow();
    virtual void updatecaption();

    static boolean url_use_ok();
    // test if url use ok for pathnames.  Same as ParamList method.
    static boolean urltest(const char*);
    // test if pathname looks like a URL. Same as ParamList method.

    static int bintest(const char* name);
    // return 0 if executable can be found, otherwise -1.
    static boolean bincheck(const char* name);
    // return true if executable can be found.
protected:
    OpenFileChooserImpl* impl_;
    TransientWindow* _t;
};

class OpenFileChooserImpl {
public:
    friend class OpenFileChooser;

    String* name_;
    WidgetKit* kit_;
    OpenFileChooser* fchooser_;
    FileBrowser* fbrowser_;
    FieldEditor* editor_;
    FieldEditor* filter_;
    FieldEditor* directory_filter_;
    int* filter_map_;
    Directory* dir_;
    OpenFileChooserAction* action_;
    const String* selected_;
    Style* style_;
    Action* update_;
    ObservableText* caption_;
    TextObserver* captionview_;
    ObservableText* subcaption_;
    TextObserver* subcaptionview_;

    void init(OpenFileChooser*, Style*, OpenFileChooserAction*);
    virtual void free();
    virtual void build();
    virtual void updatecaption();
    void clear();
    void load();
    FieldEditor* add_filter(
	Style*,
	const char* pattern_attribute, const char* default_pattern,
	const char* caption_attribute, const char* default_caption,
	Glyph*, FieldEditorAction*
    );
    boolean filtered(const String&, FieldEditor*);
    virtual void accept_browser();
    virtual void cancel_browser();
    virtual void accept_editor(FieldEditor*);
    virtual void cancel_editor(FieldEditor*);
    virtual void accept_filter(FieldEditor*);
    boolean chdir(const String&);
};

declareFieldEditorCallback(OpenFileChooserImpl)

declareActionCallback(OpenFileChooserImpl)

#include <InterViews/_leave.h>

#endif
