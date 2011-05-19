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
 * Overlay Text component definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/manips.h>
#include <Unidraw/selection.h>
#include <Unidraw/statevars.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Components/text.h>

#include <Unidraw/Commands/align.h>
#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/font.h>

#include <Unidraw/Graphic/picture.h>

#include <Unidraw/Tools/tool.h>

#include <InterViews/event.h>
#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/textbuffer.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <Attribute/attrlist.h>

#include <OS/math.h>

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stream.h>
#include <string.h>

/*****************************************************************************/

ParamList* TextOvComp::_ovtext_params = nil;
int TextOvComp::_symid = -1;

/*****************************************************************************/

ClassId TextOvComp::GetClassId () { return OVTEXT_COMP; }

boolean TextOvComp::IsA (ClassId id) {
    return OVTEXT_COMP == id || OverlayComp::IsA(id);
}

Component* TextOvComp::Copy () {
    TextOvComp* comp =
      new TextOvComp((TextGraphic*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

TextOvComp::TextOvComp (TextGraphic* graphic, OverlayComp* parent) : OverlayComp(graphic, parent) { }

TextOvComp::TextOvComp(istream& in, OverlayComp* parent) 
: OverlayComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this); 

    /* correct font vertical position */
    PSFont* f = _gr->GetFont();
    float sep = 1 - f->GetLineHt();
    Transformer* t = _gr->GetTransformer();
    float dx = 0., dy = sep;

    if (t != nil) {
        float x0, y0, x1, y1;
        t->Transform(0., 0., x0, y0);
        t->Transform(0., sep, x1, y1);
        dx = x1 - x0;
        dy = y1 - y0;
    }
    _gr->Translate(dx, dy);
}
    
ParamList* TextOvComp::GetParamList() {
    if (!_ovtext_params) 
	GrowParamList(_ovtext_params = new ParamList());
    return _ovtext_params;
}

void TextOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("text", ParamStruct::required, &TextScript::ReadText,
		  this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

void TextOvComp::Interpret (Command* cmd) {
    TextGraphic* gr = (TextGraphic*) GetGraphic();

    if (cmd->IsA(BRUSH_CMD) || cmd->IsA(PATTERN_CMD)) {
        // do nothing

    } else if (cmd->IsA(FONT_CMD)) {
        PSFont* font = ((FontCmd*) cmd)->GetFont();
        cmd->Store(this, new VoidData(gr->GetFont()));
        gr->SetFont(font);
        gr->SetLineHeight(font->GetLineHt());      // hack; should be state var
        Notify();

    } else {
        OverlayComp::Interpret(cmd);
    }
}

void TextOvComp::Uninterpret (Command* cmd) {
    Graphic* gr = GetGraphic();

    if (cmd->IsA(BRUSH_CMD) || cmd->IsA(PATTERN_CMD)) {
        // do nothing

    } else if (cmd->IsA(FONT_CMD)) {
        VoidData* vd = (VoidData*) cmd->Recall(this);
        PSFont* font = (PSFont*) vd->_void;
        gr->SetFont(font);
        Notify();

    } else {
        OverlayComp::Uninterpret(cmd);
    }
}

TextGraphic* TextOvComp::GetText () { return (TextGraphic*) GetGraphic(); }

boolean TextOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    TextGraphic* texta = GetText();
    TextGraphic* textb = ((TextOvComp&)comp).GetText();
    int lha = texta->GetLineHeight();
    int lhb = textb->GetLineHeight();
    return
	lha == lhb && 
	strcmp(texta->GetOriginal(), textb->GetOriginal()) == 0 &&
	OverlayComp::operator==(comp);
}

/****************************************************************************/

TextOvComp* TextOvView::GetTextOvComp () { return (TextOvComp*) GetSubject(); }
ClassId TextOvView::GetClassId () { return OVTEXT_VIEW; }

boolean TextOvView::IsA (ClassId id) {
    return OVTEXT_VIEW == id || OverlayView::IsA(id);
}

TextOvView::TextOvView (TextOvComp* subj) : OverlayView(subj) { }

boolean TextOvView::TextChanged () {
    TextGraphic* gview = (TextGraphic*) GetGraphic();
    TextGraphic* gsubj = (TextGraphic*) GetTextOvComp()->GetGraphic();
    
    return *gview != *gsubj;
}

void TextOvView::Interpret (Command* cmd) {
    if (cmd->IsA(ALIGNTOGRID_CMD)) {
        Transformer total;
        GetGraphic()->TotalTransformation(total);

        float tx0, ty0;
        total.Transform(0., 0., tx0, ty0);
        ((AlignToGridCmd*) cmd)->Align(this, tx0, ty0);

    } else {
        OverlayView::Interpret(cmd);
    }
}

void TextOvView::Update () {
    TextGraphic* gview = (TextGraphic*) GetGraphic();
    TextGraphic* gsubj = (TextGraphic*) GetTextOvComp()->GetGraphic();

    IncurDamage(gview);
    * (Graphic*) gview = * (Graphic*) gsubj;
    gview->SetLineHeight(gsubj->GetLineHeight());
    IncurDamage(gview);
    EraseHandles();
}

Manipulator* TextOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Manipulator* m = nil;
    Editor* ed = v->GetEditor();
    int tabWidth = Math::round(.5*ivinch);

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        FontVar* fontVar = (FontVar*) ed->GetState("FontVar");
	ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");
        PSFont* font = (fontVar == nil) ? psstdfont : fontVar->GetFont();
	PSColor* fg = (colVar == nil) ? psblack : colVar->GetFgColor();
        int lineHt = font->GetLineHt();

        Painter* painter = new Painter;
        painter->FillBg(false);
        painter->SetFont(font);
	painter->SetColors(fg, nil);
	Orientation o = v->GetOrientation();
	if (o!=Rotated) 
	  painter->SetTransformer(rel);
	else {
	  rel = new Transformer(rel);
	  rel->Rotate(90.0);
	  painter->SetTransformer(rel);
	  Unref(rel);
	}

        m = new TextManip(v, painter, lineHt, tabWidth, tool);

    } else if (tool->IsA(RESHAPE_TOOL)) {
        TextGraphic* textgr = (TextGraphic*) GetGraphic();
        Painter* painter = new Painter;
        int lineHt = textgr->GetLineHeight();
        Coord xpos, ypos;
        rel = new Transformer;
        const char* text = textgr->GetOriginal();
        int size = strlen(text);
        
        textgr->TotalTransformation(*rel);
        rel->Transform(0, 0, xpos, ypos);
        painter->FillBg(false);
        painter->SetFont(textgr->GetFont());
	painter->SetColors(textgr->GetFgColor(), nil);
        painter->SetTransformer(rel);
        Unref(rel);

        m = new TextManip(
            v, text, size, xpos, ypos, painter, lineHt, tabWidth, tool
        );

    } else {
        m = OverlayView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}

Command* TextOvView::InterpretManipulator (Manipulator* m) {
    Viewer* v = m->GetViewer();
    Editor* ed = v->GetEditor();
    Tool* tool = m->GetTool();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL) || tool->IsA(RESHAPE_TOOL)) {
        TextManip* tm = (TextManip*) m;
        int size;
        const char* text = tm->GetText(size);

        if (size == 0) {
            if (tool->IsA(RESHAPE_TOOL)) {
                cmd = new OvDeleteCmd(ed);
            } else {
                v->Update();          // to repair text display-incurred damage
            }

        } else {
            Coord xpos, ypos;
            tm->GetPosition(xpos, ypos);
            Painter* p = tm->GetPainter();
            Transformer* rel = tm->GetPainter()->GetTransformer();
            int lineHt = tm->GetLineHeight();

            Graphic* pg = GetGraphicComp()->GetGraphic();
            TextGraphic* textgr = new TextGraphic(text, lineHt, pg);

            if (tool->IsA(GRAPHIC_COMP_TOOL)) {
                textgr->SetTransformer(nil);
            }

            if (rel != nil) {
		if (v->GetOrientation()==Rotated && !tool->IsA(RESHAPE_TOOL)) 
		  rel->Rotate(-90);
                rel->InvTransform(xpos, ypos);
            }
	    if (v->GetOrientation()==Rotated && !tool->IsA(RESHAPE_TOOL))
	      textgr->Rotate(90.0);
            textgr->Translate(xpos, ypos);
            textgr->FillBg(false);
            textgr->SetFont((PSFont*) p->GetFont());
            textgr->SetColors((PSColor*) p->GetFgColor(), nil);

            if (tool->IsA(GRAPHIC_COMP_TOOL)) {
                cmd = new PasteCmd(ed, new Clipboard(new TextOvComp(textgr)));
            } else {
                cmd = new ReplaceCmd(ed, new TextOvComp(textgr));
            }
        }

    } else {
        cmd = OverlayView::InterpretManipulator(m);
    }

    return cmd;
}

Graphic* TextOvView::GetGraphic () {
    TextOvComp* ovtextcomp;
    Graphic* graphic = OverlayView::GetGraphic();

    if (graphic == nil) {
        ovtextcomp = GetTextOvComp();
        graphic = ovtextcomp->GetGraphic()->Copy();
        SetGraphic(graphic);
    }
    return graphic;
}

/****************************************************************************/

ClassId TextPS::GetClassId () { return TEXT_PS; }

boolean TextPS::IsA (ClassId id) { 
    return TEXT_PS == id || OverlayPS::IsA(id);
}

TextPS::TextPS (OverlayComp* subj) : OverlayPS(subj) { }

boolean TextPS::Definition (ostream& out) {
    TextGraphic* g = (TextGraphic*) GetGraphicComp()->GetGraphic();
    const char* text = g->GetOriginal();
    int count = strlen(text);

    out << "Begin " << MARK << " Text\n";

    float sep = g->GetLineHeight() - 1;         // correct for vert shift
    Transformer corrected, *t = g->GetTransformer();
    corrected.Translate(0., sep);

    if (t == nil) {
        g->SetTransformer(&corrected);
        TextGS(out);
        g->SetTransformer(t);

    } else {
        t->Reference();
        corrected.Postmultiply(t);
        g->SetTransformer(&corrected);
        TextGS(out);
        g->SetTransformer(t);
        Unref(t);
    }

    out << MARK << "\n";
    out << "[\n";

    int beg, end, lineSize, nextBeg, ypos = 0;

    for (beg = 0; beg < count; beg = nextBeg) {
        GetLine(text, count, beg, end, lineSize, nextBeg);
        const char* string = Filter(&text[beg], end - beg + 1);
        out << "(" << string << ")\n";
    }

    out << "] Text\n";
    out << "End\n\n";

    return out.good();
}

// Filter escapes embedded special characters that would cause syntax
// errors in a Postscript string.

const char* TextPS::Filter (const char* string, int len) {
    TextBuffer stext(sbuf, 0, SBUFSIZE);
    int dot;
    for (dot = 0; len--; string++) {
	char c = *string;

	if (!isascii(c) || iscntrl(c)) {
	    char buf[5];
	    ParamList::octal(c, &buf[sizeof(buf) - 1]);
	    dot += stext.Insert(dot, buf, sizeof(buf) - 1);

	} else {
	    switch (c) {
	    case '(':
	    case ')':
	    case '\\':
		dot += stext.Insert(dot, "\\", 1);
		// fall through
	    default:
		dot += stext.Insert(dot, string, 1);
	    }
	}
    }
    stext.Insert(dot, "", 1);

    return stext.Text();
}

/****************************************************************************/

ClassId TextScript::GetClassId () { return TEXT_SCRIPT; }

boolean TextScript::IsA (ClassId id) { 
    return TEXT_SCRIPT == id || OverlayScript::IsA(id);
}

TextScript::TextScript (TextOvComp* subj) : OverlayScript(subj) { }

boolean TextScript::Definition (ostream& out) {
    TextOvComp* comp = (TextOvComp*) GetSubject();
    TextGraphic* g = comp->GetText();
    const char* text = g->GetOriginal();

    int h = g->GetLineHeight();
    out << "text(";
    out << h << ",";
    int indent_level = 0;
    Component* parent = comp;
    do {
	parent = parent->GetParent();
	indent_level++;
    } while (parent != nil);
    ParamList::output_text(out, text, indent_level);

    float sep = g->GetLineHeight() - 1;         // correct for vert shift
    Transformer corrected, *t = g->GetTransformer();
    corrected.Translate(0., sep);
    if (t == nil) {
        g->SetTransformer(&corrected);
        TextGS(out);
        g->SetTransformer(t);

    } else {
        t->Reference();
        corrected.Postmultiply(t);
        g->SetTransformer(&corrected);
        TextGS(out);
        g->SetTransformer(t);
        Unref(t);
    }
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}
 
int TextScript::ReadText (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int line_height;
    char delim;
    char buf[BUFSIZ];

    in >> line_height >> delim;
    if (in.good())
	ParamList::parse_text(in, buf, BUFSIZ);    

    if (!in.good()) {
	return -1;
    }
    else {
    	TextGraphic* tg = new TextGraphic(buf, line_height); 	
	tg->FillBg(false);
        *(TextGraphic**)addr1 = tg; 
        return 0;
    }
}
