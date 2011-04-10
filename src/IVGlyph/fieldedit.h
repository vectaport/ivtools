/*
 * Copyright (c) 1995-1996 Vectaport Inc.
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
 * GFieldEditor -- a purely Glyph field editor without Interactors
 */

#ifndef fieldedit_h
#define fieldedit_h

#include <InterViews/input.h>
#include <InterViews/resource.h>

class Action;
class GFieldEditor;

class GFieldEditorAction : public Resource {
protected:
    GFieldEditorAction();
    virtual ~GFieldEditorAction();
public:
    virtual void accept(GFieldEditor*);
    virtual void cancel(GFieldEditor*);
};

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __GFieldEditorCallback(T) T##_GFieldEditorCallback
#define GFieldEditorCallback(T) __GFieldEditorCallback(T)
#define __GFieldEditorMemberFunction(T) T##_GFieldEditorMemberFunction
#define GFieldEditorMemberFunction(T) __GFieldEditorMemberFunction(T)
#else
#define __GFieldEditorCallback(T) T/**/_GFieldEditorCallback
#define GFieldEditorCallback(T) __GFieldEditorCallback(T)
#define __GFieldEditorMemberFunction(T) T/**/_GFieldEditorMemberFunction
#define GFieldEditorMemberFunction(T) __GFieldEditorMemberFunction(T)
#endif

#define declareGFieldEditorCallback(T) \
typedef void (T::*GFieldEditorMemberFunction(T))(GFieldEditor*); \
class GFieldEditorCallback(T) : public GFieldEditorAction { \
public: \
    GFieldEditorCallback(T)( \
	T*, GFieldEditorMemberFunction(T) accept, \
	GFieldEditorMemberFunction(T) cancel \
    ); \
    virtual ~GFieldEditorCallback(T)(); \
\
    virtual void accept(GFieldEditor*); \
    virtual void cancel(GFieldEditor*); \
private: \
    T* obj_; \
    GFieldEditorMemberFunction(T) accept_; \
    GFieldEditorMemberFunction(T) cancel_; \
};

#define implementGFieldEditorCallback(T) \
GFieldEditorCallback(T)::GFieldEditorCallback(T)( \
    T* obj, GFieldEditorMemberFunction(T) accept, \
    GFieldEditorMemberFunction(T) cancel \
) { \
    obj_ = obj; \
    accept_ = accept; \
    cancel_ = cancel; \
} \
\
GFieldEditorCallback(T)::~GFieldEditorCallback(T)() { } \
\
void GFieldEditorCallback(T)::accept(GFieldEditor* f) { (obj_->*accept_)(f); } \
void GFieldEditorCallback(T)::cancel(GFieldEditor* f) { (obj_->*cancel_)(f); }

class EivTextBuffer;

class GFieldEditor : public InputHandler {
public:
    GFieldEditor(char*, GFieldEditorAction* = nil, float minw = 0.0);
    ~GFieldEditor();
    const char* text();
    EivTextBuffer* field() { return field_; }

    virtual void keystroke(const Event&);
    virtual InputHandler* focus_in();
    virtual void focus_out();
    virtual void press(const Event&);
    virtual void drag(const Event&);
    
    int locate(const Event&);

    // editing functions
    void delete_char_forward();
    void delete_char_backward();
    void delete_region();
    void delete_to_eol();
    void beginning_of_line();
    void end_of_line();
    void forward_char();
    void backward_char();
    void insert_char(char);
    void clear_buffer();
    void update();
    void select_all();

protected:
    EivTextBuffer* field_;
    long point_pos_;
    long mark_pos_;
    float minwidth_;
    GFieldEditorAction* action_;
    boolean cursor_is_on_;
    float _frame_thickness;

    void make_body();
    void cursor_on();
    void cursor_off();
};

#endif
