/*
 * Copyright (c) 2026 Vectaport Inc.
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

#ifndef drawstates_h
#define drawstates_h

#include <IVGlyph/namestate.h>

class DrawLinkList;

//: state variable for DrawServ's draft()/share() distribution mode.
class DraftShareState : public NameState {
public:
    DraftShareState(boolean paused =false);

    boolean paused() { return _paused; }
    void paused(boolean, boolean notif =true);
protected:
    boolean _paused;
    char buf[32];
};

//: state variable for DrawServ's current set of DrawLinks.
class DrawLinkListState : public NameState {
public:
    DrawLinkListState();

    void links(DrawLinkList*, boolean notif =true);
    // format link count and hostnames in compact form, e.g.
    // "Links (2): host1,host2", or "Links: none" when empty.
protected:
    char buf[256];
};

#endif
