/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
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
 * Basic composite object for interaction.
 */

#include <IV-2_6/InterViews/box.h>
#include <IV-2_6/InterViews/shape.h>
#include <IV-2_6/_enter.h>
#include <OS/math.h>

class BoxElement {
public:
    Interactor* child;
    boolean visible;
    BoxElement* next;
};

class BoxDimension {
public:
    int natural;
    int stretch;
    int shrink;
};

class BoxCanonical {
public:
    BoxDimension major;
    BoxDimension minor;
};

Box::Box() {
    nelements = 0;
    head = nil;
    tail = nil;
}

Box::~Box() {
    register BoxElement* e;
    register BoxElement* next;

    for (e = head; e != nil; e = next) {
	next = e->next;
	delete e->child;
	delete e;
    }
}

void Box::Align(Alignment a) {
    align = a;
}

void Box::DoInsert(Interactor* i, boolean, IntCoord&, IntCoord&) {
    register BoxElement* e;

    ++nelements;
    e = new BoxElement;
    e->child = i;
    e->next = nil;
    if (head == nil) {
	head = e;
	tail = e;
    } else {
	tail->next = e;
	tail = e;
    }
}

void Box::DoChange(Interactor*) {
    Reconfig();
}

void Box::DoRemove(Interactor* i) {
    register BoxElement* e, * prev;

    --nelements;
    prev = nil;
    for (e = head; e != nil; e = e->next) {
	if (e->child == i) {
	    if (prev == nil) {
		head = e->next;
	    } else {
		prev->next = e->next;
	    }
	    if (e == tail) {
		tail = prev;
	    }
	    delete e;
	    break;
	}
	prev = e;
    }
    ComputeShape(shape);
}

void Box::Reconfig() {
    ComputeShape(shape);
}

void Box::Resize() {
    register BoxElement* e;	/* box element */
    Shape aggrshape;		/* combined shape of components */
    BoxCanonical total;		/* components' shape along major axis */
    int major, minor;		/* actual dimensions of box */
    register int have;		/* how much box is willing to change */
    register int need;		/* how much box needs to change to fit */
    boolean grow;		/* true if stretching, false if shrinking */
    BoxCanonical s;		/* element shape along major axis */
    register int pos;		/* where to put next element on major axis */
    register int len;		/* size of element along major axis */
    register int n;		/* temporary variable */

    ComputeShape(&aggrshape);
    GetActual(major, minor);
    GetCanonical(&aggrshape, total);
    n = total.major.natural;
    if (major > n) {
	/* more space than desired ==> stretch elements */
	grow = true;
	have = total.major.stretch;
	need = Math::min(major - n, have);
    } else {
	/* less (or equal) space than desired ==> (maybe) shrink elements */
	grow = false;
	have = total.major.shrink;
	need = Math::min(n - major, have);
    }
    pos = 0;
    for (e = head; e != nil; e = e->next) {
	GetCanonical(e->child->GetShape(), s);
	len = s.major.natural;
	if (have > 0) {
	    if (grow) {
		n = int(double(s.major.stretch)*double(need)/double(have));
		len += n;
		have -= s.major.stretch;
	    } else {
		n = int(double(s.major.shrink)*double(need)/double(have));
		len -= n;
		have -= s.major.shrink;
	    }
	    need -= n;
	}
	n = s.minor.natural;
	if (n == 0) {
	    n = minor;
	} else if (n > minor) {
	    n = Math::max(n - s.minor.shrink, minor);
	} else if (n < minor) {
	    n = Math::min(n + s.minor.stretch, minor);
	}
	if (n > 0 && len > 0) {
	    e->visible = true;
	    PlaceElement(e->child, pos, len, minor, n);
	} else {
	    e->visible = false;
	}
	pos += len;
    }
}

void Box::Draw() {
    register BoxElement* e;

    for (e = head; e != nil; e = e->next) {
	if (e->visible) {
	    e->child->Draw();
	}
    }
}

void Box::GetComponents(Interactor** c, int nc, Interactor**& a, int& n) {
    register BoxElement* e;
    register Interactor** ap;

    n = nelements;
    a = (n <= nc) ? c : new Interactor*[n];
    ap = a;
    for (e = head; e != nil; e = e->next) {
	*ap++ = e->child;
    }
}

inline BoxElement* Box::Head() {
    return head;
}

/*
 * Default virtuals.
 */

void Box::ComputeShape(Shape*) {}
void Box::GetActual(int& /* major */, int& /* minor */) {}
void Box::GetCanonical(Shape*, BoxCanonical&) {}
void Box::PlaceElement(Interactor*, IntCoord, int, int, int) {}

void HBox::Init() {
    align = Bottom;
    shape->Rigid(0, 0, vfil, vfil);
}
    
HBox::HBox() {
    Init();
}

HBox::HBox(Interactor* i1) {
    Init();
    Insert(i1);
}

HBox::HBox(Interactor* i1, Interactor* i2) {
    Init();
    Insert(i1);
    Insert(i2);
}

HBox::HBox(Interactor* i1, Interactor* i2, Interactor* i3) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
}

HBox::HBox(Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
}

HBox::HBox(
    Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5
) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
    Insert(i5);
}

HBox::HBox(
    Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6
) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
    Insert(i5);
    Insert(i6);
}

HBox::HBox(
    Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6, Interactor* i7
) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
    Insert(i5);
    Insert(i6);
    Insert(i7);
}

HBox::~HBox() { }

void HBox::ComputeShape(register Shape* box) {
    register BoxElement* e;
    register Shape* s;
    register int vmin, vmax;

    box->width = 0;
    box->height = 0;
    box->Rigid(0, 0, vfil, vfil);
    vmin = -vfil;
    vmax = vfil;
    for (e = Head(); e != nil; e = e->next) {
	s = e->child->GetShape();
	box->width += s->width;
	box->height = Math::max(box->height, s->height);
	box->hstretch += s->hstretch;
	box->hshrink += s->hshrink;
	vmin = Math::max(s->height - s->vshrink, vmin);
	vmax = Math::min(s->height + s->vstretch, vmax);
    }
    box->vstretch = Math::max(0, vmax - box->height);
    box->vshrink = Math::max(0, box->height - vmin);
}

void HBox::GetActual(int& major, int& minor) {
    major = xmax + 1;
    minor = ymax + 1;
}

void HBox::GetCanonical(register Shape* s, register BoxCanonical& b) {
    b.major.natural = s->width;
    b.major.shrink = s->hshrink;
    b.major.stretch = s->hstretch;
    b.minor.natural = s->height;
    b.minor.shrink = s->vshrink;
    b.minor.stretch = s->vstretch;
}

void HBox::PlaceElement(
    Interactor* i, IntCoord x, int length, int size, int h
) {
    IntCoord x1, y1, x2, y2;

    x1 = x;
    if (align == Top) {
	y1 = size - h;
    } else if (align == Center) {
	y1 = (size - h) / 2;
    } else /* Bottom */ {
	y1 = 0;
    }
    x2 = x1 + length - 1;
    y2 = y1 + h - 1;
    Place(i, x1, y1, x2, y2);
}

void VBox::Init() {
    align = Left;
    shape->Rigid(hfil, hfil, 0, 0);
}
    
VBox::VBox() {
    Init();
}

VBox::VBox(Interactor* i1) {
    Init();
    Insert(i1);
}

VBox::VBox(Interactor* i1, Interactor* i2) {
    Init();
    Insert(i1);
    Insert(i2);
}

VBox::VBox(Interactor* i1, Interactor* i2, Interactor* i3) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
}

VBox::VBox(Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
}

VBox::VBox(
    Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5
) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
    Insert(i5);
}

VBox::VBox(
    Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6
) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
    Insert(i5);
    Insert(i6);
}

VBox::VBox(
    Interactor* i1, Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6, Interactor* i7
) {
    Init();
    Insert(i1);
    Insert(i2);
    Insert(i3);
    Insert(i4);
    Insert(i5);
    Insert(i6);
    Insert(i7);
}

VBox::~VBox() { }

void VBox::ComputeShape(register Shape* box) {
    register BoxElement* e;
    register Shape* s;
    register int hmin, hmax;

    box->width = 0;
    box->height = 0;
    box->Rigid(hfil, hfil, 0, 0);
    hmin = -hfil;
    hmax = hfil;
    for (e = Head(); e != nil; e = e->next) {
	s = e->child->GetShape();
	box->width = Math::max(box->width, s->width);
	box->height += s->height;
	box->vstretch += s->vstretch;
	box->vshrink += s->vshrink;
	hmin = Math::max(s->width - s->hshrink, hmin);
	hmax = Math::min(s->width + s->hstretch, hmax);
    }
    box->hstretch = Math::max(0, hmax - box->width);
    box->hshrink = Math::max(0, box->width - hmin);
}

void VBox::GetActual(int& major, int& minor) {
    major = ymax + 1;
    minor = xmax + 1;
}

void VBox::GetCanonical(register Shape* s, register BoxCanonical& b) {
    b.major.natural = s->height;
    b.major.shrink = s->vshrink;
    b.major.stretch = s->vstretch;
    b.minor.natural = s->width;
    b.minor.shrink = s->hshrink;
    b.minor.stretch = s->hstretch;
}

void VBox::PlaceElement(
    Interactor* i, IntCoord y, int length, int size, int w
) {
    IntCoord x1, y1, x2, y2;

    if (align == Right) {
	x1 = size - w;
    } else if (align == Center) {
	x1 = (size - w) / 2;
    } else /* Left */ {
	x1 = 0;
    }
    x2 = x1 + w - 1;
    y2 = ymax - y;
    y1 = y2 - length + 1;
    Place(i, x1, y1, x2, y2);
}
