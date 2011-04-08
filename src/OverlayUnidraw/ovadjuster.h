/*
 * Copyright (c) 1995-1999 Vectaport Inc.
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
 * classes derived from Adjuster
 */

#ifndef ov_adjuster_h
#define ov_adjuster_h

#include <IV-2_6/InterViews/adjuster.h>

#include <IV-2_6/_enter.h>

//: clone of Mover.
// Partial attempt to change the appearance or location of the 
// panner buttons on a drawing editor.  Perhaps there is a unremembered bugfix
// in this subtree (which includes OvLeftMover, OvRightMover, OvUpMover, and 
// OvDownMover) but an inspection of the methods doesn't show one up.  These
// classes could be factored out in the future, or used to improve the appearance 
// and feel of the panner buttons in drawing editors other than idraw.
class OvMover : public Adjuster {
public:
    OvMover(Interactor*, int delay, int moveType);
    OvMover(const char*, Interactor*, int delay, int moveType);
    virtual ~OvMover();
protected:
    int moveType;
    void AdjustView(Event&);
private:
    void Init(int);
};

//: clone of LeftMover.
class OvLeftMover : public OvMover {
public:
    OvLeftMover(Interactor*, int delay = NO_AUTOREPEAT);
    OvLeftMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    virtual ~OvLeftMover();
private:
    void Init();
};

//: clone of RightMover.
class OvRightMover : public OvMover {
public:
    OvRightMover(Interactor*, int delay = NO_AUTOREPEAT);
    OvRightMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    virtual ~OvRightMover();
private:
    void Init();
};

//: clone of UpMover.
class OvUpMover : public OvMover {
public:
    OvUpMover(Interactor*, int delay = NO_AUTOREPEAT);
    OvUpMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    virtual ~OvUpMover();
private:
    void Init();
};

//: clone of DownMover.
class OvDownMover : public OvMover {
public:
    OvDownMover(Interactor*, int delay = NO_AUTOREPEAT);
    OvDownMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    virtual ~OvDownMover();
private:
    void Init();
};

#include <IV-2_6/_leave.h>

#endif
