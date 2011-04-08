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

/*
  MultiLine clipping to a box, using line clipping code based on
  http://www.ul.ie/~flanagan/gsoft3/gsoft3.html and
  http://www.ul.ie/~flanagan/gsoft3/gsoft3_1.html
  */

#include <OS/math.h>

static int  clip_left   =  50,
  clip_right  = 150,
  clip_bottom =  50,
  clip_top    = 150;
   
enum Region   { left   = 8, 
		right  = 4, 
		below  = 2, 
		above  = 1, 
		inside = 0 };

class CPoint {  // avoid collision with InterViews Point
public:
  int x, y;
};

int operator== (CPoint p0, CPoint p1) {
  return p0.x == p1.x && p0.y == p1.y;
}

Region FindRegion(CPoint p)
{
  Region R = inside;
  
  if ( p.x < clip_left )        R = (Region) (R | left);
  else if ( p.x > clip_right )  R = (Region) (R | right);
  if ( p.y < clip_bottom )      R = (Region) (R | below);
  else if ( p.y > clip_top )    R = (Region) (R | above);
   
  return R;
}

int find_furthest_visible_point(CPoint &alpha, 
        CPoint p0, CPoint p1)
{
  CPoint M;  Region Rp0, Rp1, Rm;

  if ( (Rp1 = FindRegion(p1)) == inside )  {
    alpha = p1;  return 1;
  }
  Rp0 = FindRegion(p0);
  while ( 1 )  {
    if ( Rp0 & Rp1 )  return 0;
    M.x = (p0.x+p1.x) >> 1;
    M.y = (p0.y+p1.y) >> 1;
    if ( M == p0 || M == p1 )  { 
      if ( Rp1 == inside )  {
        alpha = p1; return 1;
      }
      else if ( Rp0 == inside )  {
        alpha = p0; return 1;
      }
      else  return 0;
      break;
    } 
    Rm = FindRegion(M);
    if ( Rm & Rp1 )  { p1 = M;  Rp1 = Rm; }
    else  { p0 = M;  Rp0 = Rm; }
  }
  return 1;  // this is a dummy return statement,
	     // put here to satisfy the compiler,
	     // execution never gets to this
	     // point.
}

int clip(CPoint &p0, CPoint &p1)
{
  CPoint alpha, beta;

  if ( !find_furthest_visible_point(alpha,p0,p1) )  
      return 0;
  if ( !find_furthest_visible_point(beta,p1,p0) )   
      return 0;
  p0 = alpha;
  p1 = beta;
  return 1;
}

int clipline(int& x0, int& y0, int& x1, int& y1,
	     int l, int b, int r, int t)
{
  clip_left = Math::min(l, r);
  clip_right = Math::max(l, r);
  clip_bottom = Math::min(b, t);
  clip_top = Math::max(b, t);
  CPoint p0, p1;
  p0.x = x0;
  p0.y = y0;
  p1.x = x1;
  p1.y = y1;
  int ret = clip(p0, p1);
  if (ret) {  // why are they reversed?  need this for mline threading.
    x0 = p1.x;
    y0 = p1.y;
    x1 = p0.x;
    y1 = p0.y;
  }
  return ret;
}

void clipmultiline(int n, int* x, int* y,
		   int l, int b, int r, int t,
		   int& nlines, int*& ni, int**& nx, int**& ny)
{
  int havemline = 0;
  int curn = 0;
  int curp = 0;
  nlines = 0;
  ni = new int[n];
  nx = new int*[n];
  ny = new int*[n]; // n is upper bound on num of resulting mlines...
  for (int i = 0; i < n; i++) {
    ni[i] = 0;
    nx[i] = new int[n];
    ny[i] = new int[n]; // and on num of points per mline
  }
  for (int i = 1; i < n; i++) {
    int x0 = x[i-1];
    int y0 = y[i-1];
    int x1 = x[i];
    int y1 = y[i];
    if (clipline(x0, y0, x1, y1, l, b, r, t)) {
      if (havemline) {
	// add new point to current mline
	nx[nlines][ni[nlines]] = x1;
	ny[nlines][ni[nlines]] = y1;
	ni[nlines]++;

	// continue mline if 2nd point was not clipped. stop at last pt.
	if (x1 != x[i] || y1 != y[i] || i == n-1) {
	  havemline = 0;
	  nlines++;
	}
      }
      else {
	// start a new mline
	nx[nlines][0] = x0;
	ny[nlines][0] = y0;
	nx[nlines][1] = x1;
	ny[nlines][1] = y1;
	ni[nlines] = 2;

	// but only continue it if 2nd point was not clipped. stop at last pt.
	if (x1 != x[i] || y1 != y[i] || i == n-1) {
	  havemline = 0;
	  nlines++;
	}
	else {
	  havemline = 1;
	}
      }
    }
    else {
      havemline = 0;
    }
  }
}

void clipmultiline_delete(int npolys, int* ni, int** x, int** y) 
{
    for (int i= 0; i<npolys; i++) {
	delete [] x[i];
	delete [] y[i];
    }
    delete [] x;
    delete [] y;
    delete [] ni;
}
