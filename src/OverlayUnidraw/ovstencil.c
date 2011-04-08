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
 * Overlay StencilComp definitions.
 */

#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovstencil.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/unidraw.h>

#include <Unidraw/Graphic/ustencil.h>

#include <InterViews/bitmap.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <Attribute/attrlist.h>

#include <stdio.h>
#include <stream.h>
#include <string.h>

/*****************************************************************************/

ParamList* StencilOvComp::_ovstencil_params = nil;
int StencilOvComp::_symid = -1;

static const int no_mask = 0;
static const int mask_equals_image = 1;
static const int valid_mask = 2;

/*****************************************************************************/


ClassId StencilOvComp::GetClassId () { return OVSTENCIL_COMP; }

boolean StencilOvComp::IsA (ClassId id) {
    return OVSTENCIL_COMP == id || OverlayComp::IsA(id);
}

Component* StencilOvComp::Copy () {
    StencilOvComp* comp = 
      new StencilOvComp((UStencil*) GetGraphic()->Copy(), _pathname);
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

StencilOvComp::StencilOvComp (UStencil* s, const char* pathname, OverlayComp* parent ) 
: OverlayComp(s, parent) {
    _pathname = (pathname == nil) ? nil : strdup(pathname);
    _by_pathname = pathname ? true : false;
}

StencilOvComp::StencilOvComp(istream& in, OverlayComp* parent) 
: OverlayComp(nil, parent) {
    _pathname = nil;
    _by_pathname = false;
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* StencilOvComp::GetParamList() {
    if (!_ovstencil_params) 
	GrowParamList(_ovstencil_params = new ParamList());
    return _ovstencil_params;
}

void StencilOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("by pathname", ParamStruct::optional, &StencilScript::ReadStencil,
		  this, this);
    pl->add_param("imagebm", ParamStruct::keyword, &StencilScript::ReadImageBitmap,
		  this, this);
    pl->add_param("maskbm", ParamStruct::keyword, &StencilScript::ReadMaskBitmap,
		  this, this);
    OverlayComp::GrowParamList(pl);
    return;
}

StencilOvComp::~StencilOvComp () { delete _pathname; }
UStencil* StencilOvComp::GetStencil () { return (UStencil*) GetGraphic(); }

void StencilOvComp::SetPathName(const char* path) {
  delete _pathname;
  if (path) 
    _pathname = strdup(path);
}
  
const char* StencilOvComp::GetPathName () { return _pathname; }

boolean StencilOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    return
	strcmp(GetPathName(), ((StencilOvComp&)comp).GetPathName()) &&
	OverlayComp::operator==(comp);
}

/*****************************************************************************/

StencilOvComp* StencilOvView::GetStencilOvComp () {
    return (StencilOvComp*) GetSubject();
}

ClassId StencilOvView::GetClassId () { return OVSTENCIL_VIEW; }

boolean StencilOvView::IsA (ClassId id) {
    return OVSTENCIL_VIEW == id || OverlayView::IsA(id);
}

StencilOvView::StencilOvView (StencilOvComp* subj) : OverlayView(subj) { }

void StencilOvView::Update () {
    Graphic* stencil = GetGraphic();

    IncurDamage(stencil);
    *stencil = *GetStencilOvComp()->GetGraphic();
    IncurDamage(stencil);
    EraseHandles();
}

Graphic* StencilOvView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();
    
    if (graphic == nil) {
        StencilOvComp* stencilComp = GetStencilOvComp();
        Bitmap* image, *mask;
        stencilComp->GetStencil()->GetOriginal(image, mask);
        graphic = new UStencil(image, mask, stencilComp->GetGraphic());
        SetGraphic(graphic);
    }
    return graphic;
}

/*****************************************************************************/

StencilPS::StencilPS (OverlayComp* subj) : OverlayPS(subj) { }
ClassId StencilPS::GetClassId () { return STENCIL_PS; }

boolean StencilPS::IsA (ClassId id) { 
    return STENCIL_PS == id || OverlayPS::IsA(id);
}

boolean StencilPS::Definition (ostream& out) {
    UStencil* stencil = (UStencil*) GetGraphicComp()->GetGraphic();
    Bitmap* image, *mask;
    stencil->GetOriginal(image, mask);
    const char* tag = (image == mask) ? "SSten" : "FSten";
    Coord w = image->Width();
    Coord h = image->Height();

    out << "Begin " << MARK << " " << tag << "\n";
    FgColor(out);
    BgColor(out);
    Transformation(out);

    out << MARK << "\n";
    out << w << " " << h << " " << tag << " ";
    out << "{ currentfile "<< (w + 7) / 8 << " string readhexstring pop }\n";
    out << "imagemask";

    unidraw->GetCatalog()->WriteBitmapData(image, out);

    out << "\nEnd\n\n";

    return out.good();
}

/*****************************************************************************/

StencilScript::StencilScript (StencilOvComp* subj) : OverlayScript(subj) { }
ClassId StencilScript::GetClassId () { return STENCIL_SCRIPT; }

boolean StencilScript::IsA (ClassId id) { 
    return STENCIL_SCRIPT == id || OverlayScript::IsA(id);
}

boolean StencilScript::Definition (ostream& out) {
    StencilOvComp* comp = (StencilOvComp*) GetSubject();
    UStencil* stencil = comp->GetStencil();

    out << "stencil(";

    if (GetByPathnameFlag())
      out << "\"" << comp->GetPathName() << "\"";
    else {
      Bitmap* image_bitmap;
      Bitmap* mask_bitmap;
      stencil->GetOriginal(image_bitmap, mask_bitmap);
      Coord w = (int)image_bitmap->width();
      Coord h = (int)image_bitmap->height();
      if (w>0 && h>0) {
	out << " :imagebm " << w << "," << h << ",\n";
	for (int y=0; y < h; y++) {
	  out << "\"";
	  for (int x= 0; x < w; x++) {
	    boolean val = image_bitmap->peek(x, y);
	    out << (val ? "1" : "0");
	  }
	  if (y+1<h)
	    out << "\",\n";
	  else
	    out << "\"\n";
	}
      }
      

      w = (int)mask_bitmap->width();
      h = (int)mask_bitmap->height();
      if (mask_bitmap!=image_bitmap && w>0 && h>0) {
	out << " :maskbm " << w << "," << h << ",\n";
	for (int y=0; y < h; y++) {
	  out << "\"";
	  for (int x= 0; x < w; x++) {
	    boolean val = image_bitmap->peek(x, y);
	    out << (val ? "1" : "0");
	  }
	  if (y+1<h)
	    out << "\",\n";
	  else
	    out << "\"\n";
	}
      }
    }
    
    StencilGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}    

int StencilScript::ReadStencil (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    Coord w, h;
    char pathname[BUFSIZ];
    char delim;

    StencilOvComp* comp = (StencilOvComp*)addr1;
    ParamList::parse_pathname(in, pathname, BUFSIZ, comp->GetBaseDir());

    if (!in.good()) 
	return -1;

#if 1
    boolean urlflag = ParamList::urltest(pathname);
    boolean already_ref = false;
    
    const char* creator = !urlflag ? OvImportCmd::ReadCreator(pathname) : nil;
    if (!creator && !urlflag) {
      cerr << "Error in reading creator for raster: " << pathname << "\n";
      return -1;
    }
#else
    const char* creator = OvImportCmd::ReadCreator(pathname);
    if (!creator) return -1;
#endif
    Bitmap* bitmap = nil;
    if (!urlflag && strcmp(creator, "X11") == 0) 
        bitmap = OvImportCmd::XBitmap_Bitmap(pathname);
    
    else if (!urlflag && strcmp(creator, "PBM") == 0) 
        bitmap = OvImportCmd::PBM_Bitmap(pathname);
    
    else if (urlflag || 
	     strcmp(creator, "JPEG") == 0 || 
	     strcmp(creator, "GIF")==0) {
        OvImportCmd importcmd((Editor*)nil);
	OverlayComp* tempcomp = (OverlayComp*)importcmd.Import(pathname);
	if (tempcomp && tempcomp->IsA(OVSTENCIL_COMP)) {
	  UStencil* ustencil = 
	    ((StencilOvComp*)tempcomp)->GetStencil();
	  Bitmap* mask = nil;
	  if(ustencil) ustencil->GetOriginal(bitmap, mask);
	  if (bitmap) bitmap->ref(); // to protect from deletion
   	  already_ref = true;		      
	  delete tempcomp;
	  
	}
    }
    
    if (bitmap != nil) {
        if(!already_ref) bitmap->ref();
        bitmap->flush();
        comp->_gr = new UStencil(bitmap, bitmap, stdgraphic);
	comp->_pathname = strdup(pathname);
	return 0;
    } else {
      cerr << "Unable to access stencil file:  " << pathname << "\n";
      return -1;
    }
}

boolean StencilScript::GetByPathnameFlag() {
    StencilOvComp* comp = (StencilOvComp*) GetSubject();
    return comp->GetByPathnameFlag() && ((OverlayScript*)GetParent())->GetByPathnameFlag();
}

Bitmap* StencilScript::read_bitmap(istream& in) {
  int w, h;
  char delim;
  in >> w >> delim >> h >> delim;
  void* nilpointer = nil;
  Bitmap* bitmap = new Bitmap(nilpointer, w, h);

  for (int y=0; y < h; y++) {
    ParamList::skip_space(in);
    char ch;
    in >> ch;
    if (ch == '"') {
      for (int x=0; x < w; x++) {
	in >> ch;
	if (ch != '"')
	  bitmap->poke(ch=='1' ? 1 : 0, x, y);
	else
	  break;
      }
      if (ch != '"') 
	in >> ch;
      in >> ch;
      if (ch != ',') {
	in.putback(ch);
	break;
      }
    }
  }

  if (bitmap) bitmap->flush();
  return bitmap;
  
}

int StencilScript::ReadImageBitmap (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    StencilOvComp* comp = (StencilOvComp*)addr1;
    char delim;
    Bitmap* bitmap = read_bitmap(in);

    if (in.good() && bitmap) {
        bitmap->ref();
	if (comp->_gr) {
	  UStencil* stencil = (UStencil*)comp->_gr;
	  Bitmap *i, *m;
	  stencil->GetOriginal(i, m);
	  Unref(i);
	  Unref(m);
	  if (i==m) 
	    stencil->SetOriginal(bitmap, bitmap);
	  else
	    stencil->SetOriginal(bitmap, m);
	} else 
	  comp->_gr = new UStencil(bitmap, bitmap);
	return 0;
    } else {
        delete bitmap;
	cerr << "Unable to create bitmap from file." << "\n";
	return -1;
    }
}

int StencilScript::ReadMaskBitmap (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    StencilOvComp* comp = (StencilOvComp*)addr1;
    char delim;
    Bitmap* bitmap = read_bitmap(in);

    if (in.good() && bitmap) {
        bitmap->ref();
	if (comp->_gr) {
	  UStencil* stencil = (UStencil*)comp->_gr;
	  Bitmap *i, *m;
	  stencil->GetOriginal(i, m);
	  Unref(i);
	  Unref(m);
	  stencil->SetOriginal(i, bitmap);
	} else 
	  comp->_gr = new UStencil(nil, bitmap);
	return 0;
    } else {
        delete bitmap;
	cerr << "Unable to create bitmap from file." << "\n";
	return -1;
    }
}

