/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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

#include <FrameUnidraw/framestates.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <IV-look/kit.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

FrameNumberState::FrameNumberState(int fn, const char* desc, int usebg)
:NameState(nil)
{
    _number = fn;
    _desc = strdup (desc ? desc : "Current Frame");
    _usebg = usebg;
    if (_usebg && fn == 0)
	sprintf(buf, "%s: background", _desc);
    else
        sprintf(buf, "%s: %d", _desc, _number);
    name(buf, false);
}

int FrameNumberState::number() { return _number; }

void FrameNumberState::number(int fn, boolean notif) {
    _number = fn;
    if (_usebg && fn == 0)
	sprintf(buf, "%s: background", _desc);
    else
        sprintf(buf, "%s: %d", _desc, _number);
    name(buf, notif);
}

int FrameNumberState::framenumber() { return number(); }
void FrameNumberState::framenumber(int fn, boolean notif) 
{ number(fn, notif); }

/*****************************************************************************/

FrameListState::FrameListState(int fn)
:NameState(nil)
{
    _framenumber = fn;
    sprintf(buf, "Number of Frames: %d", _framenumber-1);
    name(buf, false);
}

int FrameListState::framenumber() { return _framenumber; }

void FrameListState::framenumber(int fn, boolean notif) {
    _framenumber = fn;
    sprintf(buf, "Number of Frames: %d", _framenumber-1);
    name(buf, notif);
}


