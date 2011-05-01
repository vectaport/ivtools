/*
 * Copyright (c) 1996 Vectaport Inc.
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

#ifdef CLIPPOLY

#include <OverlayUnidraw/clippoly.h>
#include <poly.h>
#include <poly_io.h>
#include <primitives.h>
#include <nclip.h>
#include <stream.h>
#include <OS/math.h>

void clippoly(ClipOperation op, 
	      int n1, float* x1, float* y1, int n2, float* x2, float* y2,
	      int& npolys, int*& ni, float**& x, float**& y
	  ) {
    int i;
#if 0
    cout << "A:" << endl;
    for (i = 0; i < n1; i++)
	cout << x1[i] << " " << y1[i] << endl;
    cout << "B:" << endl;
    for (i = 0; i < n2; i++)
	cout << x2[i] << " " << y2[i] << endl;
    cout << endl;
#endif
    Poly* poly1;
    for (i = 0; i < n1; i++) {
	if (i == 0) {
	    Point* p0 = new Point(x1[0], y1[0]);
	    poly1 = new Poly(*p0);
	}
	else if (!(x1[i] == x1[i-1] && y1[i] == y1[i-1]) &&
		 !(i==n1-1 && x1[i] == x1[0] && y1[i] == y1[0])) {
	    Point* pi = new Point(x1[i], y1[i]);
	    poly1->add(*pi);
	}
    }
    Poly* poly2;
    for (i = 0; i < n2; i++) {
	if (i == 0) {
	    Point* p0 = new Point(x2[0], y2[0]);
	    poly2 = new Poly(*p0);
	}
	else if (!(x2[i] == x2[i-1] && y2[i] == y2[i-1]) &&
		 !(i==n2-1 && x2[i] == x2[0] && y2[i] == y2[0])) {
	    Point* pi = new Point(x2[i], y2[i]);
	    poly2->add(*pi);
	}
    }
#if 0
    cout << "a:\n" << *poly1;
    cout << "b:\n" << *poly2;
#endif
    if (poly1->area() == 0) {
      npolys = 0;
      return;
    }
    else if (poly2->area() == 0) {
      npolys = 0;
      return;
    }

    PolyPList a_min_b, b_min_a, a_and_b;
    clip_poly(*poly1, *poly2, a_min_b, b_min_a, a_and_b);
#if 0
    cout << "a_min_b:\n" << a_min_b;
    cout << "b_min_a:\n" << b_min_a;
    cout << "a_and_b:\n" << a_and_b;
#endif
#if 0
    cout << "A_MIN_B:" << endl;
    for (i = 0; i < a_min_b.length(); i++) {
	Poly* poly = a_min_b[i];
	cout << "poly " << i << ":" << endl;
	const PolyNode* node;
	int n = 0;
	for (node = poly->firstnode();
	     node && (n == 0 || node != poly->firstnode());
	     node = poly->nextnode(node), n++)
	    cout << node->point().x() << " " << node->point().y() << endl;
	cout << endl;
    }
    cout << "B_MIN_A:" << endl;
    for (i = 0; i < b_min_a.length(); i++) {
	Poly* poly = b_min_a[i];
	cout << "poly " << i << ":" << endl;
	const PolyNode* node;
	int n = 0;
	for (node = poly->firstnode();
	     node && (n == 0 || node != poly->firstnode());
	     node = poly->nextnode(node), n++)
	    cout << node->point().x() << " " << node->point().y() << endl;
	cout << endl;
    }
    cout << "A_AND_B:" << endl;
    for (i = 0; i < a_and_b.length(); i++) {
	Poly* poly = a_and_b[i];
	cout << "poly " << i << ":" << endl;
	const PolyNode* node;
	int n = 0;
	for (node = poly->firstnode();
	     node && (n == 0 || node != poly->firstnode());
	     node = poly->nextnode(node), n++)
	    cout << node->point().x() << " " << node->point().y() << endl;
	cout << endl;
    }
#endif
    switch (op) {
    case A_MIN_B:
	{
	    npolys = a_min_b.length();
	    x = new float*[npolys];
	    y = new float*[npolys];
	    ni = new int[npolys];
	    PolyPListIter iter(a_min_b);
	    for (i = 0; iter(); i++) {
		Poly* poly = iter.val();
		const PolyNode* node;
		int n = 0;
		for (node = poly->firstnode();
		     node && (n == 0 || node != poly->firstnode());
		     node = poly->nextnode(node), n++)
		    ;
		ni[i] = n;
		x[i] = new float[n];
		y[i] = new float[n];
		n = 0;
		int j = 0;
		for (j = 0, node = poly->firstnode();
		     node && (n == 0 || node != poly->firstnode());
		     j++, node = poly->nextnode(node), n++) {
		    x[i][j] = node->point().x();
		    y[i][j] = node->point().y();
		}
	    }
	}
	break;
    case B_MIN_A:
	{
	    npolys = b_min_a.length();
	    x = new float*[npolys];
	    y = new float*[npolys];
	    ni = new int[npolys];
	    PolyPListIter iter(b_min_a);
	    for (i = 0; iter(); i++) {
		Poly* poly = iter.val();
		const PolyNode* node;
		int n = 0;
		for (node = poly->firstnode();
		     node && (n == 0 || node != poly->firstnode());
		     node = poly->nextnode(node), n++)
		    ;
		ni[i] = n;
		x[i] = new float[n];
		y[i] = new float[n];
		n = 0;
		int j = 0;
		for (j = 0, node = poly->firstnode();
		     node && (n == 0 || node != poly->firstnode());
		     j++, node = poly->nextnode(node), n++) {
		    x[i][j] = node->point().x();
		    y[i][j] = node->point().y();
		}
	    }
	}
	break;
    case A_AND_B:
	{
	    npolys = a_and_b.length();
	    x = new float*[npolys];
	    y = new float*[npolys];
	    ni = new int[npolys];
	    PolyPListIter iter(a_and_b);
	    for (i = 0; iter(); i++) {
		Poly* poly = iter.val();
		const PolyNode* node;
		int n = 0;
		for (node = poly->firstnode();
		     node && (n == 0 || node != poly->firstnode());
		     node = poly->nextnode(node), n++)
		    ;
		ni[i] = n;
		x[i] = new float[n];
		y[i] = new float[n];
		n = 0;
		int j = 0;
		for (j = 0, node = poly->firstnode();
		     node && (n == 0 || node != poly->firstnode());
		     j++, node = poly->nextnode(node), n++) {
		    x[i][j] = node->point().x();
		    y[i][j] = node->point().y();
		}
	    }
	}
	break;
    default:
	cerr << "Error: uknown clippoly operation" << endl;
	break;
    }

}

void clippoly_delete(int npolys, int* ni, float** x, float** y) 
{
    for (int i= 0; i<npolys; i++) {
	delete [] x[i];
	delete [] y[i];
    }
    delete [] x;
    delete [] y;
    delete [] ni;
}
#endif
