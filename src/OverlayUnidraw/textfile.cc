/*
 * Copyright (c) 1995,1999 Vectaport Inc.
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
 * Overlay Text from File component definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/textfile.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/Components/text.h>
#include <Unidraw/Tools/tool.h>

#include <Unidraw/manips.h>

#include <InterViews/transformer.h>

#include <Attribute/attrlist.h>

#include <iostream.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

ParamList* TextFileComp::_textfile_params = nil;
int TextFileComp::_symid = -1;

/*****************************************************************************/

ClassId TextFileComp::GetClassId () { return TEXTFILE_COMP; }

boolean TextFileComp::IsA (ClassId id) {
    return TEXTFILE_COMP == id || TextOvComp::IsA(id);
}

Component* TextFileComp::Copy () {
    TextFileComp* comp = 
      new TextFileComp(_pathname, _begstr, _endstr, _linewidth, GetGraphic());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

TextFileComp::TextFileComp (const char* pathname, const char* begstr, 
			    const char* endstr, int linewidth, Graphic *gs,
			    OverlayComp* parent) 
: TextOvComp(nil, parent) { 
    _pathname = strdup(pathname);
    _begstr = begstr ? strdup(begstr) : nil;
    _endstr = endstr ? strdup(endstr) : nil;
    _linewidth = linewidth;
    _gr = new TextGraphic("", gs->GetFont()->GetLineHt(), gs);
    _gr->FillBg(false);
    Init();
}

TextFileComp::TextFileComp(istream& in, OverlayComp* parent) 
: TextOvComp(nil, parent) {
    _pathname = _begstr = _endstr = nil;
    _linewidth = -1;
    _valid = GetParamList()->read_args(in, this); 
    Init();
}

TextFileComp::~TextFileComp() {
    delete _pathname;
    delete _begstr;
    delete _endstr;
}

void TextFileComp::Init() {

    /* read in the text from begstr to endstr */
    FILE* fptr = fopen(_pathname, "r");
    char *buffer;

    if (_linewidth == 0) {
	buffer = new char[1];
	buffer[0] = '\0';
	fclose (fptr);
	return;
    }
    if (!fptr) {
	buffer = new char[1];
	buffer[0] = '\0';
    } else {
	char inbuf[BUFSIZ];
        int nsub;
	buffer = new char[BUFSIZ];
	
	int bufsiz = BUFSIZ;
	int buflen = 0;

	fgets( inbuf, BUFSIZ, fptr);
	if (_begstr) 
	    while (!feof(fptr) && strncmp(_begstr, inbuf, strlen(_begstr)) != 0) 
		fgets( inbuf, BUFSIZ, fptr);

	int len;
	int nc = 0;
        int wordc = 0;
	char* wordbuf;
	char c;

	/* loop until eof or endstr is found */
	while (!feof(fptr) && (_endstr ? strncmp(_endstr, inbuf, strlen(_endstr)) != 0 : true)) {
	    len = strlen(inbuf);
            if (_linewidth > -1) 
	       nsub = len / _linewidth;
	    else 
	       nsub = 0;

	    /* double buffer length if needed */
	    if (buflen+len+nsub >= bufsiz) {
		bufsiz *= 2;
		char* newbuffer = new char[bufsiz];
		strcpy( newbuffer, buffer );
		delete buffer;
		buffer = newbuffer;
	    }

	    if (_linewidth > -1) {
		wordbuf = new char[len+nsub+1];

	        for (int i = 0; i < len; i++) {
	            c = inbuf[i];
		    ++nc;
		    if (c == ' ' || c == '\t' || c == '\n') {
			if (nc > _linewidth+1) {
			    strcpy(buffer+buflen, "\n");
	       		    ++buflen;
			    if (c == '\n' && nc > 1 ) {
			        wordbuf[wordc] = ' ';
			    } else {
			        wordbuf[wordc] = c;
                            }
			    wordbuf[wordc+1] = '\0';
			    nc = strlen(wordbuf);
			    wordc = 0;
			    strcpy(buffer+buflen, wordbuf);
	       		    buflen += strlen(wordbuf);
			
			} else {
			    if (c == '\n' && nc > 1 && i > 0) {
			        wordbuf[wordc] = ' ';
			        wordbuf[wordc+1] = '\0';
			    } else if (c == '\n' && i == 0) {
			        wordbuf[wordc] = c;
			        wordbuf[wordc+1] = c;
			        wordbuf[wordc+2] = '\0';
				nc = 0;
			    } else {
			        wordbuf[wordc] = c;
			        wordbuf[wordc+1] = '\0';
                            }
			    wordc = 0;
			    if (buffer[buflen-1] != ' ' || wordbuf[0] != ' ') {
				strcpy(buffer+buflen, wordbuf);
				buflen += strlen(wordbuf); 
			    } else {
				strcpy(buffer+buflen, wordbuf+1);
				buflen += strlen(wordbuf) - 1; 
			    }
			}
		    
		    } else {
			if (c=='\\') {
			    c = inbuf[++i];
			    if (isdigit(c)) {
				char buf[4];
				buf[0] = c; 
				buf[1] = buf[2] = buf[3] = '\0';
				if (isdigit(inbuf[i+1])) {
				    buf[1] = inbuf[++i];
				    if (isdigit(inbuf[i+1])) {
					buf[2] = inbuf[++i];
				    }
				}
				c = ParamList::octal(buf);
			    }
			} 
			wordbuf[wordc] = c;
			++wordc;
		    }
	        }
	        delete wordbuf;
	    
	    } else {
                strcpy(buffer+buflen, inbuf);
	        buflen += strlen(inbuf);
            }
	    fgets( inbuf, BUFSIZ, fptr);
	}
	/* done looping until eof or endstr is found */
    }

    fclose(fptr);

    /* setup the graphic */
    ((TextGraphic*)_gr)->SetOriginal(buffer);
    delete buffer;

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
			  
ParamList* TextFileComp::GetParamList() {
    if (!_textfile_params) 
	GrowParamList(_textfile_params = new ParamList());
    return _textfile_params;
}

void TextFileComp::GrowParamList(ParamList* pl) {
    pl->add_param("textfile", ParamStruct::required, 
		  &TextFileScript::ReadTextFile, this, this);
    pl->add_param("begstr", ParamStruct::keyword, 
		  &ParamList::read_string, this, &_begstr);
    pl->add_param("endstr", ParamStruct::keyword, 
		  &ParamList::read_string, this, &_endstr);
    pl->add_param("linewidth", ParamStruct::keyword, 
		  &ParamList::read_int, this, &_linewidth);
    OverlayComp::GrowParamList(pl);
    return;
}

boolean TextFileComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    return
	strcmp(GetPathname(), ((TextFileComp&)comp).GetPathname()) &&
	strcmp(GetBegstr(), ((TextFileComp&)comp).GetBegstr()) &&
	strcmp(GetEndstr(), ((TextFileComp&)comp).GetEndstr()) &&
	GetLineWidth() == ((TextFileComp&)comp).GetLineWidth() && 
	OverlayComp::operator==(comp);
}

/****************************************************************************/

TextFileComp* TextFileView::GetTextFileComp () { return (TextFileComp*) GetSubject(); }
ClassId TextFileView::GetClassId () { return TEXTFILE_VIEW; }

boolean TextFileView::IsA (ClassId id) {
    return TEXTFILE_VIEW == id || TextOvView::IsA(id);
}

TextFileView::TextFileView (TextFileComp* subj) : TextOvView(subj) { }

Manipulator* TextFileView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Manipulator* m = nil;
    Editor* ed = v->GetEditor();

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
	// do nothing
    } else if (tool->IsA(RESHAPE_TOOL)) {
	// do nothing
    } else {
        m = TextOvView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}

Command* TextFileView::InterpretManipulator (Manipulator* m) {
    Viewer* v = m->GetViewer();
    Editor* ed = v->GetEditor();
    Tool* tool = m->GetTool();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL) || tool->IsA(RESHAPE_TOOL)) {
	// do nothing
    } else {
        cmd = TextOvView::InterpretManipulator(m);
    }

    return cmd;
}

/****************************************************************************/

ClassId TextFileScript::GetClassId () { return TEXTFILE_SCRIPT; }

boolean TextFileScript::IsA (ClassId id) { 
    return TEXTFILE_SCRIPT == id || TextScript::IsA(id);
}

TextFileScript::TextFileScript (TextFileComp* subj) : TextScript(subj) { }

boolean TextFileScript::Definition (ostream& out) {
    TextFileComp* comp = (TextFileComp*) GetSubject();
    TextGraphic* g = comp->GetText();

    int h = g->GetLineHeight();
    out << "textfile(";
    out << h << ",\"" << comp->GetPathname() << "\"";
    if (comp->GetBegstr()) {
	out << " :begstr ";
	ParamList::output_text(out, comp->GetBegstr(), 0);
    }
    if (comp->GetEndstr()) {
	out << " :endstr ";
	ParamList::output_text(out, comp->GetEndstr(), 0);
    }
    if (comp->GetLineWidth() > -1)
        out << " :linewidth " << comp->GetLineWidth();
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
 
int TextFileScript::ReadTextFile (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int line_height;
    char delim;
    char pathname[BUFSIZ];

    TextFileComp* textfilecomp = (TextFileComp*)addr1;

    in >> line_height;
    ParamList::skip_space(in); 
    in >> delim; 
    if (delim == ',' && in.good()) {
	ParamList::skip_space(in);
	if (ParamList::parse_pathname(in, pathname, BUFSIZ, textfilecomp->GetBaseDir()) != 0) 
	    return -1;
    }

    if (!in.good()) {
	return -1;
    }
    else {
	textfilecomp->_pathname = strdup(pathname);
    	TextGraphic* tg = new TextGraphic("", line_height); 	
	tg->SetFont(psstdfont);
	tg->SetColors(psblack, nil);
	tg->FillBg(false);
	textfilecomp->SetGraphic(tg);
        return 0;
    }
}
