/*
 * planar figures
 */

#include "figure.h"
#include "grabber.bm"
#include "grabberMask.bm"
#include <InterViews/bitmap.h>
#include <InterViews/brush.h>
#include <InterViews/canvas.h>
#include <InterViews/character.h>
#include <InterViews/color.h>
#include <InterViews/cursor.h>
#include <InterViews/display.h>
#include <InterViews/font.h>
#include <InterViews/handler.h>
#include <InterViews/layout.h>
#include <InterViews/tformsetter.h>
#include <InterViews/transformer.h>
#include <InterViews/window.h>
#include <IV-X11/xcanvas.h>
#include <OS/list.h>
#include <OS/math.h>
#include <OS/memory.h>
#include <OS/string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

static void corners(
    Coord& left, Coord& bottom, Coord& right, Coord& top, const Transformer& t
) {
    Coord x1, y1, x2, y2, x3, y3, x4, y4;
    
    t.transform(left, bottom, x1, y1);
    t.transform(left, top, x2, y2);
    t.transform(right, top, x3, y3);
    t.transform(right, bottom, x4, y4);
    left = Math::min(x1, x2, x3, x4);
    bottom = Math::min(y1, y2, y3, y4);
    right = Math::max(x1, x2, x3, x4);
    top = Math::max(y1, y2, y3, y4);
}    

Graphic31::Graphic31 (Graphic31* gr) {
    if (gr != nil) {
        _brush = gr->brush();
        Resource::ref(_brush);
        _stroke = gr->stroke();
        Resource::ref(_stroke);
        _fill = gr->fill();
        Resource::ref(_fill);
        _font = gr->font();
        Resource::ref(_font);
        _closed = gr->closed();
        _curved = gr->curved();
        Coord* foo;
        
        _ctrlpts = gr->ctrlpts(foo, foo);
        _x = new Coord[buf_size];
        _y = new Coord[buf_size];
        _buf_size = buf_size;
        if (gr->transformer() != nil) {
            _t = new Transformer;
            *_t = *gr->transformer();
        } else {
            _t = nil;
        }
    } else {
        _brush = nil;
        _stroke = nil;
        _fill = nil;
        _font = nil;
        _closed = false;
        _curved = false;
        _ctrlpts = 0;
        _x = new Coord[buf_size];
        _y = new Coord[buf_size];
        _buf_size = buf_size;
        _t = nil;
    } 
    _xmin = _xmax = _ymin = _ymax = 0.0;
    _parent = nil;
}

Graphic31::Graphic31 (
    const Brush* brush, const Color* stroke, const Color* fill,
    const Font* font, boolean closed, boolean curved, int coords, 
    Transformer* t
) {
    _brush = brush;
    Resource::ref(_brush);
    _stroke = stroke;
    Resource::ref(_stroke);
    _fill = fill;
    Resource::ref(_fill);
    _font = font;
    Resource::ref(_font);
    _closed = closed;
    _curved = curved;
    _ctrlpts = 0;
    if (coords > 0) {
        _x = new Coord[coords];
        _y = new Coord[coords];
    } else {
        _x = nil;
        _y = nil;
    }
    _buf_size = coords;
    _t = nil;
    if (t != nil) {
        _t = new Transformer;
        *_t = *t;
    }
    _parent = nil;
}

Graphic31::~Graphic31 () {
    Resource::unref(_brush);
    Resource::unref(_stroke);
    Resource::unref(_fill);
    Resource::unref(_font);
    Resource::unref(_t);
    delete _x;
    delete _y;
}

int Graphic31::ctrlpts (Coord*& x, Coord*& y) const{
    x = _x;
    y = _y;
    return _ctrlpts;
}

void Graphic31::ctrlpts (Coord* x, Coord* y, int count) {
    delete _x;
    delete _y;
    _ctrlpts = max(count+1, buf_size);
    _x = new Coord[_ctrlpts];
    _y = new Coord[_ctrlpts];
    for (int i = 0; i < count; i++) {
        _x[i] = x[i];
        _y[i] = y[i];
    }
}

boolean Graphic31::grasp (const Event& e, Tool31& tool) {
    ToolState& ts = tool.toolstate();
    ts._init = e;
    ts._last = e;
    getbounds(ts._l, ts._b, ts._r, ts._t);
    if (tool.tool() == Tool31::scale) {
        Coord lx, ly;
        Coord cx, cy;
        lx = e.pointer_x(); 
        ly = e.pointer_y();
        cx = (ts._l + ts._r)/2.0;
        cy = (ts._b + ts._t)/2.0;
                    
        scale(
            Math::abs((lx-cx)*2.0/(ts._r-ts._l)), 
            Math::abs((ly-cy)*2.0/(ts._t-ts._b)),
            cx, cy
        );
    }

    return true;
}

boolean Graphic31::manipulating (const Event& e, Tool31& tool) {
    if (e.type() == Event::up) {
        return false;
    } else if (e.type() == Event::motion) {
        unsigned int tool_type = tool.tool();
        ToolState& ts = tool.toolstate();

        if (tool_type != Tool31::nop) {
            float pi = 3.14159;
            Graphic31 gs;
            Transformer* tx = ts._gs.transformer();
            
            Coord x, y, lx, ly;
            
            x = ts._last.pointer_x();
            y = ts._last.pointer_y();
            lx = e.pointer_x(); 
            ly = e.pointer_y();
            
            switch(tool_type) {
            case Tool31::select:
                break;
            case Tool31::move:
                {
                    if (tx != nil) {
                        tx->inverse_transform(lx, ly);
                        tx->inverse_transform(x, y);
                    }
                    translate(lx-x, ly-y);
                    ts._last = e;
                }
                break;
            case Tool31::scale:
                {
                    Coord cx, cy;
                    cx = (ts._l + ts._r)/2.0;
                    cy = (ts._b + ts._t)/2.0;
                    
                    scale((lx-cx)/(x-cx), (ly-cy)/(y-cy), cx, cy);
                    ts._last = e;
                } 
                break;
            case Tool31::rotate:
                {
                    Coord cx, cy;
                    cx = (ts._l + ts._r)/2.0;
                    cy = (ts._b + ts._t)/2.0;

                    float ldy = ly-cy; float ldx = lx-cx;
                    float dy = y-cy; float dx = x-cx;

		    float cur = degrees(atan2f(ldy, ldx));
		    float last = degrees(atan2f(dy, dx));
                        
                    rotate(cur-last, cx, cy);
                    ts._last = e;
                }
                break;
            }
        }
    } 
    return true;
}

boolean Graphic31::effect (const Event&, Tool31&) {
    return true;
}

void Graphic31::flush () {}

Glyph* Graphic31::clone () const { return nil; }

Transformer* Graphic31::transformer() { return _t; }

/*   To be replaced by templates  */

void Graphic31::transformer(Transformer* t) {
    if (t != nil) {
        Resource::ref(t);
    }
    if (_t != nil) {
        Resource::unref(_t);
    }
    _t = t;
}

const Brush* Graphic31::brush() { return _brush; }

void Graphic31::brush(const Brush* b) { 
    if (b != nil) {
        Resource::ref(b);
    }
    if (_brush != nil) {
        Resource::unref(_brush);
    }
    _brush = b;
}

const Color* Graphic31::stroke() { return _stroke; }

void Graphic31::stroke(const Color* s) {
    if (s != nil) {
        Resource::ref(s);
    }
    if (_stroke != nil) {
        Resource::unref(_stroke);
    }
    _stroke = s;
}

const Color* Graphic31::fill() { return _fill; }

void Graphic31::fill(const Color* f) {
    if (f != nil) {
        Resource::ref(f);
    }
    if (_fill != nil) {
        Resource::unref(_fill);
    }
    _fill = f;
}

const Font* Graphic31::font() { return _font; }

void Graphic31::font(const Font* f) {
    if (f != nil) {
        Resource::ref(f);
    }
    if (_font != nil) {
        Resource::unref(_font);
    }
    _font = f;
}

void Graphic31::closed (boolean c) { _closed = c; }

boolean Graphic31::closed () { return _closed; }

void Graphic31::curved (boolean c) { _curved = c; }

boolean Graphic31::curved () { return _curved; }

void Graphic31::parent (Graphic31* p) { _parent = p; }

Graphic31* Graphic31::parent () { return _parent; }

Graphic31* Graphic31::root () {
    Graphic31* cur, *parent = this;

    do {
        cur = parent;
        parent = cur->parent();
    } while (parent != nil);

    return cur;
}

void Graphic31::total_gs (Graphic31& gs) {
    Graphic31* p = parent();

    if (p == nil) {
        concat(nil, this, &gs);

    } else {
        p->total_gs(gs);
        concat(this, &gs, &gs);
    }
}

void Graphic31::translate (float dx, float dy) { 
    if (dx != 0 || dy != 0) {
	if (_t == nil) {
	    _t = new Transformer;
	}
	_t->translate(dx, dy);
    }
}

void Graphic31::scale (float sx, float sy, float cx, float cy) {
    float ncx, ncy;

    if (sx != 1.0 || sy != 1.0) {
	if (_t == nil) {
	    _t = new Transformer;
	}
	Transformer parents;
	parentXform(parents);
	parents.InvTransform(cx, cy, ncx, ncy);
	
	if (ncx != 0 || ncy != 0) {
	    _t->translate(-ncx, -ncy);
	    _t->scale(sx, sy);
	    _t->translate(ncx, ncy);
	} else {
	    _t->scale(sx, sy);
	}
    }
}

void Graphic31::rotate (float angle, float cx, float cy) {
    float mag = (angle < 0) ? -angle : angle;
    float ncx, ncy;

    if ((mag - int(mag)) != 0 || int(mag)%360 != 0) {
	if (_t == nil) {
	    _t = new Transformer;
	}
	Transformer parents;
	parentXform(parents);
	parents.InvTransform(cx, cy, ncx, ncy);
	
	if (ncx != 0 || ncy != 0) {
	    _t->translate(-ncx, -ncy);
	    _t->rotate(angle);
	    _t->translate(ncx, ncy);
	} else {
	    _t->rotate(angle);
	}
    }
}

void Graphic31::align (Alignment falign, Graphic31* moved, Alignment malign) {
    float fx0, fy0, fx1, fy1, mx0, my0, mx1, my1, dx = 0, dy = 0;

    getbounds(fx0, fy0, fx1, fy1);
    moved->getbounds(mx0, my0, mx1, my1);
    
    switch (falign) {
	case BottomLeft:
	case CenterLeft:
	case TopLeft:
	case Left:
	    dx = fx0;
	    break;
	case BottomCenter:
	case Center:
	case TopCenter:
	case HorizCenter:
	    dx = (fx0 + fx1 + 1)/2;
	    break;
	case BottomRight:
	case CenterRight:
	case TopRight:
	case Right:
	    dx = fx1 + 1;
	    break;
    }
    switch (falign) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	case Bottom:
	    dy = fy0;
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	case VertCenter:
	    dy = (fy0 + fy1 + 1)/2;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	case Top:
	    dy = fy1 + 1;
	    break;
    }
    
    switch (malign) {
	case BottomLeft:
	case CenterLeft:
	case TopLeft:
	case Left:
	    dx -= mx0;
	    break;	
	case BottomCenter:
	case Center:
	case TopCenter:
	case HorizCenter:
	    dx -= (mx0 + mx1 + 1)/2;
	    break;
	case BottomRight:
	case CenterRight:
	case TopRight:
	case Right:
	    dx -= (mx1 + 1);
	    break;
    }
    switch (malign) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	case Bottom:
	    dy -= my0;
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	case VertCenter:
	    dy -= (my0 + my1 + 1)/2;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	case Top:
	    dy -= (my1 + 1);
	    break;
    }
    if (dx != 0 || dy != 0) {
        Transformer parents;
	moved->parentXform(parents);

        parents.Invert();
	parents.Transform(0.0, 0.0, fx0, fy0);
	parents.Transform(dx, dy, mx0, my0);

	moved->translate(mx0-fx0, my0-fy0);
    }
}

void Graphic31::recompute_shape () {
    if (_ctrlpts == 0) {
        return;
    } else {
        _xmin = _xmax = _x[0];
        _ymin = _ymax = _y[0];
    }
    for (int i = 1; i < _ctrlpts; i++) {
        _xmin = Math::min(_xmin, _x[i]);
        _xmax = Math::max(_xmax, _x[i]);
        _ymin = Math::min(_ymin, _y[i]);
        _ymax = Math::max(_ymax, _y[i]);
    }
}

void Graphic31::getbounds (float& x0, float& y0, float& x1, float& y1) {
    Graphic31 gs;
    
    total_gs(gs);
    getbounds_gs(x0, y0, x1, y1, &gs);
}

void Graphic31::getcenter (float& x, float& y) {
    Graphic31 gs;
    float l, b, tol;
    
    total_gs(gs);
    getextent_gs(l, b, x, y, tol, &gs);
}    

boolean Graphic31::contains (PointObj& p) {
    if (parent() == nil) {
        return contains_gs(p, this);

    } else {
	Graphic31 gs;
	total_gs(gs);
	return contains_gs(p, &gs);
    }
}

boolean Graphic31::intersects (BoxObj& b) {
    if (parent() == nil) {
        return intersects_gs(b, this);

    } else {
	Graphic31 gs;
	total_gs(gs);
	return intersects_gs(b, &gs);
    }
}

void Graphic31::undraw () {}
void Graphic31::append_(Graphic31*) {}
void Graphic31::prepend_(Graphic31*) {}
void Graphic31::insert_(GlyphIndex, Graphic31*) {}
void Graphic31::remove_(GlyphIndex) {}
void Graphic31::replace_(GlyphIndex, Graphic31*) {}
void Graphic31::change_(GlyphIndex) {}
GlyphIndex Graphic31::count_ () const { return 0; }
Graphic31* Graphic31::component_ (GlyphIndex) const { return nil; }
void Graphic31::modified_ (GlyphIndex) {}

Graphic31* Graphic31::first_containing (PointObj&) { return nil; }
Graphic31* Graphic31::last_containing (PointObj&) { return nil; }
Graphic31* Graphic31::first_intersecting (BoxObj&) { return nil; }
Graphic31* Graphic31::last_intersecting (BoxObj&) { return nil; }
Graphic31* Graphic31::first_within (BoxObj&) { return nil; }
Graphic31* Graphic31::last_within (BoxObj&) { return nil; }

Graphic31& Graphic31::operator = (Graphic31& g) {
    brush(g.brush());
    stroke(g.stroke());
    fill(g.fill());
    font(g.font());

    if (g._t == nil) {
        Resource::unref(_t);
        _t = nil;

    } else {
	if (_t == nil) {
	    _t = new Transformer(g._t);
	} else {
	    *_t = *g._t;
	}
    }
    return *this;
}

void Graphic31::get_original (const Coord*& x, const Coord*& y) {
    x = _x;
    y = _y;
}

void Graphic31::add_point(Coord x, Coord y) {
    if (_ctrlpts == 0) {
        _xmin = x - 1;
        _xmax = x + 1;
        _ymin = y - 1;
        _ymax = y + 1;
    } else {
        _xmin = Math::min(_xmin, x);
        _xmax = Math::max(_xmax, x);
        _ymin = Math::min(_ymin, y);
        _ymax = Math::max(_ymax, y);
    }
    _x[_ctrlpts] = x;
    _y[_ctrlpts] = y;
    _ctrlpts += 1;
    if (_ctrlpts >= _buf_size) {
        _buf_size = buf_size + _buf_size;
        Coord* x = new Coord[_buf_size];
        Coord* y = new Coord[_buf_size];
        Memory::copy(_x, x, _ctrlpts*sizeof(int));
        Memory::copy(_y, y, _ctrlpts*sizeof(int));
        delete _x;
        delete _y;
        _x = x;
        _y = y;
    }
}

void Graphic31::add_curve(
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {
    add_point(x1, y1);
    add_point(x2, y2);
    add_point(x, y);
}

void Graphic31::Bspline_move_to (
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {
    Coord p1x = (x + x + x1) / 3;
    Coord p1y = (y + y + y1) / 3;
    Coord p2x = (x + x + x2) / 3;
    Coord p2y = (y + y + y2) / 3;
    add_point((p1x + p2x) / 2, (p1y + p2y) / 2);
}

void Graphic31::Bspline_curve_to (
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {
    Coord p1x = (x + x + x1) / 3;
    Coord p1y = (y + y + y1) / 3;
    Coord p2x = (x + x + x2) / 3;
    Coord p2y = (y + y + y2) / 3;
    Coord p3x = (x1 + x1 + x) / 3;
    Coord p3y = (y1 + y1 + y) / 3;
    add_curve((p1x + p2x) / 2, (p1y + p2y) / 2, p3x, p3y, p1x, p1y);
}

void Graphic31::request(Requisition& req) const {
    if (_ctrlpts > 0) {
        Coord left = _xmin, bottom = _ymin;
        Coord right = _xmax, top = _ymax;
        Requirement& rx = req.x_requirement();
        Requirement& ry = req.y_requirement();

        if (_t != nil) {
            corners(left, bottom, right, top, *_t);
        } 
        
        rx.natural(right - left + 1);
        rx.stretch(0.0);
        rx.shrink(0.0);
        rx.alignment(-left / rx.natural());
        
        ry.natural(top - bottom + 1);
        ry.stretch(0.0);
        ry.shrink(0.0);
        ry.alignment(-bottom / ry.natural());
    }
}

void Graphic31::allocate(Canvas* c, const Allocation& a, Extension& ext) {
    if (_ctrlpts > 0) {
        Coord w = _brush == nil ? 0 : _brush->width();
        Coord x = a.x();
        Coord y = a.y();
        ext.merge_xy(
            c, x + _xmin - w, x + _xmax + w,
            y + _ymin - w, y + _ymax + w
        );
    }
}

void Graphic31::draw(Canvas* c, const Allocation& a) const {
    if (c != nil) {
        boolean no_transformer = !_t;
        if (no_transformer) {
	  ((Graphic31*)this)->_t = new Transformer();
	  _t->Translate(a.x(), a.y());
	}
	Graphic31* gr = (Graphic31*) this;
        CanvasDamage& cd = c->rep()->damage_;
        gr->drawclipped(
            c, cd.left, cd.bottom, cd.right, cd.top
        );
	if (no_transformer) {
	  _t->Translate(-a.x(), -a.y());
	  delete _t;
	  ((Graphic31*)this)->_t = nil;
	}
    }
}

void Graphic31::drawit (Canvas* c) {
    if (parent() == nil) {
	draw_gs(c, this);

    } else {
	Graphic31 gs;
	total_gs(gs);
	draw_gs(c, &gs);
    }
}    

void Graphic31::drawclipped (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    if (parent() == nil) {
        drawclipped_gs(c, l, b, r, t, this);
    } else {
        Graphic31 gs;
        total_gs(gs);
        drawclipped_gs(c, l, b, r, t, &gs);
    }
}

void Graphic31::draw_gs (Canvas* c, Graphic31* gs) {
    const Brush* brush = gs->brush();
    const Color* stroke = gs->stroke();
    const Color* fill = gs->fill();
    Transformer* tx = gs->transformer();
    if (tx != nil) {
        c->push_transform();
        c->transform(*tx);
    }
    c->new_path();
    c->move_to(_x[0], _y[0]);
    if (_curved) {
        for (int i = 1; i < _ctrlpts; i += 3) {
            c->curve_to(
                _x[i + 2], _y[i + 2],
                _x[i], _y[i],
                _x[i + 1], _y[i + 1]
            );
        }
    } else {
        for (int i = 1; i < _ctrlpts; ++i) {
            c->line_to(_x[i], _y[i]);
        }
    }
    if (_closed) {
        c->close_path();
    }
    if (fill != nil) {
        c->fill(fill);
    }
    if (brush != nil && stroke != nil) {
        c->stroke(stroke, brush);
    }
    if (tx != nil) {
        c->pop_transform();
    }
}

void Graphic31::drawclipped_gs (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic31* gs
) {
    Coord ll, bb, rr, tt;
    getbounds_gs(ll, bb, rr, tt, gs);

    BoxObj thisBox(ll, bb, rr, tt);
    BoxObj clipBox(l, b, r, t);
    if (clipBox.Intersects(thisBox)) {
	draw_gs(c, gs);
    }
}

void Graphic31::eqv_transformer (Transformer& total) {
    Graphic31* p = parent();
    
    if (p == nil) {
        concatXform(nil, _t, &total);

    } else {
        p->eqv_transformer(total);
        concatXform(_t, &total, &total);
    }
}

void Graphic31::getextent_gs (
    Coord& l, Coord& b, Coord& cx, Coord& cy, float& tol, Graphic31* gs
) {
    tol = 1.0;
    const Brush* br = gs->brush();
    if (br != nil) {
        float width = float(br->Width());
        tol = (width > 1) ? width : tol;
    }
    if (_ctrlpts > 0) {
        Coord left = _xmin, bottom = _ymin;
        Coord right = _xmax, top = _ymax;
        Transformer* t = gs->transformer();

        if (t != nil) {
            corners(left, bottom, right, top, *t);
        } 
        l = left;
        b = bottom;
        cx = (left + right)/2.0;
        cy = (top + bottom)/2.0;
    }
}

void Graphic31::getbounds_gs (
    Coord& l, Coord& b, Coord& r, Coord& t, Graphic31* gs
) {
    float tol;
    
    getextent_gs(l, b, r, t, tol, gs);
    r += r - l;
    t += t - b;
    l -= tol;
    b -= tol;
    r += tol;
    t += tol;
}

static void invXform_gs (Coord& tx, Coord& ty, Graphic31* g) {
    Transformer* t = g->transformer();
    if (t != nil) {
        t->inverse_transform(tx, ty);
    }
}

static void Xform_gs(
    Coord x[], Coord y[], int n, Coord tx[], Coord ty[], Graphic31* g
) {
    Transformer* t = g->transformer();
    if (t != nil) {
        register Coord* ox, * oy, *nx, *ny;
        Coord* lim;
        
        lim = &x[n];
        for (
            ox = x, oy = y, nx = tx, ny = ty; ox < lim; ox++, oy++, nx++, ny++
        ) {
            t->transform(*ox, *oy, *nx, *ny);
        }
    } else {
        Memory::copy(x, tx, n*sizeof(Coord));
        Memory::copy(y, ty, n*sizeof(Coord));
    }
}

boolean Graphic31::contains_gs (PointObj& po, Graphic31* gs) { 
    Coord ll, bb, rr, tt;
    PointObj pt (&po);
    getbounds_gs(ll, bb, rr, tt, gs);
    BoxObj b(ll, bb, rr, tt);;

    if (!_curved && !_fill) {
        if (b.Contains(po)) {
            MultiLineObj ml (_x, _y, _ctrlpts);
            invXform_gs(pt._x, pt._y, gs);
            if (_closed) {
                LineObj l (_x[_ctrlpts - 1], _y[_ctrlpts - 1], *_x, *_y);
                return ml.Contains(pt) || l.Contains(pt);
            } else {
                return ml.Contains(pt);
            }
        }
        return false;

    } else if (!_curved && _fill) {
        if (b.Contains(pt)) {
            FillPolygonObj fp (_x, _y, _ctrlpts);
            invXform_gs(pt._x, pt._y, gs);
            return fp.Contains(pt);
        }
        return false;

    } else if (_curved && !_fill) {
        if (b.Contains(pt)) {
            MultiLineObj ml;
            if (_closed) {
                ml.ClosedSplineToPolygon(_x, _y, _ctrlpts);
            } else {
                ml.SplineToMultiLine(_x, _y, _ctrlpts);
            }
            invXform_gs(pt._x, pt._y, gs);
            return ml.Contains(pt);
        }
        return false;

    } else {
        if (b.Contains(pt)) {
            FillPolygonObj fp;
            fp.ClosedSplineToPolygon(_x, _y, _ctrlpts);
            invXform_gs(pt._x, pt._y, gs);
            return fp.Contains(pt);
        }
        return false;
    }
}

boolean Graphic31::intersects_gs (BoxObj& userb, Graphic31* gs) { 
    Coord* convx, *convy;
    Coord ll, bb, rr, tt;
    getbounds_gs(ll, bb, rr, tt, gs);
    BoxObj b(ll, bb, rr, tt);;
    boolean result = false;

    if (!_curved && !_fill) {
        if (b.Intersects(userb)) {
            convx = new Coord[_ctrlpts+1];
            convy = new Coord[_ctrlpts+1];
            Xform_gs(_x, _y, _ctrlpts, convx, convy, gs);
            if (_closed) {
                convx[_ctrlpts] = *convx;
                convy[_ctrlpts] = *convy;
                MultiLineObj ml(convx, convy, _ctrlpts+1);
                result = ml.Intersects(userb);
            } else {
                MultiLineObj ml(convx, convy, _ctrlpts);
                result = ml.Intersects(userb);
            }
            delete convx;
            delete convy;
        }
        return result;

    } else if (!_curved && _fill) {
        if (b.Intersects(userb)) {
            convx = new Coord[_ctrlpts];
            convy = new Coord[_ctrlpts];
            Xform_gs(_x, _y, _ctrlpts, convx, convy, gs);
            FillPolygonObj fp (convx, convy, _ctrlpts);
            result = fp.Intersects(userb);
            delete convx;
            delete convy;
        }
        return result;    

    } else if (_curved && !_fill) {
        if (b.Intersects(userb)) {
            convx = new Coord[_ctrlpts];
            convy = new Coord[_ctrlpts];
            Xform_gs(_x, _y, _ctrlpts, convx, convy, gs);
            MultiLineObj ml;
            if (_closed) {
                ml.ClosedSplineToPolygon(convx, convy, _ctrlpts);
            } else {
                ml.SplineToMultiLine(convx, convy, _ctrlpts);
            }
            result = ml.Intersects(userb);
            delete convx;
            delete convy;
        }
        return result;

    } else {
        if (b.Intersects(userb)) {
            convx = new Coord[_ctrlpts];
            convy = new Coord[_ctrlpts];
            Xform_gs(_x, _y, _ctrlpts, convx, convy, gs);
            FillPolygonObj fp;
            fp.ClosedSplineToPolygon(convx, convy, _ctrlpts);
            result = fp.Intersects(userb);
            delete convx;
            delete convy;
        }
        return result;
    }
}

void Graphic31::parentXform (Transformer& t) {
    Transformer identity;
    Graphic31* p = parent();

    if (p == nil) {
        *(&t) = *(&identity);
    } else {
        p->eqv_transformer(t);
    }
}

void Graphic31::concat_gs (Graphic31* a, Graphic31* b, Graphic31* dest) {
    const Color* stroke, *fill;
    const Font* font;
    const Brush* br;
    
    if (a == nil) {
        *dest = *b;
        return;
    } else if (b == nil) {
        *dest = *a;
        return;
    }

    if ((fill = b->fill()) == nil) {
	fill = a->fill();
    }
    dest->fill(fill);

    if ((stroke = b->stroke()) == nil) {
	stroke = a->stroke();
    }
    dest->stroke(stroke);

    if ((font = b->font()) == nil) {
	font = a->font();
    }
    dest->font(font);

    if ((br = b->brush()) == nil) {
	br = a->brush();
    }
    dest->brush(br);
}

void Graphic31::concatXform (
    Transformer* a, Transformer* b, Transformer* dest
) {
    Transformer identity;
    if (a == nil) {
        *dest = (b == nil) ? *(&identity) : *b;

    } else if (b == nil) {
        *dest = *a;
        
    } else {
        Transformer tmp(a);
        tmp.Postmultiply(b);
        *dest = tmp;
    }
}

void Graphic31::concat (Graphic31* a, Graphic31* b, Graphic31* dest) {
    Transformer* ta = (a == nil) ? nil : a->transformer();
    Transformer* tb = (b == nil) ? nil : b->transformer();
    Transformer* td = dest->transformer();
    if (td == nil) {
        td = new Transformer;
    } else {
        Resource::ref(td);
    }
    concatXform(ta, tb, td);
    dest->transformer(td);
    concat_gs(a, b, dest);
    Resource::unref(td);
}

boolean Graphic31::contains_ (Graphic31* g, PointObj& p, Graphic31* gs) {
    return g->contains_gs(p, gs);
}
boolean Graphic31::intersects_ (Graphic31* g, BoxObj& b, Graphic31* gs) {
    return g->intersects_gs(b, gs);
}
void Graphic31::getbounds_ (
    Graphic31* g, Coord& l, Coord& b, Coord& r, Coord& t, Graphic31* gs
) {
    g->getbounds_gs(l, b, r, t, gs);
}
void Graphic31::total_gs_(Graphic31* gr, Graphic31& gs) {
    gr->total_gs(gs);
}
void Graphic31::concatgs_ (Graphic31* gr, Graphic31* a, Graphic31* b, Graphic31* dest) {
    gr->concat_gs(a, b, dest);
}
void Graphic31::concatXform_ (
    Graphic31* gr, Transformer* a, Transformer* b, Transformer* dest
) {
    gr->concatXform(a, b, dest);
}
void Graphic31::concat_ (Graphic31* g, Graphic31* a, Graphic31* b, Graphic31* d) {
    g->concat(a, b, d);
}
void Graphic31::getextent_ (
    Graphic31* gr,Coord& l, Coord& b, Coord& r, Coord& t, float& tol, Graphic31* gs
) {
    gr->getextent_gs(l, b, r, t, tol, gs);
}
void Graphic31::draw_ (Graphic31* gr, Canvas* c, Graphic31* gs) {
    gr->draw_gs(c, gs);
}
void Graphic31::drawclipped_ (
    Graphic31* gr, Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic31* gs
) {
    gr->drawclipped_gs(c, l, b, r, t, gs);
}
void Graphic31::transform_ (Coord x, Coord y, Coord& tx, Coord& ty, Graphic31* g) {
    Transformer* t = (g == nil) ? transformer() : g->transformer();

    if (t != nil) {
        t->Transform(x, y, tx, ty);
    } else {
        tx = x;
        ty = y;
    }
}

/************************************************************************/

PolyGraphic::PolyGraphic (Graphic31* gr) : Graphic31 (gr) {
    LayoutKit* layout = LayoutKit::instance();
    _body = layout->overlay();
}

PolyGraphic::~PolyGraphic () {
    delete _body;
}

void PolyGraphic::request (Requisition& req) const { 
   _body->request(req);
    Requirement& rx = req.x_requirement();
    Requirement& ry = req.y_requirement();

    Coord left, bottom, right, top;
    left = -rx.natural()*rx.alignment();
    right = left + rx.natural();
    bottom = -ry.natural()*ry.alignment();
    top = bottom + ry.natural();

    if (_t != nil) {
        corners(left, bottom, right, top, *_t);
    }
    rx.natural(right - left);
    rx.stretch(0.0);
    rx.shrink(0.0);
    rx.alignment(-left / rx.natural());

    ry.natural(top - bottom);
    ry.stretch(0.0);
    ry.shrink(0.0);
    ry.alignment(-bottom / ry.natural());
}

void PolyGraphic::allocate (Canvas* c, const Allocation& a, Extension& ext) {
    if (_t != nil) {
        c->push_transform();
        c->transform(*_t);
    } 
    _body->allocate(c, a, ext);
    if (_t != nil) {
        c->pop_transform();
    }
}

void PolyGraphic::undraw () { _body->undraw(); }

void PolyGraphic::append_ (Graphic31* g) { 
    _body->append(g); 
    g->parent(this);
}

void PolyGraphic::prepend_ (Graphic31* g) { 
    _body->prepend(g); 
    g->parent(this);
}

void PolyGraphic::insert_ (GlyphIndex i, Graphic31* g) { 
    _body->insert(i, g); 
    g->parent(this);
}

void PolyGraphic::remove_ (GlyphIndex i) { 
    Graphic31* gr = (Graphic31*)_body->component(i);
    if (gr != nil) {
        gr->parent(nil);
    }
    _body->remove(i); 
}

void PolyGraphic::replace_ (GlyphIndex i, Graphic31* g) { 
    Graphic31* gr = (Graphic31*) _body->component(i);
    gr->parent(nil);
    _body->replace(i, g); 
    g->parent(this);
}

void PolyGraphic::change_ (GlyphIndex i) { _body->change(i); }

void PolyGraphic::modified_ (GlyphIndex i) { _body->modified(i); }

void PolyGraphic::flush () {
    GlyphIndex count = count_();
    
    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = component_(i);
        concat_(gr, gr, this, gr);
        gr->flush();
    }
    Graphic31 n;
    *((Graphic31*) this) = *(&n);
}

Glyph* PolyGraphic::clone () const {
    Graphic31* pg = new PolyGraphic((Graphic31*)this);

    GlyphIndex count = _body->count();
    for (GlyphIndex i = 0; i < count; i++) {
        Glyph* gr = _body->component(i);
        pg->append(gr->clone());
    }
    return pg;
}
    
GlyphIndex PolyGraphic::count_ () const { return _body->count(); }

Graphic31* PolyGraphic::component_ (GlyphIndex i) const { 
    return (Graphic31*) _body->component(i);
}

Graphic31* PolyGraphic::first_containing (PointObj& pt) {
    GlyphIndex count = _body->count();

    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);

	if (gr->contains(pt)) {
	    return gr;
	}
    }
    return nil;
}

Graphic31* PolyGraphic::last_containing (PointObj& pt) {
    GlyphIndex count = _body->count();

    for (GlyphIndex i = count-1; i >= 0; i--) {
        Graphic31* gr = (Graphic31*) _body->component(i);

	if (gr->contains(pt)) {
	    return gr;
	}
    }
    return nil;
}

Graphic31* PolyGraphic::first_intersecting (BoxObj& b) {
    GlyphIndex count = _body->count();

    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);

	if (gr->intersects(b)) {
	    return gr;
	}
    }
    return nil;
}

Graphic31* PolyGraphic::last_intersecting (BoxObj& b) {
    GlyphIndex count = _body->count();

    for (GlyphIndex i = count-1; i >= 0; i--) {
        Graphic31* gr = (Graphic31*) _body->component(i);

	if (gr->intersects(b)) {
	    return gr;
	}
    }
    return nil;
}

Graphic31* PolyGraphic::first_within (BoxObj& gb) {
    GlyphIndex count = _body->count();
    Coord l, b, r, t;

    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);
        gr->getbounds(l, b, r, t);

        BoxObj box(l, b, r, t);
	if (box.Within(gb)) {
	    return gr;
	}
    }
    return nil;
}

Graphic31* PolyGraphic::last_within (BoxObj& gb) {
    GlyphIndex count = _body->count();
    Coord l, b, r, t;

    for (GlyphIndex i = count-1; i >= 0; i--) {
        Graphic31* gr = (Graphic31*) _body->component(i);
        gr->getbounds(l, b, r, t);

        BoxObj box(l, b, r, t);
	if (box.Within(gb)) {
	    return gr;
	}
    }
    return nil;
}

void PolyGraphic::draw_gs (Canvas* c, Graphic31* gs) {
    Graphic31 gstemp;

    GlyphIndex count = _body->count();
    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);
	concat_(gr, gr, gs, &gstemp);
	draw_(gr, c, &gstemp);
    }
}

void PolyGraphic::drawclipped_gs (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic31* gs
) {
    Graphic31 gstemp;
    
    GlyphIndex count = _body->count();
    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);
        concat_(gr, gr, gs, &gstemp);
        drawclipped_(gr, c, l, b, r, t, &gstemp);
    }
}

void PolyGraphic::getextent_gs (
    Coord& l, Coord& b, Coord& cx, Coord& cy, float& tol, Graphic31* gs
) {
    Extent e;
    l = b = cx = cy = tol = 0.0;
        
    Graphic31 gstemp;
    Transformer ttemp;
    Extent te;
    gstemp.transformer(&ttemp);

    GlyphIndex count = _body->count();
    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);
        
        concatgs_(gr, gr, gs, &gstemp);
        concatXform_(gr, nil, gr->transformer(), &ttemp);
        getextent_(gr, te._left, te._bottom, te._cx, te._cy, te._tol, &gstemp);
        e.Merge(te);
    }
    gstemp.transformer(nil); // to avoid deleting ttemp explicitly
    l = e._left; b = e._bottom; cx = l+(e._cx-l)*2.0; cy = b+(e._cy-b)*2.0;
    tol = e._tol;

    Transformer* tx = gs->transformer();
    if (tx != nil) {
        corners(l, b, cx, cy, *tx);
    }
    cx = (cx + l)/2.0;
    cy = (cy + b)/2.0;
}

boolean PolyGraphic::contains_gs (PointObj& po, Graphic31* gs) {
    GlyphIndex count = _body->count();
    Graphic31 gstemp;
    Transformer ttemp;

    gstemp.transformer(&ttemp);
    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);
        concat_(gr, gr, gs, &gstemp);
        
        if (contains_(gr, po, &gstemp)) {
            gstemp.transformer(nil);
            return true;
        }
    }
    gstemp.transformer(nil); /* to avoid deleting ttemp explicitly*/
    return false;
}

boolean PolyGraphic::intersects_gs (BoxObj& box, Graphic31* gs) {
    GlyphIndex count = _body->count();
    Graphic31 gstemp;
    Transformer ttemp;

    gstemp.transformer(&ttemp);
    for (GlyphIndex i = 0; i < count; i++) {
        Graphic31* gr = (Graphic31*) _body->component(i);
        concat_(gr, gr, gs, &gstemp);
        
        if (intersects_(gr, box, &gstemp)) {
            gstemp.transformer(nil);
            return true;
        }
    }
    gstemp.transformer(nil); /* to avoid deleting ttemp explicitly*/
    return false;
}

/**********************************************************************/
declarePtrList(GraphicList, Graphic31);
implementPtrList(GraphicList, Graphic31);

static Cursor* grabber_cursor = nil;
static Cursor* window_cursor = nil;

GraphicMaster::GraphicMaster (Graphic31* gr, const Color* bg) : PolyGraphic(gr) {
    _gr_list = new GraphicList;
    if (grabber_cursor == nil) {
        Bitmap* grabber = new Bitmap(
            grabber_bits, grabber_width, grabber_height,
            grabber_x_hot, grabber_y_hot
        );
        Bitmap* grabber_mask = new Bitmap(
            grabberMask_bits, grabberMask_width, grabberMask_height,
            grabberMask_x_hot, grabberMask_y_hot
        );
        grabber_cursor = new Cursor(grabber, grabber_mask);
    }
    _bg = bg;
    Resource::ref(_bg);
}

GraphicMaster::~GraphicMaster () {
    delete _gr_list;
    Resource::unref(_bg);
}

void GraphicMaster::background(const Color* bg) {
    if (bg != nil) {
        Resource::ref(bg);
    }
    if (_bg != nil) {
        Resource::unref(_bg);
    }
    _bg = bg;
}

Glyph* GraphicMaster::clone () const {
    Graphic31* pg = new GraphicMaster((Graphic31*)this);

    GlyphIndex count = _body->count();
    for (GlyphIndex i = 0; i < count; i++) {
        Glyph* gr = _body->component(i);
        pg->append(gr->clone());
    }
    return pg;
}

void GraphicMaster::request(Requisition& req) const {
    PolyGraphic::request(req);
    Requirement& rx = req.x_requirement();
    rx.alignment(0.0);
    
    Requirement& ry = req.y_requirement();
    ry.alignment(0.0);
}

void GraphicMaster::allocate (Canvas* c, const Allocation& a, Extension& ext) {
    if (!_a.equals(a, 0.001)) {
        Allocation b(a);
        Allotment& ax = _a.x_allotment();
        Allotment& ay = _a.y_allotment();
        Allotment& bx = b.x_allotment();
        Allotment& by = b.y_allotment();
        translate((bx.span()-ax.span())/2.0, (by.span()-ay.span())/2.0);
        translate(bx.begin()-ax.begin(), by.begin()-ay.begin());
        _a = a;
    }
    if (c != nil) {
        PolyGraphic::allocate(c, a, ext);
    }
}

void GraphicMaster::drawclipped_gs(
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic31* gs
) {
    c->push_clipping();
    c->clip_rect(_a.left()-1, _a.bottom()-1, _a.right()+1, _a.top()+1);
    if (_bg != nil) {
        c->fill_rect(l, b, r, t, _bg);
    }
    PolyGraphic::drawclipped_gs(c, l, b, r, t, gs);
    c->pop_clipping();
}

boolean GraphicMaster::grasp (const Event& e, Tool31& tool) {
    if (window_cursor == nil) {
        window_cursor = e.window()->cursor();
    }
    boolean flag = false;
    unsigned int tool_type = tool.tool();
    switch (tool_type) {
    case Tool31::move:
    case Tool31::scale:
    case Tool31::rotate:
        {
            float tol = 2.0;
            BoxObj box(
                e.pointer_x()-tol, e.pointer_y()-tol, 
                e.pointer_x()+tol, e.pointer_y()+tol
            );
            Graphic31* target = last_intersecting(box);
            if (target != nil) {
                Window* w = e.window();
                Canvas* c = w->canvas();
                w->cursor(grabber_cursor);

                ToolState& ts = tool.toolstate();
                total_gs(ts._gs);

                _gr_list->append(target);
                Coord l, b, r, t;
                target->getbounds(l, b, r, t);
                c->damage(l, b, r, t);
                flag = target->grasp(e, tool);
                if (flag) {
                    target->getbounds(l, b, r, t);
                    c->damage(l, b, r, t);
                }
                CanvasRep& rep = *c->rep();
                CanvasDamage& cd = rep.damage_;
                rep.start_repair();
                drawclipped(c, cd.left, cd.bottom, cd.right, cd.top);
                rep.finish_repair();
            }
            break;
        } 
    }
    return flag;
}

boolean GraphicMaster::manipulating (const Event& e, Tool31& tool) {
    boolean flag = true;
    long count = _gr_list->count();
    Coord l, b, r, t;
    Window* w = e.window();
    Canvas* c = w->canvas();
    for (long i = 0; i < count && flag; i++) {
        Graphic31* target = _gr_list->item(i);
        target->getbounds(l, b, r, t);
        c->damage(l, b, r, t);
        flag = target->manipulating(e, tool);
        if (flag) {
            target->getbounds(l, b, r, t);
            c->damage(l, b, r, t);
        }
        CanvasRep& rep = *c->rep();
        CanvasDamage& cd = rep.damage_;
        rep.start_repair();
        drawclipped(c, cd.left, cd.bottom, cd.right, cd.top);
        rep.finish_repair();
        w->display()->flush();
    }
    return flag;
}
    
boolean GraphicMaster::effect (const Event& e, Tool31& tool) {
    boolean flag = true;
    Window* w = e.window();
    if (w != nil) {
        w->cursor(window_cursor);
        Canvas* c = w->canvas();
        long count = _gr_list->count();
        Coord l, b, r, t;
        for (long i = 0; i < count && flag; i++) {
            Graphic31* target = _gr_list->item(i);
            target->getbounds(l, b, r, t);
            c->damage(l, b, r, t);
            flag = target->effect(e, tool);
            if (flag) {
                target->getbounds(l, b, r, t);
                c->damage(l, b, r, t);
            }
            target->flush();
            CanvasRep& rep = *c->rep();
            CanvasDamage& cd = rep.damage_;
            rep.start_repair();
            drawclipped(c, cd.left, cd.bottom, cd.right, cd.top);
            rep.finish_repair();
        }
        _gr_list->remove_all();
    } 
    return flag;
}

/**********************************************************************/

Line31::Line31(
    const Brush* brush, const Color* stroke, const Color* fill,
    Coord x1, Coord y1, Coord x2, Coord y2, Transformer* t
) : Graphic31(brush, stroke, fill, nil, false, false, 2, t) {
    add_point(x1, y1);
    add_point(x2, y2);
}

Line31::~Line31 () { }

Glyph* Line31::clone () const {
    return new Line31(_brush, _stroke, _fill, _x[0], _y[0], _x[1], _y[1], _t);
}

/**********************************************************************/
Rectangle31::Rectangle31 (
    const Brush* brush, const Color* stroke, const Color* fill,
    Coord l, Coord b, Coord r, Coord t, Transformer* tx
) : Graphic31(brush, stroke, fill, nil, true, false, 4, tx) {
    add_point(r, b);
    add_point(r, t);
    add_point(l, t);
    add_point(l, b);
}

Rectangle31::~Rectangle31 () { }

Glyph* Rectangle31::clone () const {
    return new Rectangle31(
        _brush, _stroke, _fill, _x[0], _y[0], _x[1], _y[1], _t
    );
}

/**********************************************************************/
static float p0 = 1.00000000;
static float p1 = 0.89657547;   // cos 30 * sqrt(1 + tan 15 * tan 15)
static float p2 = 0.70710678;   // cos 45 
static float p3 = 0.51763809;   // cos 60 * sqrt(1 + tan 15 * tan 15)
static float p4 = 0.26794919;   // tan 15

Circle31::Circle31 (
    const Brush* brush, const Color* stroke, const Color* fill,
    Coord x, Coord y, Coord r, Transformer* t
) : Graphic31(brush, stroke, fill, nil, true, true, 25, t) {
    float px0 = p0 * r, py0 = p0 * r;
    float px1 = p1 * r, py1 = p1 * r;
    float px2 = p2 * r, py2 = p2 * r;
    float px3 = p3 * r, py3 = p3 * r;
    float px4 = p4 * r, py4 = p4 * r;
    
    add_point(x + r, y);
    add_curve(x + px2, y + py2, x + px0, y + py4, x + px1, y + py3);
    add_curve(x, y + r, x + px3, y + py1, x + px4, y + py0);
    add_curve(x - px2, y + py2, x - px4, y + py0, x - px3, y + py1);
    add_curve(x - r, y, x - px1, y + py3, x - px0, y + py4);
    add_curve(x - px2, y - py2, x - px0, y - py4, x - px1, y - py3);
    add_curve(x, y - r, x - px3, y - py1, x - px4, y - py0);
    add_curve(x + px2, y - py2, x + px4, y - py0, x + px3, y - py1);
    add_curve(x + r, y, x + px1, y - py3, x + px0, y - py4);
}

Circle31::~Circle31 () { }

Glyph* Circle31::clone () const {
    Coord r = _x[0] - _x[4];
    Coord x = _x[4];
    Coord y = _y[0];
    return new Circle31(_brush, _stroke, _fill, x, y, r, _t);
}

/**********************************************************************/
Ellipse31::Ellipse31 (
    const Brush* brush, const Color* stroke, const Color* fill,
    Coord x, Coord y, Coord rx, Coord ry, Transformer* t
) : Graphic31(brush, stroke, fill, nil, true, true, 25, t) {
    float px0 = p0 * rx, py0 = p0 * ry;
    float px1 = p1 * rx, py1 = p1 * ry;
    float px2 = p2 * rx, py2 = p2 * ry;
    float px3 = p3 * rx, py3 = p3 * ry;
    float px4 = p4 * rx, py4 = p4 * ry;
    
    add_point(x + rx, y);
    add_curve(x + px2, y + py2, x + px0, y + py4, x + px1, y + py3);
    add_curve(x, y + ry, x + px3, y + py1, x + px4, y + py0);
    add_curve(x - px2, y + py2, x - px4, y + py0, x - px3, y + py1);
    add_curve(x - rx, y, x - px1, y + py3, x - px0, y + py4);
    add_curve(x - px2, y - py2, x - px0, y - py4, x - px1, y - py3);
    add_curve(x, y - ry, x - px3, y - py1, x - px4, y - py0);
    add_curve(x + px2, y - py2, x + px4, y - py0, x + px3, y - py1);
    add_curve(x + rx, y, x + px1, y - py3, x + px0, y - py4);
}

Ellipse31::~Ellipse31 () { }

Glyph* Ellipse31::clone () const {
    Coord rx = _x[0] - _x[4];
    Coord ry = _y[4] - _x[0];
    Coord x = _x[4];
    Coord y = _y[0];
    return new Ellipse31(_brush, _stroke, _fill, x, y, rx, ry, _t);
}

/**********************************************************************/
Open_BSpline31::Open_BSpline31 (
    const Brush* brush, const Color* stroke, const Color* fill, 
    Coord* x, Coord* y, int n, Transformer* t
) : Graphic31(brush, stroke, fill, nil, false, true, (n + 2) * 3 + 1, t) {
    Bspline_move_to(x[0], y[0], x[0], y[0], x[0], y[0]);
    Bspline_curve_to(x[0], y[0], x[0], y[0], x[1], y[1]);
    for (int i = 1; i < n - 1; ++i) {
        Bspline_curve_to(x[i], y[i], x[i-1], y[i-1], x[i+1], y[i+1]);
    }
    Bspline_curve_to(x[n-1], y[n-1], x[n-2], y[n-2], x[n-1], y[n-1]);
    Bspline_curve_to(x[n-1], y[n-1], x[n-1], y[n-1], x[n-1], y[n-1]);
}

Open_BSpline31::Open_BSpline31 (Open_BSpline31* gr) : Graphic31(gr) {}


Open_BSpline31::~Open_BSpline31 () { }

Glyph* Open_BSpline31::clone () const {
    return new Open_BSpline31((Open_BSpline31*) this);
}

/**********************************************************************/
Closed_BSpline31::Closed_BSpline31 (
    const Brush* brush, const Color* stroke, const Color* fill, 
    Coord* x, Coord* y, int n, Transformer* t
) : Graphic31(brush, stroke, fill, nil, true, true, n * 3 + 1, t) {
    Bspline_move_to(x[0], y[0], x[n-1], y[n-1], x[1], y[1]);
    for (int i = 1; i < n - 1; ++i) {
        Bspline_curve_to(x[i], y[i], x[i-1], y[i-1], x[i+1], y[i+1]);
    }
    Bspline_curve_to(x[n-1], y[n-1], x[n-2], y[n-2], x[0], y[0]);
    Bspline_curve_to(x[0], y[0], x[n-1], y[n-1], x[1], y[1]);
}

Closed_BSpline31::Closed_BSpline31 (Closed_BSpline31* gr) : Graphic31(gr) {}

Closed_BSpline31::~Closed_BSpline31 () { }

Glyph* Closed_BSpline31::clone () const {
    return new Closed_BSpline31((Closed_BSpline31*) this);
}
/**********************************************************************/
Polyline31::Polyline31 (
    const Brush* brush, const Color* stroke, const Color* fill,
    Coord* x, Coord* y, int n, Transformer* t
) : Graphic31(brush, stroke, fill, nil, false, false, n, t) {
    add_point(x[0], y[0]);
    for (int i = 1; i < n; ++i) {
        add_point(x[i], y[i]);
    }
}

Polyline31::~Polyline31 () { }

Glyph* Polyline31::clone () const {
    return new Polyline31(_brush, _stroke, _fill, _x, _y, _ctrlpts, _t);
}

/**********************************************************************/
Polygon31::Polygon31 (
    const Brush* brush, const Color* stroke, const Color* fill, 
    Coord* x, Coord* y, int n, Transformer* t
) : Graphic31(brush, stroke, fill, nil, true, false, n, t) {
    add_point(x[0], y[0]);
    for (int i = 1; i < n; ++i) {
        add_point(x[i], y[i]);
    }
}

Polygon31::~Polygon31 () { }

Glyph* Polygon31::clone () const {
    return new Polygon31(_brush, _stroke, _fill, _x, _y, _ctrlpts, _t);
}

/**********************************************************************/
Text31::Text31 (
    const Font* f, const Color* fg, const char* text, Transformer* tx
) {
    _stroke = fg;
    Resource::ref(_stroke);
    _font = f;
    Resource::ref(_font);
    _t = new Transformer;
    if (tx != nil) {
        *_t = *tx;
    }
    _fill = fg; /* a hack */

    _text = nil;
    if (text != nil) {
        _text = new String(text);
    }
    LayoutKit* layout = LayoutKit::instance();
    _body = layout->overlay();
    init();
}

Text31::~Text31 () {
    delete _text;
    delete _body;
}

Glyph* Text31::clone () const {
    return new Text31(_font, _stroke, _text->string(), _t);
}

void Text31::text (const char* text) {
    delete _text;
    _text = nil;
    if (text != nil) {
        _text = new String(text);
    }
    init();
}

const char* Text31::text () {
    return _text->string();
}

void Text31::init () {
    LayoutKit* layout = LayoutKit::instance();
    PolyGlyph* col = layout->vbox();
    PolyGlyph* line = layout->hbox();
    FontBoundingBox bbox;
    _font->font_bbox(bbox);
    Coord lineheight = bbox.ascent() + bbox.descent();
    char ch;
    
    for (int i = 0; (*_text)[i] != '\0'; i++) {
        ch = (*_text)[i];

        if (ch == '\n') {
            line->append(layout->strut(_font));
            col->append(layout->fixed_dimension(line, Dimension_Y,lineheight));
            line = layout->hbox();
        } else if (ch == ' ') {
            line->append(new Character(' ', _font, _stroke));
        } else if (ch != ')' && ch != '(') {
            if (ch == '\\') {
                ch = (*_text)[++i];
                if (isdigit(ch)) {
                    ch -= '0';
                    ch *= 8;
                    char digit;
                    digit = (*_text)[i++];
                    ch = (ch * 8) + digit - '0';
                    digit = (*_text)[i++];
                    ch = (ch * 8) + digit - '0';
                }
            }
            line->append(new Character(ch, _font, _stroke));
        }
    }

    Transformer fixtext;
    fixtext.translate(0, bbox.descent());
    _t->premultiply(fixtext);
    _body->append(col);
}

void Text31::draw (Canvas* c, const Allocation& a) const {
    if (_t != nil) {
        c->push_transform();
        c->transform(*_t);
    } 
    _body->draw(c, a);
    if (_t != nil) {
        c->pop_transform();
    }
}

void Text31::draw_gs (Canvas* c, Graphic31* gs) {
    Transformer* tx = gs->transformer();
    if (tx != nil) {
        c->push_transform();
        c->transform(*tx);
    }
    if (_ctrlpts == 0) {
        Requisition req;
        request(req);
    }
    Allocation b(_a);
    Allotment& bx = b.x_allotment();
    Allotment& by = b.y_allotment();
    bx.origin(0.0);
    by.origin(0.0);
    bx.span(_x[2]-_x[0]);
    by.span(_y[2]-_y[0]);

    _body->draw(c, b);
    if (tx != nil) {
        c->pop_transform();
    }
}

void Text31::allocate(Canvas* c, const Allocation& a, Extension& ext) {
    if (_t != nil) {
        c->push_transform();
        c->transform(*_t);
    }
    if (_ctrlpts == 0) {
        Requisition req;
        request(req);
    }
    _a = a;
    Allocation b(_a);
    Allotment& bx = b.x_allotment();
    Allotment& by = b.y_allotment();
    bx.origin(0.0);
    by.origin(0.0);
    bx.span(_x[2]-_x[0]);
    by.span(_y[2]-_y[0]);
    _body->allocate(c, b, ext);
    if (_t != nil) {
        c->pop_transform();
    }
}

void Text31::getextent_gs (
    Coord& l, Coord& b, Coord& cx, Coord& cy, float& tol, Graphic31* gr
) {
    if (_ctrlpts == 0) {
        Requisition req;
        request(req);
    }
    Coord left, bottom, right, top;

    right = _x[2];
    top = _y[2];
    left = _x[0];
    bottom = _y[0];

    Transformer* t = gr->transformer();
    if (t != nil) {
        corners(left, bottom, right, top, *t);
    }
    tol = 3.0;
    l = left;
    b = bottom;
    cx = (left + right)/2.0;
    cy = (top + bottom)/2.0;
}

void Text31::request (Requisition& req) const { 
    _body->request(req);
    Requirement& rx = req.x_requirement();
    Requirement& ry = req.y_requirement();

    Coord left, bottom, right, top;
    left = -rx.natural()*rx.alignment();
    right = left + rx.natural();
    bottom = -ry.natural()*ry.alignment();
    top = bottom + ry.natural();

    Text31* text = (Text31*) this;
    text->_ctrlpts = 4;
    text->_x[0] = left;
    text->_y[0] = bottom;
    text->_x[1] = left;
    text->_y[1] = top;
    text->_x[2] = right;
    text->_y[2] = top;
    text->_x[3] = right;
    text->_y[3] = bottom;

    if (_t != nil) {
        corners(left, bottom, right, top, *_t);
    }
    rx.natural(right - left);
    rx.stretch(0.0);
    rx.shrink(0.0);
    rx.alignment(-left / rx.natural());

    ry.natural(top - bottom);
    ry.stretch(0.0);
    ry.shrink(0.0);
    ry.alignment(-bottom / ry.natural());
}

Tool31::Tool31 (unsigned int cur_tool) {
    _cur_tool = cur_tool;
    _toolstate = new ToolState;
}

Tool31::~Tool31 () {
    delete _toolstate;
}

unsigned int Tool31::tool () { return _cur_tool; }

void Tool31::tool (unsigned int cur_tool) { _cur_tool = cur_tool; }

void Tool31::reset () {
    delete _toolstate;
    _toolstate = new ToolState;
}


ToolState& Tool31::toolstate () { return *_toolstate; }

void Tool31::toolstate(ToolState* toolstate) {
    delete _toolstate;
    _toolstate = toolstate;
}

