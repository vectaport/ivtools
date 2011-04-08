/*
 * Copyright 1995 Vectaport Inc.
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
 */

/*
 *  Copyright (C) 1993 Ellemtel Telecommunication Systems Labratories
 * 
 *  Permission is granted to any individual or institution to use, copy,
 *  modify, and distribute this software, provided that this complete
 *  copyright and permission notice is maintained, intact, in all copies
 *  and supporting documentation.
 * 
 *  Ellemtel Telecommunication Systems Labratories make no representation
 *  of the suitability of this software for any purpose. It is provided
 *  "as is" without any expressed or implied warranty.
 * 
 */

#include <IV-look/kit.h>

#include <ComGlyph/terpdialog.h>

#include <IVGlyph/textedit.h>
#include <IVGlyph/textwindow.h>

#include <InterViews/display.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>

#include <stdio.h>

/*****************************************************************************/

static PropertyData properties[] = {
   { "ExprEdit*iconName", "Expr Editor" },
   { "ExprEdit*title", "Expression Editor" },
   { "ExprEdit*TextEditor*rows", "10" },
   { "ExprEdit*TextEditor*columns", "40" },
//   { "ExprEdit*TextEditor*textFont", "lucidasanstypewriter-14" },
   { "ExprEdit*TextEditor*FileChooser*rows", "10" },
   { nil }
};

static OptionDesc options[] = {
   { "-rows", "ExprEdit*TextEditor*rows", OptionValueNext },
   { nil }
};

int main(int argc, char** argv) {
    Session* session = new Session("ExprEdit", argc, argv, options, properties);
    WidgetKit& kit = *WidgetKit::instance();
    LayoutKit& layout = *LayoutKit::instance();

    PolyGlyph* mainglyph = layout.vbox();
    ApplicationWindow* mainwin = new TextEditAppWindow(mainglyph);

    
    TerpDialog* d = new TerpDialog(true, argc, argv);
    mainglyph->append(d);

    return session->run_window(mainwin);
}



