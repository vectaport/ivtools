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
 * StrChooser -- select a string
 */

#include <IV-look/choice.h>
#include "strchooser.h"
#include <IV-look/dialogs.h>
#include <IV-look/fbrowser.h>
#include <IV-look/kit.h>
#include <InterViews/action.h>
#include <InterViews/font.h>
#include <InterViews/hit.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <OS/string.h>

class StrChooserImpl {
protected:
    strchooser_callback outfunc_;
    void* base_;

private:
    friend class StrChooser;

    WidgetKit  * kit_;
    StrChooser * fchooser_;
    FileBrowser* fbrowser_;
    StrChooserAction* action_;
    GlyphIndex   selected_;
    Style      * style_;
    Action     * update_;
    StringList * strings_;
    String     * caption_;
    boolean      embed_;

    void init(StrChooser*, Style*, StrChooserAction*);
    void free();
    void build();
    void clear();
    void accept_browser();
    void cancel_browser();
};

declareActionCallback(StrChooserImpl)
implementActionCallback(StrChooserImpl)

StrChooser::StrChooser(StringList* sl, String* caption, WidgetKit* kit, Style* s, 
    StrChooserAction* a, boolean embed, strchooser_callback outfunc, void* base) 
: Dialog(nil, s) {
    impl_ = new StrChooserImpl;
    StrChooserImpl& sci = *impl_;
    sci.kit_ = kit;
    sci.strings_ = sl;
    sci.caption_ = caption;
    sci.embed_ = embed;
    sci.outfunc_ = outfunc;
    sci.base_ = base;
    sci.init(this, s, a);
}

StrChooser::~StrChooser() {
    impl_->free();
    delete impl_;
}

GlyphIndex StrChooser::selected() const {
    return impl_->selected_;
}

void StrChooser::dismiss(boolean accept) {
    Dialog::dismiss(accept);
    StrChooserImpl& fc = *impl_;
    if (fc.action_ != nil) {
	fc.action_->execute(this, accept);
    }
}

/** class StrChooserImpl **/

void StrChooserImpl::init(
    StrChooser* chooser, Style* s, StrChooserAction* a
) {
    fchooser_ = chooser;
    fbrowser_ = nil;
    Resource::ref(a);
    action_ = a;
    style_ = new Style(s);
    Resource::ref(style_);
    style_->alias("StrChooser");
    style_->alias("Dialog");
    update_ = new ActionCallback(StrChooserImpl)(
	this, &StrChooserImpl::build
    );
    style_->add_trigger_any(update_);
    build();
}

void StrChooserImpl::free() {
    Resource::unref(action_);
    style_->remove_trigger_any(update_);
    Resource::unref(style_);
}

void StrChooserImpl::build() {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    kit.push_style();
    kit.style(s);
    String caption("");
    s->find_attribute("caption", caption);
    s->find_attribute("subcaption", *caption_);
    String open("OK");
    s->find_attribute("ok", open);
    String close("Cancel");
    s->find_attribute("cancel", close);
    long rows = 10;
    s->find_attribute("rows", rows);
    const Font* f = kit.font();
    FontBoundingBox bbox;
    f->font_bbox(bbox);
    Coord height = rows * (bbox.ascent() + bbox.descent()) + 1.0;
    Coord width;
    if (!s->find_attribute("width", width)) {
	Coord defwidth = 16 * f->width('m') + 3.0;
	Coord maxwidth = 0;
	for (ListItr(StringList) i(*strings_); i.more(); i.next()) {
	  Coord curwidth = f->width(i.cur().string(), i.cur().length()) + 3.0;
	  if (curwidth > maxwidth)
	    maxwidth = curwidth;
	}
	width = maxwidth > defwidth ? maxwidth : defwidth;
    }

    Action* accept = new ActionCallback(StrChooserImpl)(
	this, &StrChooserImpl::accept_browser
    );
    Action* cancel = new ActionCallback(StrChooserImpl)(
	this, &StrChooserImpl::cancel_browser
    );
    fbrowser_ = new FileBrowser(kit_, accept, cancel);

    fchooser_->remove_all_input_handlers();
    fchooser_->append_input_handler(fbrowser_);

    Glyph* g = layout.vbox();
    if (caption.length() > 0) {
	g->append(layout.rmargin(kit.fancy_label(caption), 5.0, fil, 0.0));
    }
    if (caption_->length() > 0) {
	g->append(layout.rmargin(kit.fancy_label(*caption_), 5.0, fil, 0.0));
    }
    g->append(layout.vglue(5.0, 0.0, 2.0));
    g->append(
	layout.hbox(
	    layout.vcenter(
		kit.inset_frame(
		    layout.margin(
			layout.natural_span(fbrowser_, width, height), 1.0
		    )
		),
		1.0
	    ),
	    layout.hspace(4.0),
	    kit.vscroll_bar(fbrowser_->adjustable())
	)
    );
    if (!embed_) {
        g->append(layout.vspace(15.0));
    	g->append(
	    layout.hbox(
	    	layout.hglue(10.0),
	    	layout.vcenter(kit.default_button(open, accept)),
	    	layout.hglue(10.0, 0.0, 5.0),
	    	layout.vcenter(kit.push_button(close, cancel)),
	    	layout.hglue(10.0)
	    )
    	);

        fchooser_->body(
	    layout.back(
	        layout.vcenter(kit.outset_frame(layout.margin(g, 5.0)), 1.0),
	        new Target(nil, TargetPrimitiveHit)
	    )
        );
    } else {
        fchooser_->body(
	    layout.back(
	        layout.vcenter(g, 1.0), new Target(nil, TargetPrimitiveHit)
	    )
        );
    }
    fchooser_->focus(fbrowser_);

    for (ListItr(StringList) i(*strings_); i.more(); i.next()) {
        Glyph* name = kit.label(i.cur());
        Glyph* label = new Target(
	    layout.h_margin(name, 3.0, 0.0, 0.0, 15.0, fil, 0.0),
    	    TargetPrimitiveHit);
        TelltaleState* t = new TelltaleState(TelltaleState::is_enabled);
        fbrowser_->append_selectable(t);
        fbrowser_->append(new ChoiceItem(t, label, kit.bright_inset_frame(label)));
        fbrowser_->refresh();
    }

    kit.pop_style();
}

void StrChooserImpl::clear() {
    Browser& b = *fbrowser_;
    b.select(-1);
    GlyphIndex n = b.count();
    for (GlyphIndex i = 0; i < n; i++) {
	b.remove_selectable(0);
	b.remove(0);
    }
}

void StrChooserImpl::accept_browser() {
    GlyphIndex i = fbrowser_->selected();
    if (i == -1) {
	return;
    }
    selected_ = i;
    if (embed_ && outfunc_ != nil)
       (*outfunc_)(base_);

    fchooser_->dismiss(true);
}

void StrChooserImpl::cancel_browser() {
    selected_ = -1;
    fchooser_->dismiss(false);
}


/** class StrChooserAction **/

StrChooserAction::StrChooserAction() { }
StrChooserAction::~StrChooserAction() { }
void StrChooserAction::execute(StrChooser*, boolean) { }
