/*
 * Copyright (c) 1994-1997 Vectaport Inc.
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
 * OverlayCatalog implementation.
 */

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovcreator.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovfile.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/ovstencil.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/paramlist.h>
#include <OverlayUnidraw/textfile.h>

#include <UniIdraw/idarrows.h>

#include <Unidraw/Components/component.h>
#include <Unidraw/Components/externview.h>
#include <Unidraw/Components/psformat.h>
#include <Unidraw/Components/text.h>

#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Graphic/polygons.h>
#include <Unidraw/Graphic/rasterrect.h>
#include <Unidraw/Graphic/ustencil.h>

#include <Unidraw/unidraw.h>

#include <InterViews/bitmap.h>
#include <InterViews/raster.h>
#include <InterViews/transformer.h>

#include <OS/math.h>

#include <ctype.h>
#include <stdio.h>
#include <stream.h>
#include <string.h>
#include <fstream.h>


/*****************************************************************************/

static const int hex_encode = 6;
static const unsigned int color_base = 255;     // 2^color_depth - 1

static char hexcharmap[] = {
     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
     'a', 'b', 'c', 'd', 'e', 'f'
};

static unsigned int hexintmap[] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,
     0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

static const char* HexEncode (
    ColorIntensity ir, ColorIntensity ig, ColorIntensity ib
) {
    unsigned int r = Math::round(ir * color_base);
    unsigned int g = Math::round(ig * color_base);
    unsigned int b = Math::round(ib * color_base);

    static char enc[hex_encode+1];
    enc[hex_encode] = '\0';

    enc[0] = hexcharmap[r >> 4 & 0xf];
    enc[1] = hexcharmap[r & 0xf];
    enc[2] = hexcharmap[g >> 4 & 0xf];
    enc[3] = hexcharmap[g & 0xf];
    enc[4] = hexcharmap[b >> 4 & 0xf];
    enc[5] = hexcharmap[b & 0xf];

    return enc;
}

static void HexDecode (
    const char* enc, 
    ColorIntensity& ir, ColorIntensity& ig, ColorIntensity& ib
) {
    unsigned int r, g, b;

    r  = hexintmap[enc[0]] << 4;
    r |= hexintmap[enc[1]];
    g  = hexintmap[enc[2]] << 4;
    g |= hexintmap[enc[3]];
    b  = hexintmap[enc[4]] << 4;
    b |= hexintmap[enc[5]];

    ir = float(r) / float(color_base);
    ig = float(g) / float(color_base);
    ib = float(b) / float(color_base);
}

/*****************************************************************************/

OverlayCatalog::OverlayCatalog (
    const char* name, Creator* creator
) : IdrawCatalog(name, creator) {
    _parent = nil;
    _gs_compacted = _pts_compacted = _pic_compacted = false;
    _ed = nil;
}

boolean OverlayCatalog::Save (Component* comp, const char* name) {
    boolean ok = false;

    if (UnidrawFormat(name)) {
        ok = Catalog::Save(comp, name);

    } else {
        ExternView* ev = (ExternView*) comp->Create(SCRIPT_VIEW);

        if (ev != nil) {
            filebuf fbuf;
            ok = fbuf.open(name, output) != 0;

            if (ok) {
		((OverlayComp*)comp)->SetPathName(name);
                ostream out(&fbuf);
                comp->Attach(ev);
                ev->Update();
		((OverlaysScript*)ev)->SetCompactions(
		    _gs_compacted,
		    _pts_compacted,
		    _pic_compacted);
                ok = ev->Emit(out);

                if (ok) {
                    Forget(comp, name);
                    Register(comp, name);
                }
            }
            delete ev;
        }
    }
    return ok;
}

boolean OverlayCatalog::Retrieve (const char* filename, Component*& comp) {
    FILE* fptr = nil;
    boolean compressed = false;
    char* name = strdup(filename);
    if (Valid(name, comp)) {
        _valid = true;

    } else {
#if __GNUG__<3	  
        filebuf fbuf;
	if (strcmp(name, "-") == 0) {
	    _valid = fbuf.attach(fileno(stdin)) != 0;
	    name = nil;
	} else {
	    fptr = fopen(name, "r");
	    fptr = OvImportCmd::CheckCompression(fptr, name, compressed);
	    _valid = fptr ? fbuf.attach(fileno(fptr)) != 0 : false;
	    if (compressed) {
		int namelen = strlen(name);
		if (strcmp(name+namelen-3,".gz")==0) name[namelen-3] = '\0';
		else if (strcmp(name+namelen-2,".Z")==0) name[namelen-2] = '\0';
	    }
	}
#else
	boolean stdin_flag = strcmp(name, "-")==0;
	if (!stdin_flag) {
	  fptr = fopen(name, "r");
	  fptr = fptr ? OvImportCmd::CheckCompression(fptr, name, compressed) : nil;
	  _valid = fptr != nil;
	  if (compressed) {
	    int namelen = strlen(name);
	    if (strcmp(name+namelen-3,".gz")==0) name[namelen-3] = '\0';
	    else if (strcmp(name+namelen-2,".Z")==0) name[namelen-2] = '\0';
	  }
	} else {
	  _valid = 1;
	  name = nil;
	}
	if (!_valid && !ParamList::urltest(name)) return false;
        fileptr_filebuf fbuf(stdin_flag ? stdin : fptr, ios_base::in);
#endif
	
        if (_valid || ParamList::urltest(name)) {
	    istream in(&fbuf);
#if 0
	    const char* command = "drawtool";
	    int len = strlen(command)+1;
	    char buf[len];

	    char ch;
	    while (isspace(ch = in.get())); in.putback(ch);
	    ParamList::parse_token(in, buf, len);
	    if (strcmp(buf, command) == 0 || strcmp(buf, "ov-idraw") == 0) { 
		comp = new OverlayIdrawComp(in, name, _parent);
		_valid = in.good() && ((OverlayComp*)comp)->valid();
	    } else 
		_valid = false;

#else
	    Editor* ed = GetEditor();
	    OvImportCmd importcmd(ed);
	    importcmd.pathname(name);
	    if (ParamList::urltest(name)) {
	      comp = importcmd.Import(name);
	    } else {
	      boolean empty;
	      comp = importcmd.Import(in, empty);
	    }
	    _valid = in.good() && comp && ((OverlayComp*)comp)->valid();
	    if (!_valid) {
	      delete comp;
	      comp = nil;
	    }
	    boolean idrawcompflag = true;
	    if (comp && !comp->IsA(OVERLAY_IDRAW_COMP)) {
	      idrawcompflag = false;
	      OverlayIdrawComp* icomp = new OverlayIdrawComp();
	      icomp->Append((GraphicComp*)comp);
	      comp = icomp;
	    }
	    
#endif

            if (_valid && name && idrawcompflag) {
                Forget(comp, name);
                Register(comp, name);
            } else if (!_valid) {
		delete comp;
		comp = nil;
	    }
        }
    }
    
    if (fptr) {
	if (compressed) 
	    fclose(fptr);
	else
	    pclose(fptr);
    }
    delete name;
    return _valid;
}


GraphicComp* OverlayCatalog::ReadPostScript (istream& in) {
    Skip(in);
    in >> _buf >> _psversion;

    if (_psversion > PSV_LATEST) {
        fprintf(stderr, "warning: drawing version %lf ", _psversion);
        fprintf(stderr, "newer than idraw version %lf\n", PSV_LATEST);
    }

    float xincr, yincr;
    PSReadGridSpacing(in, xincr, yincr);    // grid is ignored because this
    OverlaysComp* comp = new OverlaysComp;  // method is only used for importing

    if (_psversion < PSV_NONREDUNDANT) {
        Skip(in);
    }

    Graphic* g = comp->GetGraphic();
    Transformer* t = g->GetTransformer();

    PSReadPictGS(in, g);
    PSReadChildren(in, comp);
    ScaleToScreenCoords(g);

    if (_psversion < PSV_NONROTATED && t != nil && t->Rotated90()) {
        Transformer identity;
        *t = identity;
        g->Translate(0.0, -8.5*ivinches);
        g->Rotate(90.0, 0.0, 0.0);
        comp->Bequeath();
    }

    _valid = in.good() && !_failed;
    return comp;
}

/*
 * PSReadChildren loops determining which kind of Component follows and
 * creating it until it reads "end" which means all of the children
 * have been created.
*/

void OverlayCatalog::PSReadChildren (istream& in, GraphicComp* comp) {
    _failed = false;
    while (in.good()) {
	Skip(in);
	GraphicComp* child = nil;
	in >> _buf;

	if (strcmp(_buf, "BSpl") == 0) 		child = ReadBSpline(in);
	else if (strcmp(_buf, "Circ") == 0) 	child = ReadCircle(in);
	else if (strcmp(_buf, "CBSpl") == 0)    child = ReadClosedBSpline(in);
	else if (strcmp(_buf, "Elli") == 0)     child = ReadEllipse(in);
	else if (strcmp(_buf, "Line") == 0)     child = ReadLine(in);
	else if (strcmp(_buf, "MLine") == 0)    child = ReadMultiLine(in);
	else if (strcmp(_buf, "Pict") == 0)     child = ReadPict(in);
	else if (strcmp(_buf, "Poly") == 0)     child = ReadPolygon(in);
	else if (strcmp(_buf, "Rect") == 0)     child = ReadRect(in);
	else if (strcmp(_buf, "Text") == 0)     child = ReadText(in);
	else if (strcmp(_buf, "SSten") == 0)    child = ReadSStencil(in);
	else if (strcmp(_buf, "FSten") == 0)    child = ReadFStencil(in);
	else if (strcmp(_buf, "Rast") == 0)     child = ReadRaster(in);
	else if (strcmp(_buf, "ColorRast") ==0) {
	  child = nil; 
	  cerr << "Support for reading idraw PostScript with color-printer ready rasters not yet available.\n"; 
	}
	else if (strcmp(_buf, "eop") == 0)      break;

	else {
	    fprintf(stderr, "unknown graphical object %s\n", _buf);
	    _failed = true;
	    break;
	}

	if (child != nil) {
	    if (in.good()) {
		comp->Append(child);
	    } else {
		delete child;
	    }
	}
    }
}

/*
 * reads data to initialize graphic comp and create children
 */

GraphicComp* OverlayCatalog::ReadPict (istream& in) {
    FullGraphic gs;
    PSReadPictGS(in, &gs);
    GraphicComp* pict = new OverlaysComp;
    *pict->GetGraphic() = gs;
    PSReadChildren(in, pict);
    _valid = in.good();
    return pict;
}


/*
 * ReadBSpline reads data to initialize its graphic comp and
 * create its components.
 */

GraphicComp* OverlayCatalog::ReadBSpline (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Coord* x, *y;
    int n;

    const Coord* cx, * cy;
    PSReadPoints(in, cx, cy, n);
    x = (Coord*)cx; y = (Coord*)cy;

    float mag;
    if (_psversion < PSV_UNIDRAW) {
        mag = 1.;
    } else {
        Skip(in);
        in >> mag;
    }
    return new ArrowSplineOvComp(
        new ArrowOpenBSpline(x, y, n, _head, _tail, mag, &gs)
    );
}

/*
 * ReadClosedBSpline reads data to initialize its graphic comp
 * and create the closed B-spline's filled interior and outline.
 */

GraphicComp* OverlayCatalog::ReadClosedBSpline (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Coord* x, *y;
    int n;

    const Coord* cx, * cy;
    PSReadPoints(in, cx, cy, n);
    x = (Coord*)cx; y = (Coord*)cy;
    return new ClosedSplineOvComp(new SFH_ClosedBSpline(x, y, n, &gs));
}

/*
 * ReadRect reads data to initialize its graphic comp and create
 * its filled interior and outline.
 */

GraphicComp* OverlayCatalog::ReadRect (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Skip(in);
    Coord l, b, r, t;
    in >> l >> b >> r >> t;
    return new RectOvComp(new SF_Rect(l, b, r, t, &gs));
}

GraphicComp* OverlayCatalog::ReadPolygon (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Coord* x, *y;
    int n;

    const Coord* cx, * cy;
    PSReadPoints(in, cx, cy, n);
    x = (Coord*)cx; y = (Coord*)cy;
    return new PolygonOvComp(new SF_Polygon(x, y, n, &gs));
}

/*
 * ReadLine reads data to initialize its graphic comp and create
 * its line.
 */

GraphicComp* OverlayCatalog::ReadLine (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Skip(in);
    Coord x0, y0, x1, y1;
    in >> x0 >> y0 >> x1 >> y1;

    float mag;
    if (_psversion < PSV_UNIDRAW) {
        mag = 1.;
    } else {
        Skip(in);
        in >> mag;
    }
    return new ArrowLineOvComp(new ArrowLine(x0,y0,x1,y1,_head,_tail,mag,&gs));
}

/*
 * ReadMultiLine reads data to initialize its graphic comp and
 * create its components.
 */

GraphicComp* OverlayCatalog::ReadMultiLine (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Coord* x, *y;
    int n;

    const Coord* cx, * cy;
    PSReadPoints(in, cx, cy, n);
    x = (Coord*)cx; y = (Coord*)cy;

    float mag;
    if (_psversion < PSV_UNIDRAW) {
        mag = 1.;
    } else {
        Skip(in);
        in >> mag;
    }
    return new ArrowMultiLineOvComp(
        new ArrowMultiLine(x, y, n, _head, _tail, mag, &gs)
    );
}

/*
 * ReadEllipse reads data to initialize its graphic comp and
 * create its filled interior and outline.
 */

GraphicComp* OverlayCatalog::ReadEllipse (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Skip(in);
    Coord x0, y0;
    int rx, ry;
    in >> x0 >> y0 >> rx >> ry;
    return new EllipseOvComp(new SF_Ellipse(x0, y0, rx, ry, &gs));
}

/*
 * ReadCircle reads data to initialize its graphic comp and
 * create its filled interior and outline.
 */

GraphicComp* OverlayCatalog::ReadCircle (istream& in) {
    FullGraphic gs;
    PSReadGS(in, &gs);
    Skip(in);
    Coord x0, y0;
    int r;
    in >> x0 >> y0 >> r;
    return new EllipseOvComp(new SF_Circle(x0, y0, r, &gs));
}

/*
 * ReadText reads its graphic comp and text in a file.
 */

GraphicComp* OverlayCatalog::ReadText (istream& in) {
    FullGraphic gs;
    PSReadTextGS(in, &gs);
    PSReadTextData(in, sbuf, SBUFSIZE);

    int lineHt = 0;

    PSFont* f = gs.GetFont();
    if (f != nil) lineHt = f->GetLineHt();

    TextGraphic* tg = new TextGraphic(sbuf, lineHt, &gs); 
    tg->FillBg(false);
    return new TextOvComp(tg);
}

GraphicComp* OverlayCatalog::ReadSStencil (istream& in) {
    FullGraphic gs;
    PSReadFgColor(in, &gs);
    PSReadBgColor(in, &gs);
    PSReadTransformer(in, &gs);
    Skip(in);
    Coord w, h;
    in >> w >> h;

    Bitmap* bitmap = new Bitmap((void*) nil, w, h);
    ReadBitmapData(bitmap, in);

    return new StencilOvComp(new UStencil(bitmap, bitmap, &gs));
}

GraphicComp* OverlayCatalog::ReadFStencil (istream& in) {
    FullGraphic gs;
    PSReadFgColor(in, &gs);
    PSReadBgColor(in, &gs);
    PSReadTransformer(in, &gs);
    Skip(in);
    Coord w, h;
    in >> w >> h;

    Bitmap* bitmap = new Bitmap((void*) nil, w, h);
    ReadBitmapData(bitmap, in);

    return new StencilOvComp(new UStencil(bitmap, nil, &gs));
}

GraphicComp* OverlayCatalog::ReadRaster (istream& in) {
    FullGraphic gs;
    PSReadTransformer(in, &gs);
    Skip(in);
    Coord w, h;
    in >> w >> h;

    char* sync_string = "colorimage";
    int n = strlen(sync_string);

    while (GetToken(in, _buf, CHARBUFSIZE) != 0) {
        if (strncmp("colorimage", _buf, n) == 0) {
            break;
        }
    }

    OverlayRaster* raster = new OverlayRaster(w, h);
    ReadRasterData(raster, in);

    return new RasterOvComp(new OverlayRasterRect(raster, &gs));
}

void OverlayCatalog::SetParent(OverlayComp* parent) {
    _parent = parent;
}

void OverlayCatalog::SetCompactions(boolean gs, boolean pts, boolean pic) {
    _gs_compacted = gs;
    _pts_compacted = pts;
    _pic_compacted = pic;
}

OverlayComp* OverlayCatalog::ReadComp(const char* name, istream& in, OverlayComp* parent) {
  OverlayComp* child = nil;

  if (strcmp(name, "aln") == 0 ||
      strcmp(name, "arrowline") == 0)           child = new ArrowLineOvComp(in, parent);
  
  else if (strcmp(name, "aml") == 0 ||
	   strcmp(name, "arrowmultiline") == 0) child = new ArrowMultiLineOvComp(in, parent);
  
  else if (strcmp(name, "asp") == 0 ||
	   strcmp(name, "arrowspline") == 0)    child = new ArrowSplineOvComp(in, parent);
  
  else if (strcmp(name, "bsp") == 0 ||
	   strcmp(name, "bspline") == 0)        child = new SplineOvComp(in, parent);
  
  else if (strcmp(name, "csp") == 0 ||
	   strcmp(name, "closedspline") == 0)   child = new ClosedSplineOvComp(in, parent);
  
  else if (strcmp(name, "ell") == 0 ||
	   strcmp(name, "ellipse") == 0)        child = new EllipseOvComp(in, parent);

  else if (strcmp(name, "ln") == 0 ||
	   strcmp(name, "line") == 0)           child = new LineOvComp(in, parent);
  
  else if (strcmp(name, "mln") == 0 ||
	   strcmp(name, "multiline") == 0)      child = new MultiLineOvComp(in, parent);
  
  else if (strcmp(name, "picture") == 0 ||
	   strcmp(name, "grp") == 0) 	        child = new OverlaysComp(in, parent);
  
  else if (strcmp(name, "poly") == 0 ||
	   strcmp(name, "polygon") == 0)        child = new PolygonOvComp(in, parent);
  
  else if (strcmp(name, "rast") == 0 ||
	   strcmp(name, "raster") == 0)         child = new RasterOvComp(in, parent);
  
  else if (strcmp(name, "rect") == 0 ||
	   strcmp(name, "rectangle") == 0)      child = new RectOvComp(in, parent);
  
  else if (strcmp(name, "sten") == 0 ||
	   strcmp(name, "stencil") == 0)        child = new StencilOvComp(in, parent);

  else if (strcmp(name, "text") == 0)           child = new TextOvComp(in, parent);

  else if (strcmp(name, "textfile") == 0)       child = new TextFileComp(in, parent);

  else if (strcmp(name, "ovfile") == 0 || 
	   strcmp(name, "drawtool") == 0)         child = new OverlayFileComp(in, parent);
  
  else {
    fprintf(stderr, "unknown graphical object %s\n", name);
    return nil;
  }
  return child;
}

OverlayCatalog* OverlayCatalog::_instance = nil;

OverlayCatalog* OverlayCatalog::Instance() {
  if (!_instance) {
    if (Component::use_unidraw()) 
      _instance = (OverlayCatalog*)unidraw->GetCatalog();
    else
      _instance = new OverlayCatalog("OverlayCatalog", new OverlayCreator());
  }
  return _instance;
}

void OverlayCatalog::Instance(OverlayCatalog* catalog) {
  _instance = catalog;
}

