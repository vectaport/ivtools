/*
 * Copyright (c) 1994 Vectaport Inc., Cartoactive Systems
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
 * PicturePS definition
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovpspict.h>

#include <Unidraw/catalog.h>
#include <Unidraw/creator.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Graphic/picture.h>

/*****************************************************************************/

ClassId PicturePS::GetClassId () { return PICTURE_PS; }

boolean PicturePS::IsA (ClassId id) {
    return PICTURE_PS == id || OverlaysPS::IsA(id);
}

PicturePS::PicturePS (OverlayComp* subj) : OverlaysPS(subj) {
}

PicturePS::~PicturePS () {
    DeleteComps();
}

void PicturePS::Update () {
    DeleteViews();

    Picture* picture = (Picture*) GetGraphicComp()->GetGraphic();
    Iterator i;

    for (picture->First(i); !picture->Done(i); picture->Next(i)) {
        Graphic* graphic = picture->GetGraphic(i);
        OverlayPS* ovpsv = CreateOvPSViewFromGraphic(graphic);

        if (ovpsv != nil) {
	    OverlayComp* comp = new OverlayComp(graphic);
	    comp->Attach(ovpsv);
	    ovpsv->Update();
            _views->Append(new UList(ovpsv));
        }
    }
}

void PicturePS::DeleteComps () {
    Iterator i;

    First(i);
    while (!Done(i)) {
	GraphicComp* comp = (GraphicComp*)GetView(i)->GetSubject();
	comp->SetGraphic(nil);
        delete comp;
	Next(i);
    }
}    

