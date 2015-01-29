//
// Simple Text Editor Buffer Implementation
//
// Copyright (C) 1993 Ellemtel Telecommunication Systems Labratories
//
// Permission is granted to any individual or institution to use, copy,
// modify, and distribute this software, provided that this complete
// copyright and permission notice is maintained, intact, in all copies
// and supporting documentation.
//
// Ellemtel Telecommunication Systems Labratories make no representation
// of the suitability of this software for any purpose. It is provided
// "as is" without any expressed or implied warranty.
//
// Jan Andersson, Torpa Konsult AB
// janne@torpa.se - 1993-08-29

// Sub-classing ApplicationWindow to receive clipboard
// selection events. Window::receive does not support this (!?)

#ifndef textwindow_h
#define textwindow_h

#include <InterViews/window.h>

class TextEditAppWindow : public ApplicationWindow {
public:
   TextEditAppWindow(Glyph*);
   virtual void receive(const Event&);
};

#endif
