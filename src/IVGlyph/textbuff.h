//
// Simple Text Editor Buffer
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

#ifndef eiv_textbuffer_h
#define eiv_textbuffer_h

// sub-class of TextBuffer to enable dynamic expansion of
// text and initialization from files.

#include <IV-2_6/InterViews/textbuffer.h>

class CopyString;
class EivTextBuffer : public TextBuffer {
 public:
   EivTextBuffer();
   ~EivTextBuffer();

   // load/save buffer from/into file
   enum { OpenError, MemoryError, ReadError, ReadOk, WriteError, WriteOk };
   int load(const char* path);
   int save();
   int save(const char* path);
   
   // Search for a match with the regular regular expression defined in
   // pattern. Returns index to the end (forward) or the beginning (backward)
   // of the match. Returns a negative number if there was no match.
   int search_forward(const char* pattern, int index);
   int search_backward(const char* pattern, int index);
   // returns start or end position of last search
   int search_beginning();
   int search_end();
   
   virtual int Insert(int index, const char* string, int count);
   virtual int Delete(int index, int count);
   
   // various access functions
   boolean buffer_modified();

   int lines();			// lines in buffer
   int characters();		// characters in buffer
   int allocated_size();	// allocated size

   int line(int);		// index to line
   int column(int);		// index to column


   void righttrim();				     
 private:
   void expand_buffer(int);
   int linecount;
   boolean saved_;
   boolean modified_;
   CopyString* path_;
   Regexp* current_regexp_;
};

inline boolean EivTextBuffer::buffer_modified()
{
   return modified_;
}

inline int EivTextBuffer::lines()
{
  return Height();
}

inline int EivTextBuffer::characters()
{
   return Length();
} 

inline int EivTextBuffer::allocated_size()
{
   return size;
} 

inline int EivTextBuffer::line(int index)
{
   return LineNumber(index);
}

inline int EivTextBuffer::column(int index)
{
   return LineOffset(index);
}

#endif
