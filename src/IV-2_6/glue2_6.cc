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
 * All that's necessary for glue is to set up the shape and
 * redraw the background.
 */

#include <IV-2_6/InterViews/glue.h>
#include <IV-2_6/InterViews/shape.h>
#include <IV-2_6/_enter.h>

Glue::Glue() {
    Init();
}

Glue::Glue(const char* name) {
    SetInstance(name);
    Init();
}

Glue::~Glue() { }

void Glue::Init() {
    SetClassName("Glue");
}

void Glue::Redraw(IntCoord, IntCoord, IntCoord, IntCoord) {
    /*
     * Don't do any drawing--assume that an ancestor will take care of it,
     * probably through Canvas::SetBackground.
     */
}

HGlue::HGlue(int nat, int str) {
    Init(nat, nat, str);
}

HGlue::HGlue(const char* name, int nat, int str) : Glue(name) {
    Init(nat, nat, str);
}

HGlue::HGlue(int nat, int shr, int str) {
    Init(nat, shr, str);
}

HGlue::HGlue(const char* name, int nat, int shr, int str) : Glue(name) {
    Init(nat, shr, str);
}

HGlue::~HGlue() { }

void HGlue::Init(int nat, int shr, int str) {
    SetClassName("HGlue");
    shape->width = nat;
    shape->height = 0;
    shape->Rigid(shr, str, vfil, vfil);
}

VGlue::VGlue(int nat, int str) {
    Init(nat, nat, str);
}

VGlue::VGlue(const char* name, int nat, int str) : Glue(name) {
    Init(nat, nat, str);
}

VGlue::VGlue(int nat, int shr, int str) {
    Init(nat, shr, str);
}

VGlue::VGlue(const char* name, int nat, int shr, int str) : Glue(name) {
    Init(nat, shr, str);
}

VGlue::~VGlue() { }

void VGlue::Init(int nat, int shr, int str) {
    SetClassName("VGlue");
    shape->width = 0;
    shape->height = nat;
    shape->Rigid(hfil, hfil, shr, str);
}
