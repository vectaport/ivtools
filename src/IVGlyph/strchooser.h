/*
 * Copyright (c) 1993 David B. Hollenbeck
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notice and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * David B. Hollenbeck may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of David B. Hollenbeck.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL DAVID B. HOLLENBECK BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * StringChooser -- select a file
 */

#ifndef ivlook_strchooser_h
#define ivlook_strchooser_h

#include <InterViews/dialog.h>
#include <InterViews/resource.h>
#include <IVGlyph/strlist.h>


class StrChooser;
class StrChooserImpl;
class WidgetKit;

typedef void (*strchooser_callback)(void*);

class StrChooserAction : public Resource {
protected:
    StrChooserAction();
    virtual ~StrChooserAction();
public:
    virtual void execute(StrChooser*, boolean accept);
};

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define StrChooserCallback(T) T##_StrChooserCallback
#define StrChooserMemberFunction(T) T##_StrChooserMemberFunction
#else
#define StrChooserCallback(T) T/**/_StrChooserCallback
#define StrChooserMemberFunction(T) T/**/_StrChooserMemberFunction
#endif

#define declareStrChooserCallback(T) \
typedef void (T::*StrChooserMemberFunction(T))(StrChooser*, boolean); \
class StrChooserCallback(T) : public StrChooserAction { \
public: \
    StrChooserCallback(T)(T*, StrChooserMemberFunction(T)); \
    virtual ~StrChooserCallback(T)(); \
\
    virtual void execute(StrChooser*, boolean accept); \
private: \
    T* obj_; \
    StrChooserMemberFunction(T) func_; \
};

#define implementStrChooserCallback(T) \
StrChooserCallback(T)::StrChooserCallback(T)( \
    T* obj, StrChooserMemberFunction(T) func \
) { \
    obj_ = obj; \
    func_ = func; \
} \
\
StrChooserCallback(T)::~StrChooserCallback(T)() { } \
\
void StrChooserCallback(T)::execute(StrChooser* f, boolean accept) { \
    StrChooserMemberFunction(T) pf = func_; \
    (obj_->*pf)(f, accept); \
}

class StrChooser : public Dialog {
public:
    StrChooser(StringList *, String *, WidgetKit*, Style*, StrChooserAction* = nil,
        boolean embed = false, strchooser_callback outfunc = nil, void* base = nil);
    virtual ~StrChooser();

    virtual GlyphIndex selected() const;
    virtual void dismiss(boolean);

private:
    StrChooserImpl* impl_;
};

#endif
