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
#include <version.h>
#include <patch.h>
#include <fstream>
#include <iostream>

#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <ComUtil/util.h>

using std::cout;
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
    { "*pagecols",      "0" },
    { "*pagerows",      "0" },
    { "*panner_off",    "false"  },
    { "*panner_align",  "br"  },
    { "*ramp_size",     "20"  },
    { "*scribble_pointer", "false" },
    { "*slider_off",    "false"  },
    { "*tile",          "false" },
    { "*twidth",        "512" },
    { "*theight",       "512" },
    { "*toolbarloc",    "l"  },
    { "*zoomer_off",    "false"  },
    { "*opaque_off",    "false"  },
    { "*stripped",      "false"  },
    { "*stdin_off",   "false"  },
#ifdef HAVE_ACE
    { "*import",        "20001" },
    { "*comdraw",          "20002" },
#endif
    { "*help",          "false"  },
    { "*runfile",       ""  },
    { "*runexpr",       ""  },
    { "*font",          "-adobe-helvetica-medium-r-normal--14-140-75-75-p-77-iso8859-1"  },
    { nil }
};

static OptionDesc options[] = {
    { "-color5", "*color5", OptionValueImplicit, "true" },
    { "-color6", "*color6", OptionValueImplicit, "true" },
    { "-gray5", "*gray5", OptionValueImplicit, "true" },
    { "-gray6", "*gray6", OptionValueImplicit, "true" },
    { "-gray7", "*gray7", OptionValueImplicit, "true" },
    { "-ncols", "*pagecols", OptionValueNext },
    { "-nrows", "*pagerows", OptionValueNext },
    { "-pagecols", "*pagecols", OptionValueNext },
    { "-pagerows", "*pagerows", OptionValueNext },
    { "-pal", "*panner_align", OptionValueNext },
    { "-panner_align", "*panner_align", OptionValueNext },
    { "-panner_off", "*panner_off", OptionValueImplicit, "true" },
    { "-poff", "*panner_off", OptionValueImplicit, "true" },
    { "-rampsize", "*rampsize", OptionValueNext },
    { "-scribble_pointer", "*scribble_pointer", OptionValueImplicit, "true" },
    { "-scrpt", "*scribble_pointer", OptionValueImplicit, "true" },
    { "-slider_off", "*slider_off", OptionValueImplicit, "true" },
    { "-soff", "*slider_off", OptionValueImplicit, "true" },
    { "-tbl", "*toolbarloc", OptionValueNext },
    { "-theight", "*theight", OptionValueNext },
    { "-th", "*theight", OptionValueNext },
    { "-tile", "*tile", OptionValueImplicit, "true" },
    { "-toolbarloc", "*toolbarloc", OptionValueNext },
    { "-twidth", "*twidth", OptionValueNext },
    { "-tw", "*twidth", OptionValueNext },
    { "-zoff", "*zoomer_off", OptionValueImplicit, "true" },
    { "-zoomer_off", "*zoomer_off", OptionValueImplicit, "true" },
    { "-opaque_off", "*opaque_off", OptionValueImplicit, "true" },
    { "-opoff", "*opaque_off", OptionValueImplicit, "true" },
    { "-stripped", "*stripped", OptionValueImplicit, "true" },
    { "-stdin_off", "*stdin_off", OptionValueImplicit, "true" },
#ifdef HAVE_ACE
    { "-import", "*import", OptionValueNext },
    { "-comdraw", "*comdraw", OptionValueNext },
#endif
    { "-help", "*help", OptionValueImplicit, "true" },
    { "--help", "*help", OptionValueImplicit, "true" },
    { "-font", "*font", OptionValueNext },
    { "-runfile", "*runfile", OptionValueNext },
    { "-runexpr", "*runexpr", OptionValueNext },
    { nil }
};

/*****************************************************************************/

#ifdef HAVE_ACE
static const char usage[] =
"drawserv  distributed drawing editor with comterp scripting\n\
Usage:  drawserv [file] [options]\n\n\
-comdraw port               port number for comdraw command socket\n\
-import portnum             port number for import socket\n\
-color5 | -color6           use 5x5x5 or 6x6x6 color cube\n\
-gray5 | -gray6 | -gray7    use 5, 6, or 7 level grayscale ramp\n\
-opaque_off | -opoff        disable opaque moving/reshaping\n\
-pagecols | -ncols n        number of page columns in tiled view\n\
-pagerows | -nrows n        number of page rows in tiled view\n\
-panner_off | -poff         disable panner\n\
-panner_align | -pal tl|tc|tr|cl|c|cr|bl|bc|br|l|r|t|b|hc|vc\n\
                            panner alignment\n\
-rampsize n                 size of color ramp\n\
-scribble_pointer | -scrpt  enable scribble pointer\n\
-slider_off | -soff         disable slider\n\
-stdin_off                  disable stdin command socket\n\
-stripped                   stripped-down tool palette\n\
-toolbarloc | -tbl r|l      toolbar location left or right\n\
-theight | -th n            tile height in pixels\n\
-tile                       enable tiled page view\n\
-twidth | -tw n             tile width in pixels\n\
-zoomer_off | -zoff         disable zoomer\n\
-runfile file               run script file after startup\n\
-runexpr cmdstr             run command string after startup\n\n\
any idraw parameter is also accepted (see idraw man page)";
#else
static const char usage[] =
"drawserv  distributed drawing editor with comterp scripting\n\
Usage:  drawserv [file] [options]\n\n\
-color5 | -color6           use 5x5x5 or 6x6x6 color cube\n\
-gray5 | -gray6 | -gray7    use 5, 6, or 7 level grayscale ramp\n\
-opaque_off | -opoff        disable opaque moving/reshaping\n\
-pagecols | -ncols n        number of page columns in tiled view\n\
-pagerows | -nrows n        number of page rows in tiled view\n\
-panner_off | -poff         disable panner\n\
-panner_align | -pal tl|tc|tr|cl|c|cr|bl|bc|br|l|r|t|b|hc|vc\n\
                            panner alignment\n\
-rampsize n                 size of color ramp\n\
-scribble_pointer | -scrpt  enable scribble pointer\n\
-slider_off | -soff         disable slider\n\
-stdin_off                  disable stdin command socket\n\
-stripped                   stripped-down tool palette\n\
-toolbarloc | -tbl r|l      toolbar location left or right\n\
-theight | -th n            tile height in pixels\n\
-tile                       enable tiled page view\n\
-twidth | -tw n             tile width in pixels\n\
-zoomer_off | -zoff         disable zoomer\n\
-runfile file               run script file after startup\n\
-runexpr cmdstr             run command string after startup\n\n\
any idraw parameter is also accepted (see idraw man page)";
#endif

/*****************************************************************************/

void handle_badpipe(int i) {
  fprintf(stderr, "broken pipe detected %d\n", i);
  return;
}

int main (int argc, char** argv) {
    /* Ctrl-C (SIGINT) is the common way an interactive session ends --
       restore tty echo first if tty_echo_off() ever ran, issue #76. */
    tty_echo_install_signal_handlers();

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

    if (strcmp(catalog->GetAttribute("help"), "true")==0) {
      cerr << usage << "\n";
      return 0;
    }

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
	cerr << usage << "\n";
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
	/*  Start up one on stdin, unless -stdin_off (mirror comdraw).  Registering
	    a live fd-0 handler under -stdin_off means a closed/EOF stdin -- as when
	    drawmo launches us detached -- fires DrawServHandler::handle_input, which
	    pops the "unexpected EOF" dialog and, if it fires while the seed update()
	    below is pumping the event loop, re-enters comterp on the shared eval
	    stack and corrupts it (SIGSEGV).  honor what -stdin_off documents. */
	const char* stdin_off_str = unidraw->GetCatalog()->GetAttribute("stdin_off");
	DrawServHandler* stdin_handler = nil;
	if (!stdin_off_str || strcmp(stdin_off_str, "false")==0) {
	    stdin_handler = new DrawServHandler();
	    if (ComterpHandler::reactor_singleton()->register_handler(0, stdin_handler,
							  ACE_Event_Handler::READ_MASK)==-1)
	      cerr << "drawserv: unable to open stdin with ACE\n";
	    else
	      tty_echo_off();  // issue #76 -- see ComUtil/ttyecho.c; only if the
	                        // handler is actually live, or OS echo goes off
	                        // with no self-echo ever registered to replace it
	    ed->stdio_setup(stdin_handler);
	}
	fprintf(stderr, "ivtools-%s drawserv: type help here for command info %s\n", VersionString, build_stamp(__DATE__, __TIME__, PATCH_KEY));
	ed->stdio_prompt(stdin_handler);

#else
	fprintf(stderr, "ivtools-%s drawserv", VersionString);
#endif

	/* execute -runfile or -runexpr after editor is fully initialized */
	ComTerpServ* terp = ed->GetComTerp();
	if (terp) {
	    /* Seed update: the mandatory first update() that wins the X11
	       map/realize race (see comdraw/main.c).  It pumps the event loop so the
	       window maps and the viewer's canvas is bound before any GUI command
	       can run.  For a drawserv this guards more than the -runfile/-runexpr
	       path: a freshly-launched node can receive a relayed GUI command over a
	       drawlink (e.g. the select()/brush() fan-out in drawmo's gsbrushB tree)
	       as the very first event it processes inside Run(), before its window's
	       Configure/Expose has bound the canvas -- OverlaySelection::Update()
	       then repairs damage through a null canvas and segfaults.  Mapping the
	       window here, before Run() owns the loop, closes that window.
	       Not something the user typed -- one-shot suppress its self-echo
	       (issue #76, ttyecho.c; see comdraw/main.c's fuller comment for
	       why a one-shot flag, not a held-open disable_prompt(), is the
	       reentrancy-safe way to suppress a single internal eval like
	       this one). */
	    tty_echo_suppress_next();
	    terp->run("update(1000000)\n");

	    const char* runfile = catalog->GetAttribute("runfile");
	    if (runfile && *runfile) {
	        if (terp->runfile(runfile) < 0)
	            cerr << "drawserv: error running script file: " << runfile << "\n";
	    }
	    const char* runexpr = catalog->GetAttribute("runexpr");
	    if (runexpr && *runexpr) {
	        int bufsize;
	        char* runexpr_nl = restore_escapes(runexpr, bufsize);
	        strncat(runexpr_nl, "\n", bufsize - strlen(runexpr_nl) - 1);
	        /* same pattern as comterp's own bare-expression mode (see
	           comterp_/main.c): terp->run(const char*) evaluates and
	           hands the result back in C++ without printing it, so print
	           it explicitly here. */
	        terp->brief(1);
	        ComValue::comterp(terp);
	        ComValue comval(terp->run(runexpr_nl));
	        cout << comval << "\n";
	        cout.flush();
	        if (*terp->errmsg())
	            cerr << "drawserv: error running expression: " << runexpr << "\n";
	        delete [] runexpr_nl;
	    }
	}

	unidraw->Run();
    }

    delete unidraw;
    return exit_status;
}

