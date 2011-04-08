/*
 * Copyright (c) 1999 Vectaport Inc.
 * Copyright (c) 1997 R.B. Kissh & Associates, Vectaport Inc.
 * Copyright (c) 1994-1996 Vectaport Inc.
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
 * RasterOvComp definitions.
 */
// #define RASTER_DAMAGE1 // define for incremental flushing of raster
// #define RASTER_DAMAGE2 // define to use mbr of incrementally loaded region for raster damage.

#include <OS/math.h>
#include <math.h>

#include <OverlayUnidraw/grayraster.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovexport.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovpainter.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/paramlist.h>
#include <OverlayUnidraw/ovipcmds.h>
#include <OverlayUnidraw/ovviewer.h>

#include <Unidraw/editor.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/globals.h>

#include <Unidraw/Graphic/damage.h>

#include <Unidraw/Graphic/rasterrect.h>
#include <IVGlyph/gdialogs.h>

#include <InterViews/session.h>
#include <InterViews/tiff.h>
#include <InterViews/transformer.h>
#include <InterViews/window.h>
#include <InterViews/style.h>

#include <IV-2_6/_enter.h>

#include <IV-X11/xdisplay.h>
#include <IV-X11/xraster.h>
#include <IV-X11/xcolor.h>
#include <OS/math.h>
#include <OS/memory.h>

#include <Attribute/attrlist.h>

#include <stdio.h>
#include <stream.h>
#include <string.h>

implementList(CopyStringList,CopyString)

boolean RasterOvComp::_use_gray_raster = false;
boolean RasterOvComp::_warned = false;

/*****************************************************************************/

ParamList* RasterOvComp::_ovraster_params = nil;

static const int color_depth = 8;               // bits per color in PostScript

static char hexcharmap[] = {
     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
     'a', 'b', 'c', 'd', 'e', 'f'
};

/*****************************************************************************/

int RasterOvComp::_symid = -1;

static ostream& operator<<(ostream& out, const CopyStringList& sl) {
    for (ListItr(CopyStringList) i(sl); i.more(); i.next()) {
        out << i.cur_ref().string() << "\n";
    }
    out << "\n";
    return out;
}


ClassId RasterOvComp::GetClassId () { return OVRASTER_COMP; }

boolean RasterOvComp::IsA (ClassId id) {
    return OVRASTER_COMP == id || OverlayComp::IsA(id);
}


Component* RasterOvComp::Copy () {
    RasterOvComp* nc = new RasterOvComp(
        (OverlayRasterRect*) GetGraphic()->Copy(), _pathname, (OverlayComp*)GetParent()
    );
    if (attrlist()) nc->SetAttributeList(new AttributeList(attrlist()));

    for (ListItr(CopyStringList) i(_commands); i.more(); i.next()) {
        nc->_commands.append(i.cur_ref());
    }
    nc->_com_exp = _com_exp;

    return nc;
}

RasterOvComp::RasterOvComp (OverlayRasterRect* s, const char* pathname, 
OverlayComp* parent) : OverlayComp(s, parent), _com_exp("") {
    _pathname = (pathname == nil) ? nil : strdup(pathname);
    if(pathname) _import_flags |= bypath_mask;
}

RasterOvComp::RasterOvComp(istream& in, OverlayComp* parent) 
: OverlayComp(nil, parent), _com_exp("") {
    _pathname = nil;
    _import_flags = 0x0;
    _valid = GetParamList()->read_args(in, this);

    // update the size of the raster 
    OverlayRasterRect* orr;
    if (orr=GetOverlayRasterRect()) {
        OverlayRaster* r;
        if (r=orr->GetOverlayRaster())
            if (orr->xbeg() != -1)
                r->init_rep(
                    orr->xend() - orr->xbeg() + 1,
                    orr->yend() - orr->ybeg() + 1
                );
    }
}

ParamList* RasterOvComp::GetParamList() {
    if (!_ovraster_params) 
	GrowParamList(_ovraster_params = new ParamList());
    return _ovraster_params;
}

void RasterOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("pathname", ParamStruct::optional, &RasterScript::ReadRaster,
		  this, this);
    pl->add_param("rgb", ParamStruct::keyword, &RasterScript::ReadRGB,
		  this, this);
    pl->add_param("gray", ParamStruct::keyword, &RasterScript::ReadGrayUChar,
		  this, this);
    pl->add_param("graychar", ParamStruct::keyword, &RasterScript::ReadGrayChar,
		  this, this);
    pl->add_param("grayuchar", ParamStruct::keyword, &RasterScript::ReadGrayUChar,
		  this, this);
    pl->add_param("grayint", ParamStruct::keyword, &RasterScript::ReadGrayInt,
		  this, this);
    pl->add_param("grayuint", ParamStruct::keyword, &RasterScript::ReadGrayUInt,
		  this, this);
    pl->add_param("graylong", ParamStruct::keyword, &RasterScript::ReadGrayLong,
		  this, this);
    pl->add_param("grayulong", ParamStruct::keyword, &RasterScript::ReadGrayULong,
		  this, this);
    pl->add_param("grayfloat", ParamStruct::keyword, &RasterScript::ReadGrayFloat,
		  this, this);
    pl->add_param("graydouble", ParamStruct::keyword, &RasterScript::ReadGrayDouble,
		  this, this);

    pl->add_param("proc", ParamStruct::keyword, &RasterScript::ReadProcess,
		  this, this);

    OverlayRasterRect* orr = new OverlayRasterRect();
    _gr = orr;
    pl->add_param_indirect("sub", ParamStruct::optional, &ParamList::read_int,
		  this, &_gr, &orr->_xbeg, &orr->_xend, &orr->_ybeg, &orr->_yend);
    delete orr;
    _gr = nil;

    OverlayComp::GrowParamList(pl);
    return;
}

RasterOvComp::~RasterOvComp () { 
    if (_pathname) {
	delete _pathname;
        _pathname = 0;
    }

    OvImportCmd::detach(this);  // maybe in the process of downloading 
}

OverlayRasterRect* RasterOvComp::GetOverlayRasterRect () { return (OverlayRasterRect*) GetGraphic(); }

void RasterOvComp::SetPathName(const char* path) {
  delete _pathname;
  if (path) 
    _pathname = strdup(path);
}
  
const char* RasterOvComp::GetPathName () { return _pathname; }

boolean RasterOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    OverlayRasterRect* rasta = GetOverlayRasterRect();
    OverlayRasterRect* rastb = ((RasterOvComp&)comp).GetOverlayRasterRect();
    return
	strcmp(GetPathName(), ((RasterOvComp&)comp).GetPathName()) &&
	rasta->xbeg() == rastb->xbeg() &&
	rasta->ybeg() == rastb->ybeg() &&
	rasta->xend() == rastb->xend() &&
	rasta->yend() == rastb->yend() &&
	OverlayComp::operator==(comp);
}


void RasterOvComp::Interpret (Command* cmd) {
    OverlayRasterRect* gr = GetOverlayRasterRect();

    if (gr == nil) {
        return;
    }

    if (cmd->IsA(IMAGE_CMD)) {
        ImageCmd* icmd = (ImageCmd*)cmd;
        _commands.append(icmd->Cmd());
#ifndef NDEBUG
        cerr << _commands;
#endif
    }
    else {
        OverlayComp::Interpret(cmd);
    }
}

void RasterOvComp::Uninterpret (Command* cmd) {
    Graphic* gr = GetGraphic();

    if (gr == nil) {
        return;
    }

    if (cmd->IsA(IMAGE_CMD)) {
        _commands.remove(_commands.count() - 1);
#ifndef NDEBUG
        cerr << _commands;
#endif
    }
    else {
        OverlayComp::Uninterpret(cmd);
    }
}

void RasterOvComp::Configure(Editor* ed) {

#ifndef NDEBUG
cerr << "Configure: " << _com_exp.string() << endl;
#endif

    if (_com_exp != "") {
        RasterTerp terp(ed);
        CopyString tmp(_com_exp);
        _com_exp = "";

        int result = terp.execute(this, tmp);

        // ignoring result ####
    }

    if (GetOverlayRasterRect()->GetOverlayRaster()->status() && !_warned) {
      _warned = true;
      GAcknowledgeDialog::post(ed->GetWindow(), "unable to allocate enough colormap entries on the X server", "quit other programs and restart", "colormap problem");
    }
}


ParamList* RasterOvComp::get_param_list() {
  if (!_ovraster_params) {
    RasterOvComp raster;
    raster.GrowParamList(_ovraster_params = new ParamList());
  }
  return _ovraster_params;
}

/*****************************************************************************/

RasterOvComp* RasterOvView::GetRasterOvComp () {
    return (RasterOvComp*) GetSubject();
}

ClassId RasterOvView::GetClassId () { return OVRASTER_VIEW; }

boolean RasterOvView::IsA (ClassId id) {
    return OVRASTER_VIEW == id || OverlayView::IsA(id);
}

RasterOvView::RasterOvView (RasterOvComp* subj) : OverlayView(subj) { }

void RasterOvView::Update () {
    OverlayRasterRect* raster = (OverlayRasterRect*)GetGraphic();
    OverlayRasterRect* subj = (OverlayRasterRect*)GetRasterOvComp()->GetGraphic();

#if defined(RASTER_DAMAGE2)
    if (!subj->damage_done())
#endif
      IncurDamage(raster);
    *raster = *subj;
#if defined(RASTER_DAMAGE2)
    if (!subj->damage_done())
#endif
      IncurDamage(raster);
    EraseHandles();
}

Graphic* RasterOvView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();
    
    if (graphic == nil) {
        OverlayRasterRect* rr = GetRasterOvComp()->GetOverlayRasterRect();

        OverlayRaster* r = rr ? rr->GetOverlayRaster() : nil;
	graphic = r ? new OverlayRasterRect(r, rr) : nil;

        SetGraphic(graphic);
    }
    return graphic;
}

OverlayRaster* RasterOvView::GetOverlayRaster () 
{
  return GetOverlayRasterRect()->GetOverlayRaster(); 
}

/*****************************************************************************/

RasterPS::RasterPS (OverlayComp* subj) : OverlayPS(subj) { }
ClassId RasterPS::GetClassId () { return RASTER_PS; }

boolean RasterPS::IsA (ClassId id) { 
    return RASTER_PS == id || OverlayPS::IsA(id);
}

boolean RasterPS::Definition (ostream& out) {
    RasterOvComp* comp = (RasterOvComp*) GetGraphicComp();
    OverlayRasterRect* rr =  (OverlayRasterRect*) comp->GetGraphic();
    OverlayRaster* raster = (OverlayRaster*)rr->GetOriginal();
    const char* pathname = comp->GetPathName();
    if (!raster->initialized()) rr->load_image(pathname);
    Coord w = raster->Width();
    Coord h = raster->Height();

    boolean idraw_format = OverlayPS::idraw_format;
    if (GetCommand() && GetCommand()->IsA(OV_EXPORT_CMD))
      idraw_format = ((OvExportCmd*)GetCommand())->idraw_format();
    else if (GetCommand() && GetCommand()->IsA(OVPRINT_CMD)) 
      idraw_format = ((OvPrintCmd*)GetCommand())->idraw_format();
    
    if (idraw_format) {
      
	out << "Begin " << MARK << " " << "Rast\n";
	Transformation(out);

	out << MARK << "\n";
	out << w << " " << h << " " << color_depth << " Rast ";
	out << "{ currentfile ";
	out << (w * color_depth + 7) / 8 << " ";
	out << "string readhexstring pop }\n";
	out << "image";
	
	IdrawCatalog* catalog = (IdrawCatalog*)unidraw->GetCatalog();
	catalog->IdrawCatalog::WriteGraymapData(raster, out);
	
	catalog->Mark(out);
	out << "colorimage";
	catalog->IdrawCatalog::WriteRasterData(raster, out);
	
	out << "\nEnd\n\n";

	return out.good();
    }

    if (comp->GetPathName() && strstr(comp->GetPathName(), ".pgm"))  {

	out << "Begin " << MARK << " " << "GrayRast\n";
	Transformation(out);

	out << MARK << "\n";
	out << w << " " << h << " " << color_depth << " Rast ";
	out << "{ currentfile ";
	out << (w * color_depth + 7) / 8 << " ";
	out << "string readhexstring pop }\n";
	out << "image";

	Catalog* catalog = unidraw->GetCatalog();
	catalog->WriteGraymapData(raster, out);

	out << "\nEnd\n\n";

    } else {

	out << "Begin " << MARK << " " << "ColorRast\n";
	Transformation(out);

	out << "\n/readstring {\n";
	out << "  currentfile exch readhexstring pop\n";
	out << "} bind def\n";
	out << "/rpicstr " << w << " string def\n";
	out << "/gpicstr " << w << " string def\n";
	out << "/bpicstr " << w << " string def\n\n";
	
	out << w << " " << h << " scale\n";
	out << w << " " << h << " 8\n";
	out << "[ " << w << " 0 0 -" << h << " 0 " << h << " ]\n";
	out << "{ rpicstr readstring }\n";
	out << "{ gpicstr readstring }\n";
	out << "{ bpicstr readstring }\n";
	out << "true 3\n";
	out << "colorimage\n";
	
	ColorIntensity r, g, b;
	float alpha;
	int count = 0;
        int i;
	for (int j = h-1; j>=0; --j) {
	    for (i=0; i<w; ++i) {
		raster->peek(i, j, r, g, b, alpha);
		int ir = (int)(r*255);
		out << hexcharmap[ir/16] << hexcharmap[ir%16];
		if (++count%40 == 0) out << "\n";
	    }
	    for (i=0; i<w; ++i) {
		raster->peek(i, j, r, g, b, alpha);
		int ig = (int)(g*255);
		out << hexcharmap[ig/16] << hexcharmap[ig%16];
		if (++count%40 == 0) out << "\n";
	    }
	    for (i=0; i<w; ++i) {
		raster->peek(i, j, r, g, b, alpha);
		int ib = (int)(b*255);
		out << hexcharmap[ib/16] << hexcharmap[ib%16];
		if (++count%40 == 0) out << "\n";
	}
	}

	out << "\nEnd\n\n";
    }

    return out.good();
}

/*****************************************************************************/

ClassId RasterScript::GetClassId () { return RASTER_SCRIPT; }

boolean RasterScript::IsA (ClassId id) { 
    return RASTER_SCRIPT == id || OverlayScript::IsA(id);
}

RasterScript::RasterScript (RasterOvComp* subj) : OverlayScript(subj) { }

boolean RasterScript::Definition (ostream& out) {
    RasterOvComp* comp = (RasterOvComp*) GetSubject();
    OverlayRasterRect* rr = comp->GetOverlayRasterRect();
    OverlayRaster* raster = (OverlayRaster*)rr->GetOriginal();

    out << (GetFromCommandFlag() && GetByPathnameFlag() && comp->GetPathName()
	    ? "ovfile(:popen " : "raster(");
    
    if (GetByPathnameFlag() && comp->GetPathName()){
      out << "\"" << comp->GetPathName() << "\"";

    } else {
      const char* pathname = comp->GetPathName();
      if (raster->grayraster()) {
	switch (raster->value_type()) {
	case AttributeValue::CharType: 
	  out << ":graychar ";
	  break;
	case AttributeValue::UCharType: 
	  out << ":grayuchar ";
	  break;
	case AttributeValue::ShortType: 
	  out << ":grayshort ";
	  break;
	case AttributeValue::UShortType: 
	  out << ":grayushort ";
	  break;
	case AttributeValue::IntType: 
	  out << ":grayint ";
	  break;
	case AttributeValue::UIntType: 
	  out << ":grayuint ";
	  break;
	case AttributeValue::LongType: 
	  out << ":graylong ";
	  break;
	case AttributeValue::ULongType: 
	  out << ":grayulong ";
	  break;
	case AttributeValue::FloatType: 
	  out << ":grayfloat ";
	  break;
	case AttributeValue::DoubleType: 
	  out << ":graydouble ";
	  break;
	default: 
	  out << ":gray ";
	  break;
	}
      } 
      else if (raster->gray_flag())
	out << ":gray ";
      else
	out << ":rgb ";
      raster->write(out);
    }

    if (rr->xbeg()>=0 || rr->xend()>=0 || rr->ybeg()>=0 || rr->yend()>=0)
        out << " :sub " <<
	    rr->xbeg() << "," <<
	    rr->xend() << "," <<
	    rr->ybeg() << "," <<
	    rr->yend();

    MinGS(out);
    Annotation(out);

    const long count = comp->_commands.count();
    if (count) {
        out << " :proc \"";
        int j = 0;
        for (ListItr(CopyStringList) i(comp->_commands); i.more(); i.next()) {
            out << i.cur_ref().string();
            if (++j != count) {
                out << "; ";
            }
        }
        out << "\"";
    }

    Attributes(out);
    out << ")";

    return out.good();
}    

int RasterScript::ReadRaster (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    char pathname[BUFSIZ];

    RasterOvComp* comp = (RasterOvComp*)addr1;
    ParamList::parse_pathname(in, pathname, BUFSIZ, comp->GetBaseDir());

    if (!in.good()) {
	cerr << "Error in reading pathname for raster\n";
	return -1;
    }

    boolean urlflag = ParamList::urltest(pathname);

    const char* creator = !urlflag ? OvImportCmd::ReadCreator(pathname) : nil;
    if (!creator && !urlflag) {
	cerr << "Error in reading creator for raster: " << pathname << "\n";
	return -1;
    }
    OverlayRaster* ovraster = nil;
    const boolean delayed = true;
    boolean already_ref = false;
    if (!urlflag && strcmp(creator, "TIFF") == 0) {
	ovraster = OvImportCmd::TIFF_Raster(pathname);
	
    } else if (!urlflag && strcmp(creator, "PGM") == 0) {
	ovraster = OvImportCmd::PGM_Raster(pathname, delayed);
	
    } else if (!urlflag && strcmp(creator, "PPM") == 0) {
	ovraster = OvImportCmd::PPM_Raster(pathname, delayed);
    } else if (!urlflag && 
	       (strcmp(creator, "JPEG") == 0 || strcmp(creator, "GIF")==0 ||
		strcmp(creator, "PNG")==0)) {
      OvImportCmd importcmd((Editor*)nil);
      OverlayComp* tempcomp = (OverlayComp*)importcmd.Import(pathname);
      if (tempcomp && tempcomp->IsA(OVRASTER_COMP)) {
	OverlayRasterRect* ovrrect = 
	  ((RasterOvComp*)tempcomp)->GetOverlayRasterRect();
	ovraster = ovrrect ? ovrrect->GetOverlayRaster() : nil;
	if (ovraster) ovraster->ref(); // to protect from deletion
	already_ref = true;		      
	delete tempcomp;
      }
    } else if (urlflag) {
      ovraster = OvImportCmd::CreatePlaceImage();
      ovraster->initialized(false);
    } 

    if (ovraster) {
      comp->_gr = new OverlayRasterRect(ovraster);
      if (already_ref) {
          ovraster->unref();
      }
      comp->_pathname = strdup(pathname);
      comp->SetByPathnameFlag(true);
      return 0;
    } else {
      cerr << "Unable to access image path:  " << pathname << "\n";
      return -1;
    }
}

int RasterScript::ReadRGB (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    RasterOvComp* comp = (RasterOvComp*)addr1;
    int w, h; 
    int x = 0;
    int y = 0;
    int ir, ig, ib;
    char paren, delim;
    
    in >> w >> delim >> h >> delim;

    OverlayRaster* raster = new OverlayRaster(w, h);
    raster->read(in);

    if (in.good()) {
	comp->_gr = new OverlayRasterRect(raster);
	return 0;
    } else {
        delete raster;
	cerr << "Unable to create image from file." << "\n";
	return -1;
    }
}

int RasterScript::ReadGrayChar (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::CharType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create char data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayUChar (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::UCharType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create unsigned char data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayShort (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::ShortType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create short data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayUShort (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::UShortType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create unsigned short data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayInt (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::IntType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create int data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayUInt (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::UIntType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create unsigned int data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayLong (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::LongType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create long data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayULong (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::ULongType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create unsigned long data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayFloat (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::FloatType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create float data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadGrayDouble (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  RasterOvComp* comp = (RasterOvComp*)addr1;
  int w, h, x, y;
  char delim;
  
  in >> w >> delim >> h >> delim;

  GrayRaster* raster = new GrayRaster(w, h, AttributeValue::DoubleType);
  raster->read(in);
  raster->top2bottom(false);
  
  if (in.good()) {
    comp->_gr = new OverlayRasterRect(raster);
    return 0;
  } else {
    delete raster;
    cerr << "Unable to create double data raster from file." << "\n";
    return -1;
  }
}

int RasterScript::ReadSub (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    IntCoord xbeg;
    IntCoord xend;
    IntCoord ybeg;
    IntCoord yend;
    char delim;
    OverlayRasterRect* gr = *(OverlayRasterRect**)addr1;
 
    ParamList::skip_space(in);
    in >> xbeg >> delim >> xend >> delim >> ybeg >> delim >> yend;
    if (!in.good()) {
        return -1;
    }
    else {
        gr->xbeg(xbeg);
        gr->xend(xend);
        gr->ybeg(ybeg);
        gr->yend(yend);
        return 0;
    }
}


int RasterScript::ReadProcess (
    istream& in, void* addr1, void*, void*, void*
) {
    ParamList::skip_space(in);

    if (ParamList::parse_string(in,sbuf,SBUFSIZE) >= 0) { 
        RasterOvComp* comp = (RasterOvComp*)addr1;
        comp->_com_exp = sbuf;
        return 0;
    }
    else {
        return -1;
    }
}


boolean RasterScript::GetByPathnameFlag() {
    RasterOvComp* comp = (RasterOvComp*) GetSubject();
    return comp->GetByPathnameFlag() && ((OverlayScript*)GetParent())->GetByPathnameFlag();
}

boolean RasterScript::GetFromCommandFlag() {
    RasterOvComp* comp = (RasterOvComp*) GetSubject();
    return comp->GetFromCommandFlag();
}

/*****************************************************************************/

OverlayRasterRect::OverlayRasterRect(OverlayRaster* r, Graphic* gr) : RasterRect(r, gr) {
#if 0 /* now done in OverlayGraphic::new_painter() */
    Unref(_p);
    _p = new OverlayPainter();
    Resource::ref(_p);
#endif
    _xbeg = _xend = _ybeg = _yend = -1;
    _damage_done = 0;
}

OverlayRasterRect::~OverlayRasterRect () {}

Graphic* OverlayRasterRect::Copy () {
    OverlayRasterRect* new_rr;
    new_rr = new OverlayRasterRect(
	    ((OverlayRaster*)GetOriginal())->copy(), this);
    new_rr->xbeg(_xbeg);
    new_rr->xend(_xend);
    new_rr->ybeg(_ybeg);
    new_rr->yend(_yend);
    return new_rr;
}

#undef RasterRect
void OverlayRasterRect::draw (Canvas *c, Graphic* gs) {
    update(gs);
    ((OverlayPainter*)_p)->RasterRect(c, 0, 0, this);
}

const char* OverlayRasterRect::path() const {
    OverlayRasterRect* th = (OverlayRasterRect*)this; 
    return ((RasterOvView*)th->GetTag())->GetRasterOvComp()->GetPathName();
}

OverlayRaster* OverlayRasterRect::GetOverlayRaster() {
    return (OverlayRaster*)GetOriginal();
}

void OverlayRasterRect::load_image(const char* pathname) {
    if (GetOverlayRaster()->initialized()) return;

    if (!pathname) 
	pathname = ((RasterOvView*)GetTag())->GetRasterOvComp()->GetPathName();

    if (pathname) {
      if (!ParamList::urltest(pathname)) {

	/* local file */
        const char* creator = OvImportCmd::ReadCreator(pathname);
        if (strcmp(creator, "PGM") == 0)
	    OvImportCmd::PGM_Raster(
                pathname, false, (OverlayRaster*)_raster, _xbeg, _xend, _ybeg,
                _yend
            );
        else if (strcmp(creator, "PPM") == 0) 
	    OvImportCmd::PPM_Raster(
                pathname, false, (OverlayRaster*)_raster, _xbeg, _xend, _ybeg,
                _yend);
        else 
	    cerr << "unexpected image file format (" << creator << ") in " << 
            pathname << "\n"; 

      } else {

	/* file by URL */
	OvImportCmd importcmd((Editor*)nil);
	OverlayComp* tempcomp = (OverlayComp*)importcmd.Import(pathname);
	if (tempcomp && tempcomp->IsA(OVRASTER_COMP)) {

          OvImportCmd::changeComp(
            (RasterOvComp*)tempcomp,
            ((RasterOvView*)GetTag())->GetRasterOvComp()
          );

#if 0
	  OverlayRasterRect* ovrrect = 
	    ((RasterOvComp*)tempcomp)->GetOverlayRasterRect();
	  OverlayRaster* ovraster = ovrrect 
	    ? ovrrect->GetOverlayRaster() : nil;
	  if (ovraster) ovraster->ref();  // to protect from deletion
	  delete tempcomp;
	  Unref(_raster);
	  _raster = ovraster;
	  uncacheParents();
	  ((OverlayRaster*)_raster)->initialize();
#else
	  uncacheParents(); // necessary?
	  ((OverlayRaster*)_raster)->initialize();
#endif
	}
      }
    }
    if (_raster->pwidth()) 
      ((OverlayRaster*)_raster)->initialize();
}

void OverlayRasterRect::xbeg(IntCoord xbeg) { _xbeg = xbeg; }
void OverlayRasterRect::xend(IntCoord xend) { _xend = xend; }
void OverlayRasterRect::ybeg(IntCoord ybeg) { _ybeg = ybeg; }
void OverlayRasterRect::yend(IntCoord yend) { _yend = yend; }

IntCoord OverlayRasterRect::xbeg() const { return _xbeg; }
IntCoord OverlayRasterRect::xend() const { return _xend; }
IntCoord OverlayRasterRect::ybeg() const { return _ybeg; }
IntCoord OverlayRasterRect::yend() const { return _yend; }

OverlayRasterRect& OverlayRasterRect::operator = (OverlayRasterRect& rect) {
    Graphic::operator=(rect);
    _xbeg = rect.xbeg();
    _xend = rect.xend();
    _ybeg = rect.ybeg();
    _yend = rect.yend();

    if( _damage_done = rect.damage_done()) {
      _damage_l = rect._damage_l;
      _damage_b = rect._damage_b;
      _damage_r = rect._damage_r;
      _damage_t = rect._damage_t;
    }

    Unref(_raster);
    _raster = rect._raster;
    Resource::ref(_raster);

    return *this;
}

void OverlayRasterRect::damage_flush() {
  if (_raster) {
#if defined(RASTER_DAMAGE1)
    if (_damage_done) {
      _raster->flushrect(_damage_l, _damage_b, _damage_r, _damage_t);
      _damage_done = 0;
    } else
#endif
      _raster->flush();
  }
}

void OverlayRasterRect::damage_rect(IntCoord l, IntCoord b, 
				    IntCoord r, IntCoord t) 
{
  _damage_l = l;
  _damage_b = b;
  _damage_r = r;
  _damage_t = t;
  _damage_done = 1;
}

/*****************************************************************************/

// #define LEAKCHECK

#ifdef LEAKCHECK
#include <OverlayUnidraw/leakchecker.h>
static LeakChecker checker("OverlayRaster");
#endif


XColor* OverlayRaster::_gray_map = nil;
int OverlayRaster::_unique_grays = 0;
boolean OverlayRaster::_gray_initialized = false;
XColor* OverlayRaster::_color_map = nil;
int OverlayRaster::_unique_colors = 0;

OverlayRaster::OverlayRaster(unsigned long width, unsigned long height) : Raster (new RasterRep) {
    init_rep(width, height);
#ifdef LEAKCHECK
    checker.create();
#endif
    _grayflag = false;
    _init = true;
}

OverlayRaster::OverlayRaster(const OverlayRaster& raster) 
: Raster(new RasterRep) {
    construct(raster);
    _grayflag = false;
    _init = true;
}

OverlayRaster::OverlayRaster(const Raster& raster)
: Raster(new RasterRep) {
    construct(raster);
    _grayflag = false;
    _init = true;
}


OverlayRaster::OverlayRaster(
  unsigned long width, unsigned long height, unsigned long  bwidth
) 
  : Raster (new RasterRep) 
{
    init_rep(width, height);
#ifdef LEAKCHECK
    checker.create();
#endif
    _grayflag = false;
    _init = true;

    RasterRep* r = rep();
    DisplayRep* dr = r->display_->rep();
    XDisplay* dpy = dr->display_;

    r->pixmap_ = XCreatePixmap(
	dpy, dr->root_, r->pwidth_, r->pheight_, dr->default_visual_->depth()
    );
    r->gc_ = XCreateGC(dpy, r->pixmap_, 0, nil);

    const Style* s = Session::instance()->style();

    // this bg/fg code is in World, but want to avoid that class

    String v("#ffffff");
    if (!s->find_attribute("background", v)) {
        s->find_attribute("Background", v);
    }
    const Color* bc = Color::lookup(
        Session::instance()->default_display(), v
    );
    if (bc == nil) {
        bc = new Color(1.0, 1.0, 1.0, 1.0);
    }
    Resource::ref(bc);

    v = "#000000";
    if (!s->find_attribute("foreground", v)) {
        s->find_attribute("Foreground", v);
    }
    const Color* fc = Color::lookup(
        Session::instance()->default_display(), v
    );
    if (fc == nil) {
        fc = new Color(0.0, 0.0, 0.0, 1.0);
    }
    Resource::ref(fc);

    // oh brother
    String rv;
    if (s->find_attribute("reverseVideo", rv)) {
        if (rv.case_insensitive_equal("on")) {
            const Color* swap = bc;
            bc = fc;
            fc = swap;
	}
    }

    // set up GC

    GC gc = XCreateGC(dpy, r->pixmap_, 0, nil); 

    ColorRep* cr = fc->rep(dr->default_visual_);
    unsigned long fpixel = cr->xcolor_.pixel;

    cr = bc->rep(dr->default_visual_);
    unsigned long bpixel = cr->xcolor_.pixel;

    XSetForeground(dpy, gc, bpixel);
    XFillRectangle(dpy, r->pixmap_, gc, 0, 0, r->pwidth_, r->pheight_);

    // draw the border outline here

    bwidth = (bwidth % 2) ? bwidth + 1 : bwidth;

    XSetForeground(dpy, gc, fpixel);
    XSetLineAttributes(dpy, gc, bwidth, LineSolid, CapButt, JoinMiter); 
    XDrawRectangle(
        dpy, r->pixmap_, gc, bwidth/2, bwidth/2, r->pwidth_ - bwidth,
        r->pheight_ - bwidth
    );

    Resource::unref(fc);
    Resource::unref(bc);
    XFreeGC(dpy, gc);

#ifdef XSHM
    init_shared_memory();
#endif

    if (!r->shared_memory_) {
        r->image_ = XGetImage(
            dpy, r->pixmap_, 0, 0, r->pwidth_, r->pheight_, AllPlanes, ZPixmap
        );
    }
}


void OverlayRaster::construct(const Raster& raster) {
    _grayflag = false;
    RasterRep* r = rep();
    raster.flush();
    RasterRep& rr = *(raster.rep());
    r->display_ = rr.display_;
    r->modified_ = true;
    r->width_ = rr.width_;
    r->height_ = rr.height_;
    r->left_ = rr.left_;
    r->bottom_ = rr.bottom_;
    r->right_ = rr.right_;
    r->top_ = rr.top_;
    r->pwidth_ = rr.pwidth_;
    r->pheight_ = rr.pheight_;
    r->shared_memory_ = false;

    if (rr.pixmap_) {
        DisplayRep* dr = r->display_->rep();
        XDisplay* dpy = dr->display_;
        r->pixmap_ = XCreatePixmap(
	  dpy, dr->root_, r->pwidth_, r->pheight_, dr->default_visual_->depth()
        );
        r->gc_ = XCreateGC(dpy, r->pixmap_, 0, nil);

        XCopyArea(
	    dpy, rr.pixmap_, r->pixmap_, r->gc_,
	    0, 0, r->pwidth_, r->pheight_, 0, 0
	);

#ifdef XSHM
        init_shared_memory();
#endif
        if (!r->shared_memory_) {
            r->image_ = XGetImage(
                dpy, r->pixmap_, 0, 0, r->pwidth_, r->pheight_, AllPlanes, ZPixmap
            );
        }
    } else {
	r->pixmap_ = nil;
	r->gc_ = nil;
	r->image_ = nil;
    }
#ifdef LEAKCHECK
    checker.create();
#endif
}

OverlayRaster::~OverlayRaster() {
#ifdef LEAKCHECK
    checker.destroy();
#endif
    OverlayPainter::Uncache(this);
}

void OverlayRaster::init_rep(unsigned long w, unsigned long h) {
    RasterRep* r = rep();
    Display* d = Session::instance()->default_display();
    r->display_ = d;
    r->modified_ = false;
    r->pwidth_ = (unsigned int)w;
    r->pheight_ = (unsigned int)h;
    r->width_ = d->to_coord(r->pwidth_);
    r->height_ = d->to_coord(r->pheight_);
    r->left_ = 0;
    r->bottom_ = 0;
    r->right_ = r->width_;
    r->top_ = r->height_;
    r->pixmap_ = nil;
    r->gc_ = nil;
    r->image_ = nil;
    r->shared_memory_ = false;
}

void OverlayRaster::init_space() {
    RasterRep* r = rep();
    if (r->pixmap_) return;
    DisplayRep* dr = r->display_->rep();
    XDisplay* dpy = dr->display_;

    r->pixmap_ = XCreatePixmap(
	dpy, dr->root_, r->pwidth_, r->pheight_, dr->default_visual_->depth()
    );
    r->gc_ = XCreateGC(dpy, r->pixmap_, 0, nil);

#ifdef XSHM
    init_shared_memory();
#endif

    if (!r->shared_memory_) {
        r->image_ = XGetImage(
            dpy, r->pixmap_, 0, 0, r->pwidth_, r->pheight_, AllPlanes, ZPixmap
        );
    }
}


void OverlayRasterRect::SetRaster(OverlayRaster* nr) {
    Unref(_raster);
    _raster = nr;
    Resource::ref(_raster);
    _xbeg = _xend = _ybeg = _yend = -1;
}

OverlayRaster* OverlayRaster::copy() const {
    return new OverlayRaster(*this);
}


void OverlayRaster::poke(
    unsigned long x, unsigned long y,
    ColorIntensity red, ColorIntensity green, ColorIntensity blue, float alpha
) {
    RasterRep* r = rep();
    if (!r->pixmap_) init_space();
    Raster::poke(x, y, red, green, blue, alpha);
}

void OverlayRaster::graypeek(unsigned long x, unsigned long y, unsigned int& i)
{
  float rval, gval, bval, aval;
  peek(x, y, rval, gval, bval, aval);
  i = (unsigned int) (gval*(float)0xff); 
}

void OverlayRaster::graypeek(unsigned long x, unsigned long y, unsigned long& l)
{
  float rval, gval, bval, aval;
  peek(x, y, rval, gval, bval, aval);
  l = (unsigned long) (gval*(float)0xff); 
}

void OverlayRaster::graypeek(unsigned long x, unsigned long y, float& f)
{
  float rval, gval, bval, aval;
  peek(x, y, rval, gval, bval, aval);
  f = (float) (gval*(float)0xff); 
}

void OverlayRaster::graypeek(unsigned long x, unsigned long y, double& d)
{
  float rval, gval, bval, aval;
  peek(x, y, rval, gval, bval, aval);
  d = (double) (gval*(float)0xff); 
}

void OverlayRaster::graypeek(unsigned long x, unsigned long y, AttributeValue& val)
{
  float rval, gval, bval, aval;
  peek(x, y, rval, gval, bval, aval);
  val.double_ref() = (double) (gval*(float)0xff); 
}

void OverlayRaster::graypoke(unsigned long x, unsigned long y, unsigned int i)
{
    if (!gray_initialized())
	gray_init();
    RasterRep* r = rep();
    if (!_gray_map) {
	float value = float(i)/0xff;
	poke(x, y, value, value, value, 1.0);
    }
    else {
	if (!r->pixmap_) init_space();
	XPutPixel(r->image_, (unsigned int)x, r->pheight_ - (unsigned int)y - 1, _gray_map[i].pixel);
    }
    r->modified_ = true;
}

void OverlayRaster::graypoke(unsigned long x, unsigned long y, unsigned long l)
{
  graypoke(x, y, (unsigned int)l);
}

void OverlayRaster::graypoke(unsigned long x, unsigned long y, float f)
{
  graypoke(x, y, (unsigned int)f);
}

void OverlayRaster::graypoke(unsigned long x, unsigned long y, double d)
{
  graypoke(x, y, (unsigned int)d);
}

void OverlayRaster::graypoke(unsigned long x, unsigned long y, AttributeValue val)
{
  graypoke(x, y, val.uint_val());
}

long OverlayRaster::gray_lookup(int byte) 
{
    if (!gray_initialized())
	gray_init();
    if (_gray_map)
	return _gray_map[byte].pixel;
    else 
	return -1;
}

void OverlayRaster::initialized(boolean init) {
    _init = init;
}

boolean OverlayRaster::initialized() {
    RasterRep* r = rep();
    return ((r->pixmap_ != 0) & _init) ? true : false;
}

void OverlayRaster::initialize() {
    _init = true;
    flush();
    // ensure that pixmap_ is created
    init_space();
}

void OverlayRaster::flush() const {
    RasterRep* r = rep();
    if (r->pixmap_)
      Raster::flush();
}

void OverlayRaster::flushrect(IntCoord left, IntCoord bottom, 
			      IntCoord right, IntCoord top) const {
    RasterRep* r = rep();
    if (r->pixmap_)
	Raster::flushrect(left, bottom, right, top);
}

int OverlayRaster::status() const {
  return 0;  // ok by default
}

int OverlayRaster::gray_init() {
    if (gray_initialized()) return 0;
    int status = gray_init(7);
    if (status)
	status = gray_init(6);
    if (status)
	status = gray_init(5);
    if (status) {
	delete _gray_map;
	_gray_map = nil;
    }
    _gray_initialized = true;
    return status;
}

int OverlayRaster::gray_init(int nbits)
{
    if (gray_initialized()) return 0;

    /* allocate graymap for 8 bit images */
    const int GRAY_LEVELS = 256;
    if (!_gray_map) 
	_gray_map = new XColor[GRAY_LEVELS];
    _unique_grays = 2<<(nbits-1);
    
    /* Allocate a contiguous block within this color map */
    /* one greater than _unique_grays in length.         */
    XColormap colormap = 
	DefaultColormap(Session::instance()->default_display()->rep()->display_,
			Session::instance()->default_display()->rep()->screen_);
    unsigned long* indices = new unsigned long[_unique_grays+1];
    if( XAllocColorCells(Session::instance()->default_display()->rep()->display_, 
			 colormap, true, 0, 0, indices, _unique_grays+1 ) == 0 ) {
	delete indices;	
	return -1;
    }
    
    /* Adjust the allocated block of color cells so it */
    /* starts on an even boundary.                     */
    int free_cell = (indices[0]&0x1) ? _unique_colors : 0;
    int status = XFreeColors( Session::instance()->default_display()->rep()->display_, 
			      colormap, indices+free_cell, 1, 0 );
    int align = free_cell==0 ? 1 : 0;
    
    /* Set up the colormap, such that contiguous entries in the color */
    /* map are differing in the most-significant bit.                 */
    unsigned long delta = 65536/_unique_grays;
    unsigned long lo_color = 0;
    unsigned long hi_color = 32768;
    int repfactor = GRAY_LEVELS/_unique_grays;
    int off_lo;
    int off_hi;

#if 0
    cerr << "nbits: " << nbits << "\t_unique_grays: " << _unique_grays << 
     "\tdelta: " << delta << "\n";
#endif 
    
    for(int i=0; i<_unique_grays; i+=2 ) {

	/* Lower half of the gray scale, values replicated GRAY_LEVELS/_unique_grays */
	off_lo = i*repfactor/2;


#if 0
        cerr << "off_lo: " << off_lo << "\tlo_color: " << lo_color << "\n";
#endif

	_gray_map[off_lo].red = _gray_map[off_lo].green = _gray_map[off_lo].blue = 
	    lo_color;
	_gray_map[off_lo].pixel = indices[align+i];
	_gray_map[off_lo].flags = DoRed | DoGreen | DoBlue;
	for (int k=1; k<repfactor; k++) 
	    _gray_map[off_lo+k] = _gray_map[off_lo];
	XStoreColor(Session::instance()->default_display()->rep()->display_,
		    colormap,_gray_map+off_lo);
#if 0
        cerr << "low color: " << (_gray_map+off_lo)->red << "\n";
#endif
	lo_color+=delta;
	
	/* Upper half of the gray scale, values replicated GRAY_LEVELS/_unique_grays */
	off_hi = i*repfactor/2 + GRAY_LEVELS/2;

#if 0
        cerr << "off_hi: " << off_hi << "\thi_color: " << hi_color << "\n";
#endif

	_gray_map[off_hi].red = _gray_map[off_hi].green = _gray_map[off_hi].blue = 
	    hi_color;
	_gray_map[off_hi].pixel = indices[align+i+1];
	_gray_map[off_hi].flags = DoRed | DoGreen | DoBlue;
	for (int k=1; k<repfactor; k++) 
	    _gray_map[off_hi+k] = _gray_map[off_hi];
	XStoreColor(Session::instance()->default_display()->rep()->display_,
		    colormap,_gray_map+off_hi);
#if 0
        cerr << "\thi color: " << (_gray_map+off_hi)->red << "\n";
#endif
	hi_color+=delta;
	
    }

    delete indices;
    _gray_initialized = true;
    return 0;
}


int OverlayRaster::color_init(int nlevels)
{
    if (_color_map) return 0;
    if (nlevels > 6 || nlevels < 5) return -1;

    /* allocate colormap for 8 bit images */
    _unique_colors = nlevels*nlevels*nlevels;
    _color_map = new XColor[_unique_colors];
    
    /* Allocate a contiguous block within this color map */
    /* one greater than _unique_colors in length.         */
    XColormap colormap = 
	DefaultColormap(Session::instance()->default_display()->rep()->display_,
			Session::instance()->default_display()->rep()->screen_);
    unsigned long* indices = new unsigned long[_unique_colors+1];
    if( XAllocColorCells(Session::instance()->default_display()->rep()->display_, 
			 colormap, true, 0, 0, indices, _unique_colors+1 ) == 0 ) {
	delete indices;	
	return -1;
    }
    
    /* Adjust the allocated block of color cells so it */
    /* starts on an even boundary.                     */
    int index0 = indices[0];
    int free_cell = (indices[0]&0x1) ? _unique_colors : 0;
    int status = XFreeColors( Session::instance()->default_display()->rep()->display_, 
			      colormap, indices+free_cell, 1, 0 );
    int align = free_cell==0 ? 1 : 0;
    
    /* Set up the colormap, such that contiguous entries in the color */
    /* map are differing as much as possible                          */
    unsigned long delta = 65536/(nlevels-1);
    unsigned long red_lo = 0;
    unsigned long grn_lo = 0;
    unsigned long blu_lo = 0;
    unsigned long red_hi = nlevels==6 ? 3*delta : 2*delta;
    unsigned long grn_hi = nlevels==6 ? 0 : 2*delta;
    unsigned long blu_hi = nlevels==6 ? 0 : 3*delta;
    
    for(int i=0; i<_unique_colors; i+=2 ) {
	
	/* Lower half of the color scale */
	_color_map[i].red = red_lo;
	_color_map[i].green = grn_lo;
	_color_map[i].blue = blu_lo;
	int offset = _unique_colors%2 ? _unique_colors-1 : _unique_colors-2;
	_color_map[i].pixel = indices[offset - (i-align)];
	_color_map[i].flags = DoRed | DoGreen | DoBlue;
	XStoreColor(Session::instance()->default_display()->rep()->display_,
		    colormap,_color_map+i);

	if ((blu_lo += delta) > 65536) {
	    blu_lo = 0;
	    if ((grn_lo += delta) > 65536) {
		grn_lo = 0;
		red_lo += delta;
		if (red_lo == 65536) red_lo--;
	    } 
	    else if (grn_lo==65536) grn_lo--;
	} 
	else if (blu_lo == 65536) blu_lo--;

	int j = i+1;
	if (j==_unique_colors) break;
	
	/* Upper half of the color scale */
	_color_map[j].red = red_hi;
	_color_map[j].green = grn_hi;
	_color_map[j].blue = blu_hi;
	_color_map[j].pixel = indices[j-align];
	_color_map[j].flags = DoRed | DoGreen | DoBlue;
	XStoreColor(Session::instance()->default_display()->rep()->display_,
		    colormap,_color_map+j);
	
	if ((blu_hi += delta) > 65536) {
	    blu_hi = 0;
	    if ((grn_hi += delta) > 65536) {
		grn_hi = 0;
		red_hi += delta;
		if (red_hi == 65536) red_hi--;
	    } 
	    else if (grn_hi==65536) grn_hi--;
	} 
	else if (blu_hi == 65536) blu_hi--;

    }
    
    delete indices;
    return 0;
}


OverlayRaster* OverlayRaster::scale(
    ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
) {
    OverlayRaster* nrast = new OverlayRaster(*this);
    nrast->scale(mingray, maxgray);
    cmd = ScaleGrayFunc::CommandString(mingray,maxgray);
    return nrast;
}


OverlayRaster* OverlayRaster::pseudocolor(
    ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
) {
    OverlayRaster* nrast = pseudocolor(mingray, maxgray);
    cmd = PseudocolorFunc::CommandString(mingray,maxgray);
    return nrast;
}


OverlayRaster* OverlayRaster::logscale(
    ColorIntensity minintensity, ColorIntensity maxintensity, CopyString& cmd
) {
    OverlayRaster* nrast = new OverlayRaster(*this);
    nrast->logscale(minintensity, maxintensity);
    cmd = LogScaleFunc::CommandString(minintensity, maxintensity);
    return nrast;
}


OverlayRaster* OverlayRaster::addgrayramp(
    CopyString& cmd, RampAlignment algn
) {
    OverlayRaster* nrast = new OverlayRaster(*this);
    nrast->_addgrayramp(algn);
    cmd = GrayRampFunc::CommandString(algn);
    return nrast;
}


OverlayRaster* OverlayRaster::addgrayramp(
    CopyString& cmd, Coord x, Coord y
) {
    OverlayRaster* nrast = new OverlayRaster(*this);
    RampAlignment algn = ramppos(x, y);
    nrast->_addgrayramp(algn);
    cmd = GrayRampFunc::CommandString(algn);
    return nrast;
}


void OverlayRaster::scale(
    ColorIntensity mingray, ColorIntensity maxgray
) {
    RasterRep* rp = rep();
    float fmin = mingray * 0xff;
    float fmax = maxgray * 0xff;
    int min = Math::round(fmin);
    int max = Math::round(fmax);

    float ratio = ((fmax - fmin) == 0) ? 0. : (0xff / (max - min));

    unsigned int width = rp->pwidth_;
    unsigned int height = rp->pheight_;
    unsigned int byte;
    int w,h;

    for (w = 0; w < width; w++) {
        for (h = 0; h < height; h++) {
            graypeek(w, h, byte);
            if (byte < min) byte = min;
            if (byte > max) byte = max;
            unsigned int newval = Math::round((byte - min) * ratio);
            graypoke(w, h, newval);
        }
    }
}


OverlayRaster* OverlayRaster::pseudocolor(
    ColorIntensity mingray, ColorIntensity maxgray
) {
    OverlayRaster* color = new OverlayRaster(pwidth(), pheight());

    float ratio = (1.0 / (maxgray - mingray));
    int steps = 5;

    unsigned int byte;
    float gray;

    RasterRep* rp = rep();
    unsigned int width = rp->pwidth_;
    unsigned int height = rp->pheight_;
    int w,h;

    for (w = 0; w < width; w++) {
	for (h = 0; h < height; h++) {

	    graypeek(w, h, byte);
            gray = byte / float(0xff);

	    if (gray < mingray) gray = mingray;
	    if (gray > maxgray) gray = maxgray;
 	    float grayfract = (gray - mingray) * ratio;

	    grayfract = 
                (grayfract*steps - fmod(grayfract*steps,1.0)) / (float)steps;
	    float newr, newg, newb;

#if 0
	    newr = grayfract * maxgray;
	    newg = (1.0 - grayfract) * maxgray;
	    newb = 0.0;
#else
	    grayfract *= maxgray;
	    newr = grayfract < 0.5 ? 0.0 : (grayfract-.5)*2;
	    newg = grayfract < 0.5 ? grayfract*2 : 1.0 - (grayfract-.5)*2;
	    newb = grayfract < 0.5 ? 1.0 - (grayfract-.5)*2 : 0.0;
	    newr = max((float)0.0, newr);
	    newg = max((float)0.0, newg);
	    newb = max((float)0.0, newb);
#endif

	    color->poke(w, h, newr, newg, newb, 1.0);
        }
    }
    return color;
}


void OverlayRaster::logscale 
( ColorIntensity mingray, ColorIntensity maxgray )
{
  int n = 255;
  int min, max;
  min = Math::round(mingray * 0xff);
  max = Math::round(maxgray * 0xff);

  RasterRep* rp = rep();
  unsigned int width = rp->pwidth_;
  unsigned int height = rp->pheight_;
  int w,h;
  unsigned int byte;
  int nvals = max-min+1;
  double e = exp(1.0);
  
  for (w = 0; w < width; w++) {
    for (h = 0; h < height; h++) {
      graypeek(w, h, byte);
      if (byte < min) byte = min;
      if (byte > max) byte = max;
#if 0
      unsigned int ival = 
	(unsigned int)Math::round(pow( mingray, ((n - byte) / float(n) ) ) * float(n));
#else
      double val = (byte-((double)min)) / nvals * (e - 1.0) + 1.0;
      unsigned int ival = (unsigned int) (log(val)*n);
#endif
      graypoke(w, h, ival);
      
    }
  }
}

static float dist(
    float x1, float y1, float x2, float y2
) {
    float xd = x2 - x1;
    float yd = y2 - y1;
    return sqrt((xd*xd) + (yd*yd));
}


RampAlignment OverlayRaster::ramppos(IntCoord x, IntCoord y) {
    float xside[4];
    float yside[4];

    xside[0] = 0;             yside[0] = y;              // l
    xside[1] = pwidth() - 1;  yside[1] = y;              // r
    xside[2] = x;             yside[2] = 0;              // b
    xside[3] = x;             yside[3] = pheight() - 1 ; // t

    float dists[4];
    int i;
    for (i = 0; i < 4; i++) {
        dists[i] = dist(x, y, xside[i], yside[i]);
    }

    float side = min(min(dists[0], dists[1]), min(dists[2], dists[3]));

    RampAlignment align;
    if ( side == dists[0] ) {
        align = (y > pheight() / 2) ? R_LT : R_LB;
    }
    else if ( side == dists[1] ) {
        align = (y > pheight() / 2) ? R_RT : R_RB;
    }
    else if ( side == dists[2] ) {
        align = (x > pwidth() / 2) ? R_BR : R_BL;
    }
    else {
        align = (x > pwidth() / 2) ? R_TR : R_TL;
    }

    return align;
}


void OverlayRaster::computeramp(
    boolean vert, RampAlignment align, IntCoord& width, IntCoord& height
) {
    // the length will be 1/size * length of longer dimension

    float size = 4.;
    const char* csize = unidraw->GetCatalog()->GetAttribute("rampsize");
    if (csize) {
        size = atoi(csize);
        if (size < 1. || size > 30.) {
            cerr << "rampsize < 1 or > 30 is ignored, using default\n";
            size = 4.;
        }
    }
    float fw, fh;

    if (pwidth() > pheight()) {
        if (vert) {
            fw = pwidth() / (size * 4.);
            fh = fw * 4.;

            if (fh > pheight()) {
                fh = pheight();
                fw = fh / 4.;
            }
        }
	else {
            fw = pwidth() / size;
            fh = fw / 4.;

            if (fh > pheight()) {
                fh = pheight();
                fw = (fh * 4.) < pwidth() ? (fh * 4.) : pwidth();
            }
        }
    }
    else {
        if (vert) {
            fh = pheight() / size;
  	    fw = fh / 4.;

            if (fw > pwidth()) {
                fw = pwidth();
                fh = (fw * 4.) < pheight() ? (fw * 4.) : pheight();
            }
        }
	else {
            fh = pheight() / (size * 4.);
            fw = fh * 4.;

            if (fw > pwidth()) {
                fw = pwidth();
                fh = fw / 4.;
            }
        }
    }

    width = Math::round(fw);
    height = Math::round(fh);
}


void OverlayRaster::_addgrayramp(
    RampAlignment align, IntCoord w, IntCoord h
) {
    u_long l, b;

    boolean horiz = false;
    switch (align) {
    case R_TL:
    case R_TR:
    case R_BL:
    case R_BR:
        horiz = true;
        break;
    }

    if (w == 0 || h == 0) {
        computeramp(!horiz, align, w, h);
    }

    switch (align) {
    case R_TL:
    case R_LT:
        l = 0;
        b = pheight() - h;
        break;
    case R_TR:
    case R_RT:
        l = pwidth() - w;
        b = pheight() - h;
        break;
    case R_BL:
    case R_LB:
        l = 0;
        b = 0;
        break;
    case R_BR:
    case R_RB:
        l = pwidth() - w;
        b = 0;
        break;
    }

    paintgrayramp(b, l, h, w, horiz);
}

void OverlayRaster::paintgrayramp
(IntCoord l, IntCoord b, unsigned int w, unsigned int h, boolean horiz) {

    IntCoord rows = b + h;
    IntCoord cols = l + w;

    float gray;
    IntCoord row;
    IntCoord col;
    for (row = b; row < rows; row++) {
        for (col = l; col < cols; col++) {

	    if (horiz) {
  	        gray = (float(col) - l) / 
                    (((cols - l) == 1) ? 1 : ((cols - l) - 1));
	    }
            else {
  	        gray = (float(row) - b) / 
                    (((rows - b) == 1) ? 1 : ((rows - b) - 1));
            }   

            poke(col, row, gray, gray, gray, 1.); 
        }
    }
}



boolean OverlayRaster::write (ostream& out) {
  Coord w = Width();
  Coord h = Height();

  if (!gray_flag()) {
    out << w << "," << h << ",";
    int x = 0;	
    int y = 0;
    ColorIntensity r, g, b;
    int ir, ig, ib;
    float alpha;	
    for (y; y < h; y++) {
      x = 0;
      for (x; x < w; x++) {
	peek(x, y, r, g, b, alpha);
	int ir = (int)(r*255);
	int ig = (int)(g*255);	
	int ib = (int)(b*255);
	out << "(" << ir << "," << ig << "," << ib << ")";
	if (!(y == h-1 && x == w-1))
	  out << ",";
      }
      if (y != h-1)
	out << "\n";
    }
  } else {
    out << w << "," << h << ",";
    int x = 0;	
    int y = 0;
    unsigned int byte;
    for (y; y < h; y++) {
      x = 0;
      for (x; x < w; x++) {
	graypeek(x, y, byte);
	out << byte;
	if (!(y == h-1 && x == w-1))
	  out << ",";
      }
      if (y != h-1)
	out << "\n";
    }
  }
  return true;
}

boolean OverlayRaster::read(istream& in) {
  int w = Width();
  int h = Height();

  char delim;

  if (!gray_flag()) {
    char paren;
    int ir, ig, ib;
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	in >> paren >> ir >> delim >> ig >> delim >> ib >> paren;
	poke(x, y, float(ir)/0xff, float(ig)/0xff, float(ib)/0xff, 1.0);
	if (!(y == h-1 && x == w-1))
	  in >> delim;
      }
    }
  } 

  else {
    unsigned int byte;
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	in >> byte;
	graypoke(x, y, byte);
	if (!(y == h-1 && x == w-1))
	  in >> delim;
      }
    }
  }
  return true;
}
