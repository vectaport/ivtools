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

#include <DrawServ/drawstates.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <Unidraw/iterator.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

DraftShareState::DraftShareState(boolean paused)
:NameState(nil)
{
    _paused = paused;
    snprintf(buf, sizeof(buf), "%s", _paused ? "Draft" : "Share");
    name(buf, false);
}

void DraftShareState::paused(boolean paused, boolean notif) {
    _paused = paused;
    snprintf(buf, sizeof(buf), "%s", _paused ? "Draft" : "Share");
    name(buf, notif);
}

/*****************************************************************************/

DrawLinkListState::DrawLinkListState()
:NameState(nil)
{
    snprintf(buf, sizeof(buf), "Links: none");
    name(buf, false);
}

void DrawLinkListState::links(DrawLinkList* linklist, boolean notif) {
    if (!linklist || linklist->IsEmpty()) {
        snprintf(buf, sizeof(buf), "Links: none");
    } else {
        char hosts[200];
        hosts[0] = '\0';
        Iterator it;
        linklist->First(it);
        boolean first = true;
        while (!linklist->Done(it)) {
            DrawLink* link = linklist->GetDrawLink(it);
            if (!first) strncat(hosts, ",", sizeof(hosts)-strlen(hosts)-1);
            strncat(hosts, link->hostname(), sizeof(hosts)-strlen(hosts)-1);
            first = false;
            linklist->Next(it);
        }
        snprintf(buf, sizeof(buf), "Links (%d): %s", linklist->Number(), hosts);
    }
    name(buf, notif);
}
