/*
 * Copyright (c) 1990, 1991 Stanford University
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
 * UPage - defines the viewer's page boundary.
 */

#ifndef unidraw_upage_h
#define unidraw_upage_h

#include <Unidraw/enter-scope.h>

#include <Unidraw/Graphic/graphic.h>

class PSBrush;

class PageGraphic : public Graphic {
public:
    PageGraphic(float, float, Graphic* = nil);
    virtual ~PageGraphic();

    void GetOriginal(float&, float&);

    virtual void SetBrush(PSBrush*);
    virtual PSBrush* GetBrush();

    virtual Graphic* Copy();
protected:
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
private:
    float _width, _height;
    PSBrush* _br;
};

class UPage {
public:
    UPage(float w, float h);
    UPage(Graphic*);
    virtual ~UPage();

    virtual Graphic* GetGraphic();
protected:
    Graphic* _graphic;
};

#endif
