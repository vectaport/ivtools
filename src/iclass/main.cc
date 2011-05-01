/*
 * Copyright (c) 1989 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * InterViews class browser main program.
 */

#include "classbuffer.h"
#include "iclass.h"

#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <InterViews/world.h>

#include <string.h>
#include <iostream.h>
#include <fstream>

using std::cout;

/*****************************************************************************/

static PropertyData properties[] = {
    { "*Message*font",              "*helvetica-bold-r-normal--10*" },
    { "*Dialog*Message*font",       "*helvetica-bold-r-normal--12*" },
    { "*path*font",                 "*helvetica-bold-r-normal--12*" },
    { "*PulldownMenu*Message*font", "*helvetica-bold-r-normal--12*" },
    { "*PushButton*font",           "*helvetica-bold-r-normal--12*" },
    { "*StringBrowser*font",        "*helvetica-bold-r-normal--10*" },
    { "*dirBrowser*font",           "*helvetica-bold-r-normal--10*" },
    { "*StringEditor*font",         "*helvetica-bold-r-normal--10*" },
    { "*recursive",                 "false" },
    { "*verbose",                   "false" },
    { "*CPlusPlusFiles",            "false" },
    { "*DerivedClassGraph",         "false" },
    { "*DerivedClassTree",          "false" },
    { "*showButton",                "true" },
    { "*dirBrowser*singleClick",    "on" },
    { nil }
};

static OptionDesc options[] = {
    { "-r", "*recursive", OptionValueImplicit, "true" },
    { "-v", "*verbose", OptionValueImplicit, "true" },
    { "-c", "*CPlusPlusFiles", OptionValueImplicit, "true" },
    { "-d", "*DerivedClassGraph", OptionValueImplicit, "true" },
    { "-t", "*DerivedClassTree", OptionValueImplicit, "true" },
    { nil }
};

/*****************************************************************************/

void print_class_subtree(ostream& out, ClassBuffer* cbuffer, const char* classname, int depth) {
  
  for(int i=0; i<depth; i++) out << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

  out << "<a name=" << classname << " href=" << classname << ".html>" 
      << classname << "</a><br>\n";

  const char* childname;
  char* prevname=strdup("");
  int childindex=0;
  while (childname = cbuffer->Child(classname, childindex)) {
    if (strcmp(prevname, childname)!=0) 
      print_class_subtree(out, cbuffer, childname, depth+1);
    delete prevname;
    prevname = strdup(childname);
    childindex++;
  }
}

/*****************************************************************************/

int main (int argc, char** argv) {
    World world("iclass", argc, argv, options, properties);
    const char* recursive = world.GetAttribute("recursive");
    const char* verbose = world.GetAttribute("verbose");
    const char* CPlusPlusFiles = world.GetAttribute("CPlusPlusFiles");
    const char* DerivedClassGraph = world.GetAttribute("DerivedClassGraph");
    const char* DerivedClassTree = world.GetAttribute("DerivedClassTree");
    ClassBuffer* buffer = new ClassBuffer(
        strcmp(recursive, "true") == 0, strcmp(verbose, "true") == 0,
	strcmp(CPlusPlusFiles, "true") == 0
    );

    for (int i = 1; i < argc; ++i) {
        buffer->Search(argv[i]);
    }

    if (strcmp(DerivedClassGraph, "true")==0) {
      const char* classname;
      int classindex = 0;
      while (classname = buffer->Class(classindex)) {
	classindex++;
	cout << "<a name=" << classname << " href=" << classname << ".html>" 
	  << classname << "</a>:<br>\n";
	const char* childname;
        char* prevname=strdup("");
	int childindex = 0;
	while (childname = buffer->Child(classname, childindex)) {
	  childindex++;
	  if (strcmp(prevname, childname)!=0)
	    cout << "<li> <a href=" << childname << ".html>" << childname << "</a></li>\n";
	  delete prevname;
	  prevname = strdup(childname);
	}
	cout << "<p>\n";
      }
      return 0;
    }

    if (strcmp(DerivedClassTree, "true")==0) {
      const char* classname;
      int classindex = 0;
      while (classname = buffer->Class(classindex)) {
	const char* parent = buffer->Parent(classname);
	if (!parent) print_class_subtree(cout, buffer, classname, 0);
	classindex++;
      }
      return 0;
    }

    IClass* iclass = new IClass(buffer);
    ApplicationWindow* window = new ApplicationWindow(iclass);
    Style* s = new Style(Session::instance()->style());
    s->attribute("name", "InterViews class browser");
    s->attribute("iconName", "iclass");
    window->style(s);
    window->map();
    iclass->Run();
    return 0;
}
