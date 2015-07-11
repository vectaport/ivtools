/*
 * Copyright 1990, 1991, 1992 O'Reilly and Associates, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * online documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of O'Reilly
 * and Associates, Inc. not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  O'Reilly and Associates, Inc. makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * O'REILLY AND ASSOCIATES, INC. DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL O'REILLY AND ASSOCIATES, INC.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Adrian Nye of O'Reilly and Associates, Inc.
 */

#include <stdlib.h>

// **
#include "Xd.h"
// **


#include <stream.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>

// **
#include "Xud.h"
// **


// **
#include <OverlayUnidraw/oved.h>
#include "xtudsession.h"
// **


// --------------------------------------------------------------------------


// **
static PropertyData properties[] = {
    { "*IdrawEditor*name", "InterViews drawing editor" },
    { "*IdrawEditor*iconName", "Idraw" },
    { "*domain",  "drawing" },
    { "*initialbrush",  "2" },
    { "*initialfgcolor","1" },
    { "*initialbgcolor","10" },
    { "*initialfont",   "4" },
    { "*initialpattern","1" },
    { "*initialarrow", "none" },
    { "*pagewidth", "8.5" },
    { "*pageheight", "11" },
    { "*gridxincr", "8" },
    { "*gridyincr", "8" },
    { "*font1", "-*-courier-medium-r-normal-*-8-*-*-*-*-*-*-* Courier 8" },
    { "*font2", "-*-courier-medium-r-normal-*-10-*-*-*-*-*-*-* Courier 10" },
    { "*font3", "-*-courier-bold-r-normal-*-12-*-*-*-*-*-*-* Courier-Bold 12" },
    { "*font4",
        "-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-* Helvetica 12"
    },
    { "*font5",
        "-*-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-* Helvetica 14"
    },
    { "*font6",
        "-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-* Helvetica-Bold 14"
    },
    { "*font7",
        "-*-helvetica-medium-o-normal-*-14-*-*-*-*-*-*-* Helvetica-Oblique 14"
    },
    { "*font8",
        "-*-times-medium-r-normal-*-12-*-*-*-*-*-*-*  Times-Roman 12"
    },
    { "*font9", "-*-times-medium-r-normal-*-14-*-*-*-*-*-*-* Times-Roman 14" } ,
    { "*font10", "-*-times-bold-r-normal-*-14-*-*-*-*-*-*-*  Times-Bold 14" },
    { "*font11",
        "-*-times-medium-i-normal-*-14-*-*-*-*-*-*-* Times-Italic 14"
    },
    { "*brush1",        "none" },
    { "*brush2",        "ffff 0" },
    { "*brush3",        "ffff 1" },
    { "*brush4",        "ffff 2" },
    { "*brush5",        "ffff 3" },
    { "*brush6",        "fff0 0" },
    { "*brush7",        "fff0 1" },
    { "*brush8",        "fff0 2" },
    { "*brush9",        "fff0 3" },
    { "*pattern1",      "none" },
    { "*pattern2",      "0.0" },
    { "*pattern3",      "1.0" },
    { "*pattern4",      "0.75" },
    { "*pattern5",      "0.5" },
    { "*pattern6",      "0.25" },
    { "*pattern7",      "1248" },
    { "*pattern8",      "8421" },
    { "*pattern9",      "f000" },
    { "*pattern10",     "8888" },
    { "*pattern11",     "f888" },
    { "*pattern12",     "8525" },
    { "*pattern13",     "cc33" },
    { "*pattern14",     "7bed" },
    { "*fgcolor1",      "Black" },
    { "*fgcolor2",      "Brown 42240 10752 10752" },
    { "*fgcolor3",      "Red" },
    { "*fgcolor4",      "Orange" },
    { "*fgcolor5",      "Yellow" },
    { "*fgcolor6",      "Green" },
    { "*fgcolor7",      "Blue" },
    { "*fgcolor8",      "Indigo 48896 0 65280" },
    { "*fgcolor9",      "Violet 20224 12032 20224" },
    { "*fgcolor10",     "White" },
    { "*fgcolor11",     "LtGray 50000 50000 50000" },
    { "*fgcolor12",     "DkGray 33000 33000 33000" },
    { "*bgcolor1",      "Black" },
    { "*bgcolor2",      "Brown 42240 10752 10752" },
    { "*bgcolor3",      "Red" },
    { "*bgcolor4",      "Orange" },
    { "*bgcolor5",      "Yellow" },
    { "*bgcolor6",      "Green" },
    { "*bgcolor7",      "Blue" },
    { "*bgcolor8",      "Indigo 48896 0 65280" },
    { "*bgcolor9",      "Violet 20224 12032 20224" },
    { "*bgcolor10",     "White" },
    { "*bgcolor11",     "LtGray 50000 50000 50000" },
    { "*bgcolor12",     "DkGray 33000 33000 33000" },
    { "*history",       "20" },
    { nil }
};

static OptionDesc options[] = {
    { nil }
};

// **


// --------------------------------------------------------------------------

void pressMe(
    Widget w, XtPointer client_data, XtPointer call_data
) {
    cerr << "Thankyou!\n";
}


void quitProg(
    Widget w, XtPointer client_data, XtPointer call_data
) {
    cerr << "Bye!\n";
    exit(0);
}


main(int argc, char** argv) {

    XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL);

    XtAppContext app_context;

    Widget topLevel = XtVaAppInitialize(
            &app_context,       /* Application context */
            "hey",              /* Application class */
            NULL, 0,            /* command line option list */
            &argc, argv,        /* command line args */
            NULL,               /* for missing app-defaults file */
            NULL);              /* terminate varargs list */

    Widget rowColumn = XtVaCreateManagedWidget(
            "rowColumn",                /* widget name */
            xmRowColumnWidgetClass,     /* widget class */
            topLevel,                   /* parent widget*/
            NULL);                      /* terminate varargs list */

    Widget quit = XtVaCreateManagedWidget(
            "quit",                     /* widget name */
            xmPushButtonWidgetClass,    /* widget class */
            rowColumn,                  /* parent widget*/
            NULL);                      /* terminate varargs list */


    Widget pressme = XtVaCreateManagedWidget(
            "pressme",                  /* widget name */
            xmPushButtonWidgetClass,    /* widget class */
            rowColumn,                  /* parent widget*/
            NULL);                      /* terminate varargs list */

    XtAddCallback(quit, XmNactivateCallback, quitProg, 0);
    XtAddCallback(pressme, XmNactivateCallback, pressMe, 0);


    Widget frame = XtVaCreateManagedWidget(
            "frame",            /* widget name */
            xmFrameWidgetClass, /* widget class */
            rowColumn,         /* parent widget*/
            NULL);              /* terminate varargs list */

// **
    XtUdSession::initialize(
        "testing", argc, argv, XtDisplay(topLevel), options, properties
    );
    IdrawEditor* ed = new OverlayEditor((const char*)nil);
    XtUnidraw::instance().Open(ed, frame);
// **

    XtRealizeWidget(topLevel);
    XtAppMainLoop(app_context);
}

