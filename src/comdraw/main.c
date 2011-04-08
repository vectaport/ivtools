/*
 * Copyright (c) 1994-2000 Vectaport, Inc.
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
 * comdraw main program.
 */

#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovcreator.h>
#include <ComUnidraw/comeditor.h>

#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovunidraw.h>

#include <Unidraw/Commands/edit.h>
#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>

#include <InterViews/world.h>

#ifdef HAVE_ACE
#include <ComUnidraw/comterp-acehandler.h>
#include <OverlayUnidraw/aceimport.h>
#include <AceDispatch/ace_dispatcher.h>
#endif

#include <stream.h>
#include <string.h>
#include <math.h>
#include <version.h>

static int nmsg = 0;

/*****************************************************************************/

static PropertyData properties[] = {
    { "*ComEditor*name", "comdraw" },
    { "*ComEditor*iconName", "comdraw" },
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
    { "*history",	"20" },
    { "*color5",        "false" },
    { "*color6",        "false" },
    { "*gray5",         "false" },
    { "*gray6",         "false" },
    { "*gray7",         "false" },
    { "*pagecols",      "0" },
    { "*pagerows",      "0" },
    { "*panner_off",    "false"  },
    { "*panner_align",  "br"  },
    { "*ramp_size",     "20"  },
    { "*scribble_pointer", "false" },
    { "*slider_off",    "false"  },
    { "*theight",       "512" },
    { "*tile",          "false" },
    { "*toolbarloc",    "l"  },
    { "*twidth",        "512" },
    { "*zoomer_off",    "false"  },
    { "*opaque_off",    "false"  },
    { "*stripped",        "false"  },
#ifdef HAVE_ACE
    { "*import",        "20001" },
    { "*comdraw",       "20002" },
    { "*wbmaster",      "false" },
    { "*wbslave",       "false" },
    { "*wbhost",        "localhost" },
    { "*wbport",        "20002" },
#endif
    { "*help",          "false"  },
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
    { "-tile", "*tile", OptionValueImplicit, "true" },
    { "-toolbarloc", "*toolbarloc", OptionValueNext },
    { "-tw", "*theight", OptionValueNext },
    { "-tw", "*twidth", OptionValueNext },
    { "-twidth", "*twidth", OptionValueNext },
    { "-zoff", "*zoomer_off", OptionValueImplicit, "true" },
    { "-zoomer_off", "*zoomer_off", OptionValueImplicit, "true" },
    { "-opaque_off", "*opaque_off", OptionValueImplicit, "true" },
    { "-opoff", "*opaque_off", OptionValueImplicit, "true" },
    { "-stripped", "*stripped", OptionValueImplicit, "true" },
#ifdef HAVE_ACE
    { "-import", "*import", OptionValueNext },
    { "-comdraw", "*comdraw", OptionValueNext },
    { "-wbmaster", "*wbmaster", OptionValueImplicit, "true" },
    { "-wbslave", "*wbslave", OptionValueImplicit, "true" },
    { "-wbhost", "*wbhost", OptionValueNext },
    { "-wbport", "*wbport", OptionValueNext },
#endif
    { "-help", "*help", OptionValueImplicit, "true" },
    { "-font", "*font", OptionValueNext },
    { nil }
};

#ifdef HAVE_ACE
static char* usage =
"Usage: comdraw [any idraw parameter] [-comdraw port] [-color5] \n\
[-color6] [-import portnum] [-gray5] [-gray6] [-gray7] [-opaque_off|-opoff] \n\
[-pagecols|-ncols n] [-pagerows|-nrows n] [-panner_off|-poff] \n\
[-panner_align|-pal tl|tc|tr|cl|c|cr|cl|bl|br|l|r|t|b|hc|vc] \n\
[-rampsize n ] [-scribble_pointer|-scrpt ] [-slider_off|-soff] [-stripped]\n\
[-toolbarloc|-tbl r|l ] [-theight|-th n] [-tile] [-twidth|-tw n] \n\
[-wbhost host] [-wbmaster] [-wbslave] [-wbport port] [-zoomer_off|-zoff] [file]";
#else
static char* usage =
"Usage: comdraw [any idraw parameter] [-color5] \n\
[-color6] [-gray5] [-gray6] [-gray7] [-opaque_off|-opoff] \n\
[-pagecols|-ncols n] [-pagerows|-nrows n] [-panner_off|-poff] \n\
[-panner_align|-pal tl|tc|tr|cl|c|cr|cl|bl|br|l|r|t|b|hc|vc] \n\
[-rampsize n ] [-scribble_pointer|-scrpt ] [-slider_off|-soff] [-stripped]\n\
[-toolbarloc|-tbl r|l ] [-theight|-th n] [-tile] [-twidth|-tw n] \n\
[-zoomer_off|-zoff] [file]";
#endif

/*****************************************************************************/

int main (int argc, char** argv) {
#ifdef HAVE_ACE
    Dispatcher::instance(new AceDispatcher(IMPORT_REACTOR::instance()));
#endif
    OverlayCreator creator;
    OverlayCatalog* catalog = new OverlayCatalog("comdraw", &creator);
    OverlayUnidraw* unidraw = new OverlayUnidraw(
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
        cerr << "comdraw:  unable to open import port " << importnum << "\n";

    else if (COMTERP_REACTOR::instance ()->register_handler 
	     (import_acceptor, ACE_Event_Handler::READ_MASK) == -1)
        cerr << "comdraw:  unable to register UnidrawImportAcceptor with ACE reactor\n";
    else
        cerr << "accepting import port (" << importnum << ") connections\n";


    // Acceptor factory.
    UnidrawComterpAcceptor* peer_acceptor = new UnidrawComterpAcceptor();

    const char* portstr = catalog->GetAttribute("comdraw");
    int portnum = atoi(portstr);
    if (peer_acceptor->open 
	(ACE_INET_Addr (portnum)) == -1)
        cerr << "comdraw:  unable to open port " << portnum << "\n";

    else if (COMTERP_REACTOR::instance ()->register_handler 
	     (peer_acceptor, ACE_Event_Handler::READ_MASK) == -1)
        cerr << "comdraw:  unable to register ComterpAcceptor with ACE reactor\n";
    else
        cerr << "accepting comdraw port (" << portnum << ") connections\n";


    // Register COMTERP_QUIT_HANDLER to receive SIGINT commands.  When received,
    // COMTERP_QUIT_HANDLER becomes "set" and thus, the event loop below will
    // exit.
    if (COMTERP_REACTOR::instance ()->register_handler 
	     (SIGINT, COMTERP_QUIT_HANDLER::instance ()) == -1)
        cerr << "comdraw:  unable to register quit handler with ACE reactor\n";

#endif
    int exit_status = 0;

    if (argc > 2) {
	cerr << usage << "\n";
	exit_status = 1;

    } else {
	const char* initial_file = (argc == 2) ? argv[1] : nil;
	ComEditor* ed = new ComEditor(initial_file);

	unidraw->Open(ed);

#ifdef HAVE_ACE
	/*  Start up one on stdin */
	UnidrawComterpHandler* stdin_handler = new UnidrawComterpHandler();
#if 0
	if (ACE::register_stdin_handler(stdin_handler, COMTERP_REACTOR::instance(), nil) == -1)
#else
	if (COMTERP_REACTOR::instance()->register_handler(0, stdin_handler, 
							  ACE_Event_Handler::READ_MASK)==-1)
#endif
	  cerr << "comdraw: unable to open stdin with ACE\n";
	ed->SetComTerp(stdin_handler->comterp());
#endif

	fprintf(stderr, "ivtools-%s comdraw: see \"man comdraw\" or type help here for command info\n", VersionString);
	unidraw->Run();
    }

    delete unidraw;
    return exit_status;
}

