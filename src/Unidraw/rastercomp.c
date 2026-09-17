/*
 * Copyright (c) 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * RasterComp definitions.
 */

#include <Unidraw/catalog.h>
#include <Unidraw/classes.h>
#include <Unidraw/unidraw.h>

#include <Unidraw/Components/rastercomp.h>

#include <Unidraw/Graphic/rasterrect.h>

#include <InterViews/raster.h>
#include <InterViews/transformer.h>

#include <OS/math.h>

#include <IV-2_6/_enter.h>

#include <stream.h>
#include <string.h>

/*****************************************************************************/

static const int color_depth = 8;               // bits per color in PostScript

static char hexcharmap[] = {
     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
     'a', 'b', 'c', 'd', 'e', 'f'
};

/*****************************************************************************/

ClassId RasterComp::GetClassId () { return RASTER_COMP; }

boolean RasterComp::IsA (ClassId id) {
    return RASTER_COMP == id || GraphicComp::IsA(id);
}

Component* RasterComp::Copy () {
    return new RasterComp((RasterRect*) GetGraphic()->Copy(), _filename);
}

RasterComp::RasterComp (RasterRect* s, const char* filename) : GraphicComp(s) {
    _filename = (filename == nil) ? nil : strdup(filename);
}

RasterComp::~RasterComp () { delete _filename; }

RasterRect* RasterComp::GetRasterRect () { return (RasterRect*) GetGraphic(); }
const char* RasterComp::GetFileName () { return _filename; }

void RasterComp::Read (istream& in) {
    GraphicComp::Read(in);
    Raster* raster = ReadRaster(in);
    RasterRect* rr = new RasterRect(raster);

    Transformer* t = ReadTransformer(in);
    rr->SetTransformer(t);
    Unref(t);

    SetGraphic(rr);
    _filename = ReadString(in);
}

void RasterComp::Write (ostream& out) {
    GraphicComp::Write(out);
    RasterRect* rr = GetRasterRect();
    Raster* raster = rr->GetOriginal();

    WriteRaster(raster, out);
    WriteTransformer(rr->GetTransformer(), out);
    WriteString(_filename, out);
}

/*****************************************************************************/

RasterComp* RasterView::GetRasterComp () {
    return (RasterComp*) GetSubject();
}

ClassId RasterView::GetClassId () { return RASTER_VIEW; }

boolean RasterView::IsA (ClassId id) {
    return RASTER_VIEW == id || GraphicView::IsA(id);
}

RasterView::RasterView (RasterComp* subj) : GraphicView(subj) { }

void RasterView::Update () {
    Graphic* raster = GetGraphic();

    IncurDamage(raster);
    *raster = *GetRasterComp()->GetGraphic();
    IncurDamage(raster);
    EraseHandles();
}

Graphic* RasterView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();
    
    if (graphic == nil) {
        RasterRect* rr = GetRasterComp()->GetRasterRect();
        graphic = new RasterRect(rr->GetOriginal(), rr);
        SetGraphic(graphic);
    }
    return graphic;
}

/*****************************************************************************/

PSRaster::PSRaster (RasterComp* subj) : PostScriptView(subj) { }
ClassId PSRaster::GetClassId () { return PS_RASTER; }

boolean PSRaster::IsA (ClassId id) { 
    return PS_RASTER == id || PostScriptView::IsA(id);
}

boolean PSRaster::Definition (ostream& out) {
    RasterComp* comp = (RasterComp*) GetSubject();
    Raster* raster = comp->GetRasterRect()->GetOriginal();
    Coord w = raster->Width();
    Coord h = raster->Height();

    out << "Begin " << MARK << " " << "ColorRast\n";
    Transformation(out);

    Catalog* catalog = unidraw->GetCatalog();
    catalog->Mark(out);
    out << w << " " << h << "\n";

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

    catalog->Mark(out);

    ColorIntensity r, g, b;
    float alpha;
    int count = 0;
    for (int j = h-1; j>=0; --j) {
	for (int i=0; i<w; ++i) {
	    raster->peek(i, j, r, g, b, alpha);
	    int ir = Math::round(r*255);
	    out << hexcharmap[ir/16] << hexcharmap[ir%16];
	    if (++count%40 == 0) out << "\n";
	}
	for (int i=0; i<w; ++i) {
	    raster->peek(i, j, r, g, b, alpha);
	    int ig = Math::round(g*255);
	    out << hexcharmap[ig/16] << hexcharmap[ig%16];
	    if (++count%40 == 0) out << "\n";
	}
	for (int i=0; i<w; ++i) {
	    raster->peek(i, j, r, g, b, alpha);
	    int ib = Math::round(b*255);
	    out << hexcharmap[ib/16] << hexcharmap[ib%16];
	    if (++count%40 == 0) out << "\n";
	}
    }

    out << "\nEnd\n\n";

    return out.good();
}
