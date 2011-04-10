/*
 * planar figures
 */

#ifndef figure_h
#define figure_h

#define Fig31Line Line31
#define Fig31Rectangle Rectangle31
#define Fig31Circle Circle31
#define Fig31Ellipse Ellipse31
#define Fig31Open_BSpline Open_BSpline31
#define Fig31Closed_BSpline Closed_BSpline31
#define Fig31Polyline Polyline31
#define Fig31Polygon Polygon31
#define Fig31Text Text31

#include <InterViews/event.h>
#include <InterViews/glyph.h>
#include <InterViews/transformer.h>
#include "globals.h"

class BoxObjList;
class Brush;
class Color;
class Font;
class GraphicList;
class PolyGlyph;
class String;
class ToolState;
class Transformer;

const int buf_size = 10;

class Tool31 {
public:
    enum {
        nop, select, move, scale, stretch, rotate, alter, create
    };

    Tool31(unsigned int = Tool31::nop);
    virtual ~Tool31();

    virtual unsigned int tool();
    virtual void tool(unsigned int);
    
    virtual ToolState& toolstate();
    virtual void toolstate(ToolState*);
    virtual void reset();
protected:
    unsigned int _cur_tool;
    ToolState* _toolstate;
};

class Graphic31 : public Glyph {
public:
    Graphic31(Graphic31* gr = nil);
    virtual ~Graphic31 ();

    virtual void request(Requisition&) const;
    virtual void allocate(Canvas*, const Allocation&, Extension&);
    virtual void draw(Canvas*, const Allocation&) const;
    virtual void drawit(Canvas*);
    virtual void drawclipped(Canvas*, Coord, Coord, Coord, Coord);

    virtual boolean grasp(const Event&, Tool31&);
    virtual boolean manipulating(const Event&, Tool31&);
    virtual boolean effect(const Event&, Tool31&);

    virtual Glyph* clone() const;
    virtual void flush();

    virtual Transformer* transformer();
    virtual void transformer(Transformer*);
    void eqv_transformer(Transformer&);

    virtual void brush(const Brush*);
    virtual const Brush* brush();
    virtual void stroke(const Color*);
    virtual const Color* stroke();
    virtual void fill(const Color*);
    virtual const Color* fill();
    virtual void font(const Font*);
    virtual const Font* font();
    virtual void closed(boolean);
    virtual boolean closed();
    virtual void curved(boolean);
    virtual boolean curved();
    virtual int ctrlpts(Coord*&, Coord*&) const;
    virtual void ctrlpts(Coord*, Coord*, int);
    virtual Graphic31* parent();
    virtual void parent(Graphic31*);
    virtual Graphic31* root();

    void translate(Coord dx, Coord dy);
    void scale(Coord sx, Coord sy, Coord ctrx = 0.0, Coord ctry = 0.0);
    void rotate(float angle, Coord ctrx = 0.0, Coord ctry = 0.0);
    void align(Alignment, Graphic31*, Alignment);

    virtual void recompute_shape();
    virtual void getbounds(Coord&, Coord&, Coord&, Coord&);
    virtual void getcenter(Coord&, Coord&);    
    virtual boolean contains(PointObj&);
    virtual boolean intersects(BoxObj&);

    virtual void undraw();
    virtual void append_(Graphic31*);
    virtual void prepend_(Graphic31*);
    virtual void insert_(GlyphIndex, Graphic31*);
    virtual void remove_(GlyphIndex);
    virtual void replace_(GlyphIndex, Graphic31*);
    virtual void change_(GlyphIndex);

    virtual GlyphIndex count_() const;
    virtual Graphic31* component_(GlyphIndex) const;
    virtual void modified_(GlyphIndex);

    virtual Graphic31* first_containing(PointObj&);
    virtual Graphic31* last_containing(PointObj&);

    virtual Graphic31* first_intersecting(BoxObj&);
    virtual Graphic31* last_intersecting(BoxObj&);

    virtual Graphic31* first_within(BoxObj&);
    virtual Graphic31* last_within(BoxObj&);

    virtual Graphic31& operator = (Graphic31&);

    void get_original(const Coord*&, const Coord*&);
    void add_point (Coord x, Coord y);
    void add_curve (Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2);
    void Bspline_move_to (
        Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );
    void Bspline_curve_to (
        Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );
protected:
    Graphic31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        const Font* font, boolean closed, boolean curved, int coords, 
        Transformer*
    );

    virtual void draw_gs(Canvas*, Graphic31*);
    virtual void drawclipped_gs(
        Canvas*, Coord, Coord, Coord, Coord, Graphic31*
    );
    virtual void getextent_gs(
        Coord&, Coord&, Coord&, Coord& ,float& ,Graphic31* gs
    );
    virtual boolean contains_gs(PointObj&, Graphic31* gs);
    virtual boolean intersects_gs(BoxObj&, Graphic31* gs);
    virtual void getbounds_gs(Coord&, Coord&, Coord&, Coord&, Graphic31* gs);

    virtual void total_gs (Graphic31& gs);
    void parentXform(Transformer& t);
    
    virtual void concat_gs(Graphic31* a, Graphic31* b, Graphic31* dest);
    virtual void concatXform(
        Transformer* a, Transformer* b, Transformer* dest
    );
    virtual void concat(Graphic31* a, Graphic31* b, Graphic31* dest);

/*   Helpers   */

    virtual boolean contains_(Graphic31*, PointObj&, Graphic31* gs);
    virtual boolean intersects_(Graphic31*, BoxObj&, Graphic31* gs);
    virtual void getbounds_(
        Graphic31*, Coord&, Coord&, Coord&, Coord&, Graphic31* gs
    );
    void total_gs_(Graphic31*, Graphic31& gs);
    void concatgs_(Graphic31*, Graphic31*, Graphic31*, Graphic31*);
    void concatXform_(Graphic31*, Transformer*, Transformer*, Transformer*);
    void concat_(Graphic31*, Graphic31*, Graphic31*, Graphic31*);
    void getextent_(Graphic31*, Coord&, Coord&, Coord&, Coord&, float&,Graphic31*);

    void draw_(Graphic31*, Canvas*, Graphic31*);
    void drawclipped_(Graphic31*, Canvas*, Coord, Coord, Coord, Coord, Graphic31*);
    void transform_(Coord, Coord, Coord&, Coord&, Graphic31*);
protected:
    const Brush* _brush;
    const Color* _stroke;
    const Color* _fill;
    const Font* _font;
    Transformer*  _t;

    boolean _closed;
    boolean _curved;
    int _ctrlpts;
    int _buf_size;
    Coord* _x;
    Coord* _y;

    Coord _xmin;
    Coord _xmax;
    Coord _ymin;
    Coord _ymax;
    Graphic31* _parent;
};

class PolyGraphic : public Graphic31 {
public:
    PolyGraphic(Graphic31* = nil) ;
    virtual ~PolyGraphic();
    virtual void request(Requisition&) const;
    virtual void allocate(Canvas*, const Allocation&, Extension&);

    virtual void undraw();

    virtual void append_(Graphic31*);
    virtual void prepend_(Graphic31*);
    virtual void insert_(GlyphIndex, Graphic31*);
    virtual void remove_(GlyphIndex);
    virtual void replace_(GlyphIndex, Graphic31*);
    virtual void change_(GlyphIndex);

    virtual GlyphIndex count_() const;
    virtual Graphic31* component_(GlyphIndex) const;

    virtual void modified_(GlyphIndex);
    virtual void flush();
    virtual Glyph* clone() const;

    virtual Graphic31* first_containing(PointObj&);
    virtual Graphic31* last_containing(PointObj&);

    virtual Graphic31* first_intersecting(BoxObj&);
    virtual Graphic31* last_intersecting(BoxObj&);

    virtual Graphic31* first_within(BoxObj&);
    virtual Graphic31* last_within(BoxObj&);
protected:
    virtual void draw_gs(Canvas*, Graphic31*);
    virtual void drawclipped_gs(
        Canvas*, Coord, Coord, Coord, Coord, Graphic31*
    );
    virtual boolean contains_gs(PointObj&, Graphic31*);
    virtual boolean intersects_gs(BoxObj&, Graphic31*);
    virtual void getextent_gs(
        Coord&, Coord&, Coord&, Coord&, float&, Graphic31* gs
    );

protected:
    PolyGlyph* _body;
};

class GraphicMaster : public PolyGraphic {
public:
    GraphicMaster(Graphic31* = nil, const Color* bg = nil);
    virtual ~GraphicMaster();

    void background(const Color*);
    const Color* background();

    virtual void request(Requisition&) const;
    virtual void allocate(Canvas*, const Allocation&, Extension&);
    virtual boolean grasp(const Event&, Tool31&);
    virtual boolean manipulating(const Event&, Tool31&);
    virtual boolean effect(const Event&, Tool31&);
    virtual Glyph* clone() const;
protected:
    virtual void drawclipped_gs(
        Canvas*, Coord, Coord, Coord, Coord, Graphic31*
    );
protected:
    GraphicList* _gr_list;
    const Color* _bg;
    Allocation _a;
};
inline const Color* GraphicMaster::background () { return _bg; }

class Line31 : public Graphic31 {
public:
    Line31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord x1, Coord y1, Coord x2, Coord y2, Transformer* t = nil
    );
    virtual Glyph* clone() const;
    
protected:
    virtual ~Line31 ();
};

class Rectangle31 : public Graphic31 {
public:
    Rectangle31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord l, Coord b, Coord r, Coord t, Transformer* t = nil
    );
    virtual Glyph* clone() const;

protected:
    virtual ~Rectangle31 ();
};

class Circle31 : public Graphic31 {
public:
    Circle31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord x, Coord y, Coord r, Transformer* t = nil
    );
    virtual Glyph* clone() const;

protected:
    virtual ~Circle31 ();
};

class Ellipse31 : public Graphic31 {
public:
    Ellipse31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord x, Coord y, Coord rx, Coord ry, Transformer* t = nil
    );
    virtual Glyph* clone() const;

protected:
    virtual ~Ellipse31 ();
};

class Open_BSpline31 : public Graphic31 {
public:
    Open_BSpline31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int ctrlpts, Transformer* t = nil
    );
    virtual Glyph* clone() const;

protected:
    Open_BSpline31(Open_BSpline31*);
    virtual ~Open_BSpline31 ();
};

class Closed_BSpline31 : public Graphic31 {
public:
    Closed_BSpline31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int ctrlpts, Transformer* t = nil
    );
    virtual Glyph* clone() const;

protected:
    Closed_BSpline31(Closed_BSpline31*);
    virtual ~Closed_BSpline31 ();
};

class Polyline31 : public Graphic31 {
public:
    Polyline31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int ctrlpts, Transformer* t = nil
    );
    virtual Glyph* clone() const;

protected:
    virtual ~Polyline31 ();
};

class Polygon31 : public Graphic31 {
public:
    Polygon31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int ctrlpts, Transformer* t = nil
    );
    virtual Glyph* clone() const;

protected:
    virtual ~Polygon31 ();
};

class Text31 : public Graphic31 {
public:
    Text31 (
        const Font* font, const Color* stroke, const char*, Transformer* t = nil
    );
    virtual void text(const char*);
    virtual const char* text();
    virtual void draw(Canvas*, const Allocation&) const;
    virtual void request(Requisition&) const;
    virtual void allocate(Canvas*, const Allocation&, Extension&);
    virtual Glyph* clone() const;

protected:
    virtual ~Text31();
    
    virtual void getextent_gs (Coord&, Coord&, Coord&, Coord&,float&,Graphic31*);
    virtual void draw_gs(Canvas*, Graphic31*);

    void init();
protected:
    String* _text;
    Allocation _a;
    PolyGlyph* _body;
};

class ToolState {
public:
    Event _init;
    Event _last;
    Coord _l, _b, _r, _t;
    Graphic31 _gs;
};

#endif
