/*
 * Copyright (c) 1994 Vectaport Inc.
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

#include <OverlayUnidraw/ovdialog.h>
#include <Unidraw/globals.h>

#include <InterViews/box.h>
#include <InterViews/button.h>
#include <InterViews/event.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/matcheditor.h>
#include <InterViews/sensor.h>

#include <OS/math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
using namespace std;

/*****************************************************************************/

PanDialog::PanDialog () : BasicDialog(
    new ButtonState, "", "Enter X and Y pan values:"
) {
    _medit = new MatchEditor(state, "9999999999999999999");
    _medit->Message("");
    _medit->Match("%f %f", false);

    input = new Sensor;
    Ref(input);
    input->Catch(KeyEvent);

    Insert(Interior());
    SelectMessage();
}

void PanDialog::Handle (Event& e) { _medit->Handle(e); }

boolean PanDialog::Accept () {
    Event e;
    int v = 0;

    state->SetValue(0);
    _medit->Edit();
    state->GetValue(v);

    while (v == 0) {
	Read(e);
	Forward(e);
	state->GetValue(v);
    }

    return v == '\r';
}

void PanDialog::SelectMessage () {
    _medit->Select(0, strlen(_medit->Text()));
}

Interactor* PanDialog::Interior () {
    const int space = Math::round(.5*ivcm);

    VBox* titleblock = new VBox(
        new HBox(_title, new HGlue),
        new HBox(_subtitle, new HGlue)
    );

    return new MarginFrame(
        new VBox(
            titleblock,
            new VGlue(space),
            new Frame(new MarginFrame(_medit, 2)),
            new VGlue(space),
            new HBox(
                new HGlue,
                new PushButton("Cancel", state, '\007'),
		new HGlue(space, 0),
                new PushButton("  OK  ", state, '\r')
            )
        ), space, space/2, 0
    );
}
    
void PanDialog::GetValues (float& x, float& y) {
    char* movement = nil;
    movement = strdup(_medit->Text());

    if (sscanf(movement,"%f %f",&x, &y) != 2) {
	x = y = 1.0;
    }
    delete movement;
}

/*****************************************************************************/

ZoomDialog::ZoomDialog () : BasicDialog(
    new ButtonState, "", "Enter zoom factor:"
) {
    _medit = new MatchEditor(state, "9999999999999999999");
    _medit->Message("");
    _medit->Match("%f", false);

    input = new Sensor;
    Ref(input);
    input->Catch(KeyEvent);

    Insert(Interior());
    SelectMessage();
}

void ZoomDialog::Handle (Event& e) { _medit->Handle(e); }

boolean ZoomDialog::Accept () {
    Event e;
    int v = 0;

    state->SetValue(0);
    _medit->Edit();
    state->GetValue(v);

    while (v == 0) {
	Read(e);
	Forward(e);
	state->GetValue(v);
    }

    return v == '\r';
}

void ZoomDialog::SelectMessage () {
    _medit->Select(0, strlen(_medit->Text()));
}

Interactor* ZoomDialog::Interior () {
    const int space = Math::round(.5*ivcm);

    VBox* titleblock = new VBox(
        new HBox(_title, new HGlue),
        new HBox(_subtitle, new HGlue)
    );

    return new MarginFrame(
        new VBox(
            titleblock,
            new VGlue(space),
            new Frame(new MarginFrame(_medit, 2)),
            new VGlue(space),
            new HBox(
                new HGlue,
                new PushButton("Cancel", state, '\007'),
		new HGlue(space, 0),
                new PushButton("  OK  ", state, '\r')
            )
        ), space, space/2, 0
    );
}
    
void ZoomDialog::GetValue (float& angle) {
    char* movement = nil;
    movement = strdup(_medit->Text());

    if (sscanf(movement,"%f",&angle) != 1) {
	angle = 0.0;
    }
    delete movement;
}

/*****************************************************************************/

PageDialog::PageDialog () : BasicDialog(
    new ButtonState, "", "Enter Page width and height values:"
) {
    _medit = new MatchEditor(state, "9999999999999999999");
    _medit->Message("");
    _medit->Match("%f %f", false);

    input = new Sensor;
    Ref(input);
    input->Catch(KeyEvent);

    Insert(Interior());
    SelectMessage();
}

void PageDialog::Handle (Event& e) { _medit->Handle(e); }

boolean PageDialog::Accept () {
    Event e;
    int v = 0;

    state->SetValue(0);
    _medit->Edit();
    state->GetValue(v);

    while (v == 0) {
	Read(e);
	Forward(e);
	state->GetValue(v);
    }

    return v == '\r';
}

void PageDialog::SelectMessage () {
    _medit->Select(0, strlen(_medit->Text()));
}

Interactor* PageDialog::Interior () {
    const int space = Math::round(.5*ivcm);

    VBox* titleblock = new VBox(
        new HBox(_title, new HGlue),
        new HBox(_subtitle, new HGlue)
    );

    return new MarginFrame(
        new VBox(
            titleblock,
            new VGlue(space),
            new Frame(new MarginFrame(_medit, 2)),
            new VGlue(space),
            new HBox(
                new HGlue,
                new PushButton("Cancel", state, '\007'),
		new HGlue(space, 0),
                new PushButton("  OK  ", state, '\r')
            )
        ), space, space/2, 0
    );
}
    
void PageDialog::GetValues (float& x, float& y) {
    char* movement = nil;
    movement = strdup(_medit->Text());

    if (sscanf(movement,"%f %f",&x, &y) != 2) {
	x = y = 1.0;
    }
    delete movement;
}

