/*
 * Copyright (c) 2004 Scott E. Johnston
 * Copyright (c) 1994-1999 Vectaport, Inc.
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
 * drawserv main program.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ACE
#include <DrawServ/drawserv-handler.h>
#include <OverlayUnidraw/aceimport.h>
#include <AceDispatch/ace_dispatcher.h>
#include <ComTerp/comhandler.h>
#endif

#include <DrawServ/drawcatalog.h>
#include <DrawServ/drawcreator.h>
#include <DrawServ/drawcomps.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawkit.h>
#include <DrawServ/drawserv.h>

#include <GraphUnidraw/grapheditor.h>

#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovunidraw.h>

#include <Unidraw/Commands/edit.h>
#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>

#include <InterViews/world.h>

#include <stream.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>

using std::cerr;

static int nmsg = 0;

static OverlayEditor* launch_comdraw() {
  ComEditor* ed = new ComEditor((const char*)nil, OverlayKit::Instance());
  unidraw->Open(ed);
  return ed;
}

static OverlayEditor* launch_flipbook() {
  FrameEditor* ed = new FrameEditor((const char*)nil, FrameKit::Instance());
  unidraw->Open(ed);
  return ed;
}

static OverlayEditor* launch_graphdraw() {
  GraphEditor* ed = new GraphEditor((const char*)nil, GraphKit::Instance());
  unidraw->Open(ed);
  return ed;
}

/*****************************************************************************/

static PropertyData properties[] = {
    { "*ComEditor*name", "ivtools drawserv" },
    { "*ComEditor*iconName", "ivtools drawserv" },
    { "*domain",  "drawing" },
    { "*TextEditor*rows", "10" },
    { "*TextEditor*columns", "40" },
    { "*TextEditor*FileChooser*rows", "10" },
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
    { "*brush1",	"none" },
    { "*brush2",	"ffff 0" },
    { "*brush3",	"ffff 1" },
    { "*brush4",	"ffff 2" },
    { "*brush5",	"ffff 3" },
    { "*brush6",	"fff0 0" },
    { "*brush7",	"fff0 1" },
    { "*brush8",	"fff0 2" },
    { "*brush9",	"fff0 3" },
    { "*pattern1",	"none" },
    { "*pattern2",	"0.0" },
    { "*pattern3",	"1.0" },
    { "*pattern4",	"0.75" },
    { "*pattern5",	"0.5" },
    { "*pattern6",	"0.25" },
    { "*pattern7",	"1248" },
    { "*pattern8",	"8421" },
    { "*pattern9",	"f000" },
    { "*pattern10",	"8888" },
    { "*pattern11",	"f888" },
    { "*pattern12",	"8525" },
    { "*pattern13",	"cc33" },
    { "*pattern14",	"7bed" },
    { "*fgcolor1",	"Black" },
    { "*fgcolor2",	"Brown 42240 10752 10752" },
    { "*fgcolor3",	"Red" },
    { "*fgcolor4",	"Orange" },
    { "*fgcolor5",	"Yellow" },
    { "*fgcolor6",	"Green" },
    { "*fgcolor7",	"Blue" },
    { "*fgcolor8",	"Indigo 48896 0 65280" },
    { "*fgcolor9",	"Violet 20224 12032 20224" },
    { "*fgcolor10",	"White" },
    { "*fgcolor11",	"LtGray 50000 50000 50000" },
    { "*fgcolor12",	"DkGray 33000 33000 33000" },
    { "*bgcolor1",	"Black" },
    { "*bgcolor2",	"Brown 42240 10752 10752" },
    { "*bgcolor3",	"Red" },
    { "*bgcolor4",	"Orange" },
    { "*bgcolor5",	"Yellow" },
    { "*bgcolor6",	"Green" },
    { "*bgcolor7",	"Blue" },
    { "*bgcolor8",	"Indigo 48896 0 65280" },
    { "*bgcolor9",	"Violet 20224 12032 20224" },
    { "*bgcolor10",	"White" },
    { "*bgcolor11",	"LtGray 50000 50000 50000" },
    { "*bgcolor12",	"DkGray 33000 33000 33000" },
    { "*bgcolor13",	"none" },
    { "*history",	"20" },
    { "*color6",        "false" },
    { "*color5",        "false" },
    { "*gray7",         "false" },
    { "*gray6",         "false" },
    { "*gray5",         "false" },
    { "*tile",          "false" },
    { "*twidth",        "512" },
    { "*theight",       "512" },
    { "*opaque_off",    "false"  },
    { "*stdin_off",   "false"  },
#ifdef HAVE_ACE
    { "*import",        "20001" },
    { "*comdraw",          "20002" },
#endif
    { "*font",          "-adobe-helvetica-medium-r-normal--14-140-75-75-p-77-iso8859-1"  },
    { nil }
};

static OptionDesc options[] = {
    { "-color6", "*color6", OptionValueImplicit, "true" },
    { "-color5", "*color5", OptionValueImplicit, "true" },
    { "-gray7", "*gray7", OptionValueImplicit, "true" },
    { "-gray6", "*gray6", OptionValueImplicit, "true" },
    { "-gray5", "*gray5", OptionValueImplicit, "true" },
    { "-tile", "*tile", OptionValueImplicit, "true" },
    { "-twidth", "*twidth", OptionValueNext },
    { "-theight", "*theight", OptionValueNext },
    { "-opaque_off", "*opaque_off", OptionValueImplicit, "true" },
    { "-opoff", "*opaque_off", OptionValueImplicit, "true" },
    { "-stdin_off", "*stdin_off", OptionValueImplicit, "true" },
#ifdef HAVE_ACE
    { "-import", "*import", OptionValueNext },
    { "-comdraw", "*comdraw", OptionValueNext },
#endif
    { "-font", "*font", OptionValueNext },
    { nil }
};

/*****************************************************************************/

void handle_badpipe(int i) {
  fprintf(stderr, "broken pipe detected %d\n", i);
  return;
}

int main (int argc, char** argv) {

#if 0
    /* ignore broken pipe, so socket writes that are in error return EPIPE */
#if 0
    struct sigaction oldaction, newaction;
    newaction.sa_handler = SIG_IGN;
    newaction.sa_mask = 0;
    newaction.sa_flags = 0;
    newaction.sa_sigaction = 0;
    int status = sigaction(SIGPIPE, &newaction, &oldaction); 
    fprintf(stderr, "sigaction status %d  errno %d\n", status, errno);
#else
    void (*func)(int) = nil;
    func = signal(SIGPIPE, &handle_badpipe);
    if (func==SIG_ERR) 
      fprintf(stderr, "SIG_ERR returned from signal, errno = %d\n", errno);
#endif
#endif
  
#ifdef HAVE_ACE
    Dispatcher::instance(new AceDispatcher(ComterpHandler::reactor_singleton()));
#endif
    DrawCreator creator;
    DrawCatalog* catalog = new DrawCatalog("ivtools drawserv", &creator);
    DrawServ* unidraw = new DrawServ(
        catalog, argc, argv, options, properties
    );

#ifdef HAVE_ACE

    UnidrawImportAcceptor* import_acceptor = new UnidrawImportAcceptor();

    const char* importstr = catalog->GetAttribute("import");
    int importnum = atoi(importstr);
    if (import_acceptor->open 
	(ACE_INET_Addr (importnum)) == -1)
        cerr << "drawserv:  unable to open import port " << importnum << "\n";

    else if (ComterpHandler::reactor_singleton()->register_handler 
	     (import_acceptor, ACE_Event_Handler::READ_MASK) == -1)
        cerr << "drawserv:  unable to register UnidrawImportAcceptor with ACE reactor\n";
    else
        cerr << "accepting import port (" << importnum << ") connections\n";


    // Acceptor factory.
    DrawServAcceptor* peer_acceptor = new DrawServAcceptor();

    const char* portstr = catalog->GetAttribute("comdraw");
    int portnum = atoi(portstr);
    if (peer_acceptor->open 
	(ACE_INET_Addr (portnum), ComterpHandler::reactor_singleton()) == -1)
        cerr << "drawserv:  unable to open port " << portnum << "\n";

    else if (ComterpHandler::reactor_singleton()->register_handler 
	     (peer_acceptor, ACE_Event_Handler::READ_MASK) == -1)
        cerr << "drawserv:  unable to register ComterpAcceptor with ACE reactor\n";
    else
        cerr << "accepting comdraw port (" << portnum << ") connections\n";


    // Register COMTERP_QUIT_HANDLER to receive SIGINT commands.  When received,
    // COMTERP_QUIT_HANDLER becomes "set" and thus, the event loop below will
    // exit.
#if 0
    if (ComterpHandler::reactor_singleton()->register_handler 
	     (SIGINT, COMTERP_QUIT_HANDLER::instance ()) == -1)
        cerr << "drawserv:  unable to register quit handler with ACE reactor\n";
#endif

#endif

    OverlayEditor::add_edlauncher("Comdraw", &launch_comdraw);
    OverlayEditor::add_edlauncher("Flipbook", &launch_flipbook);
    OverlayEditor::add_edlauncher("Graphdraw", &launch_graphdraw);


    int exit_status = 0;

    if (argc > 2) {
	cerr << "Usage: drawserv [file]" << "\n";
	exit_status = 1;

    } else {
	const char* initial_file = (argc == 2) ? argv[1] : nil;
	DrawEditor* ed = nil;
	if (initial_file) 
	  ed = new DrawEditor(initial_file, DrawKit::Instance());
	else 
	  ed = new DrawEditor(new DrawIdrawComp, DrawKit::Instance());

	unidraw->Open(ed);

#ifdef HAVE_ACE
	/*  Start up one on stdin */
	DrawServHandler* stdin_handler = new DrawServHandler();
#if 0
	if (ACE::register_stdin_handler(stdin_handler, ComterpHandler::reactor_singleton(), nil) == -1)
#else
	if (ComterpHandler::reactor_singleton()->register_handler(0, stdin_handler, 
							  ACE_Event_Handler::READ_MASK)==-1)
#endif
	  cerr << "drawserv: unable to open stdin with ACE\n";
	ed->SetComTerp(stdin_handler->comterp());
	fprintf(stderr,
		"ivtools-%s drawserv: type help here for command info\n",
		PACKAGE_VERSION);
#else
	fprintf(stderr, "ivtools-%s drawserv", PACKAGE_VERSION);
#endif

	unidraw->Run();
    }

    delete unidraw;
    return exit_status;
}

