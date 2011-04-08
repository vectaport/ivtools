//
// Simple Text Editor
//
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

#ifndef eiv_texteditor_h
#define eiv_texteditor_h

// Properties defined for this class
//
// Property                    Default value
// ----------------------------------------------
// TextEditor*rows             24
// TextEditor*columns          80
// TextEditor*textFont         fixed
// 

typedef unsigned int EivTextUnit;

class TE_Adjustable;
class TE_View;

class EivTextEditor : public MonoGlyph {
public:
   EivTextEditor(Style*, boolean active= true);
   virtual ~EivTextEditor();

   // load/save file. Default for save is to save to current
   // file name (specified by load())
   int load(const char* path);
   int save(const char* path = nil);

   // popup window to allow user to load/save a file
   void load_popup();
   void save_popup();

   // Selections

   // get dot and mark indices
   int dot();
   int mark();

   // modify selection in term of text indices
   void select(const int dot, const int mark);
   void select_all();

   // move the current selection forward or backward the specified
   // unit of text.
   enum { Character, Word, Line, Text };
   void select_beginning(const EivTextUnit);
   void select_end(const EivTextUnit);

   // move the current selection forward or backward the specified numbers
   // of the specified units.
   void select_backward(const EivTextUnit, const int count);
   void select_forward(const EivTextUnit, const int count);

   // search for regular expression specified by pattern
   // match is selected
   void find_forward(const char* pattern);
   void find_backward(const char* pattern);

   // Yeah, I know. There are useful functions missing here...


    // Text get and set methods
    const char* text();
    void text(const char*, boolean update =true);

    void insert_string(char* str, int count);

    InputHandler* focusable();
    TE_View* textview();

protected:
   TE_Adjustable* te_adjustable_;
   TE_View*   te_view_;
   Glyph* sb_;
   Style* style_;
};

#endif

