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

#if defined(sun) && !defined(solaris) && __GNUC__==2 && !defined(__GNUC_MINOR__)
#define _GCC_SIZE_T /* workaround for _G_size_t conflict */
#endif

#include <stdio.h>
#include <fcntl.h> 
#include <sys/stat.h>
extern "C" {
#include <malloc.h>
}

#include <InterViews/regexp.h>

#include <OS/math.h>
#include <OS/memory.h>
#include <OS/string.h>

#include <stdlib.h>
#include <string.h>

#include "textbuff.h"
#if 0
// Probably not truly portable (?)
extern "C" {
   extern int close(int);
   extern int read(int, char*, unsigned int);
   extern int write(int, char*, unsigned int);
};
#endif
#ifdef __GNUC__
#include <unistd.h>
#endif

// allocate 25% extra space to prepare for expansion without
// the need to expand buffer
static const float allocate_extra = 0.25;

EivTextBuffer::EivTextBuffer()
: TextBuffer((char *)malloc(1024), 0, 1024) // we always allocate 1k buffer
{
   current_regexp_ = nil;
   saved_ = false;
   modified_ = false;
   path_ = nil;
}

EivTextBuffer::~EivTextBuffer()
{
   delete text;
   if (path_ != nil)
      delete path_;
   if (current_regexp_ != nil)
      delete current_regexp_;
}

int EivTextBuffer::load(const char* path)
{
   path_ = new CopyString(path);

   // open file
   int fd;
   fd = open(path_->string(), O_RDONLY);
   if (fd < 0) 
      return OpenError;		// can't open file 

   // get file size
   struct stat info;
   if (fstat(fd, &info) < 0) {
      close(fd);
      return OpenError;		// can't access file
   }
   int len = (int) info.st_size;

   // allocate buffer
   int add_size = int(len * allocate_extra);
   char* buffer = (char *)realloc(text, len + add_size);
   if (buffer == nil) {
      close(fd);
      return MemoryError;	// out of memory
   }

   // read file into buffer
   int bytes_read = read(fd, buffer, len);
   if (bytes_read != len) {
      close(fd);
      return ReadError;		// read failed
   }
   
   // update protected TextBuffer variables
   text = buffer;
   size = bytes_read + add_size;
   // hack to update private TextBuffer variables
   // (length and linecount are updated)
   TextBuffer::Insert(0, buffer, bytes_read);

   close(fd);
   return ReadOk;		// ok
}

void EivTextBuffer ::expand_buffer(int count)
{
   int add_size = int(Math::max(
      count * (1 + allocate_extra),
      size * allocate_extra));;
   // re-allocate
   // printf("EivTextBuffer::allocating more memory\n");
   // printf("\t add:%d bytes\n", add_size);
   char* buffer = (char *)realloc(text, size + add_size);
   if (buffer == nil)
      return;			// quitely ???

   // update protected TextBuffer variables
   text = buffer;
   size += add_size;
}

int EivTextBuffer::save()
{
   printf("save buffer to file: %s\n", path_->string());
   int fd = open((char*)path_->string(), O_WRONLY | O_CREAT, 0666);
   if (fd < 0)
      return OpenError;		// can't open file

   int len = write(fd, text, length);
   if (len != length) {
      perror("EivTextBuffer:save");
      return WriteError;			// can't write to file
   }

   return WriteOk;
}

int EivTextBuffer::save(const char* path)
{
   if (path_ != nil)
      delete path_;
   path_ = new CopyString(path);
   return save();
}

int EivTextBuffer::search_forward(const char* pattern, int index)
{
   if (current_regexp_ != nil)
      delete current_regexp_;
   current_regexp_ = new Regexp(pattern);
   return ForwardSearch(current_regexp_, index);
}

int EivTextBuffer::search_backward(const char* pattern, int index)
{
   if (current_regexp_ != nil)
      delete current_regexp_;
   current_regexp_ = new Regexp(pattern);
   return BackwardSearch(current_regexp_, index);
}

int EivTextBuffer::search_beginning()
{
   return current_regexp_->BeginningOfMatch();
}

int EivTextBuffer::search_end()
{
   return current_regexp_->EndOfMatch();
}

int EivTextBuffer::Insert(int index, const char* string, int count)
{
   if (!modified_)
      modified_ = true;
   if (length + count >= size) 
      expand_buffer(count);
   return TextBuffer::Insert(index, string, count);
}

int EivTextBuffer::Delete(int index, int count)
{
   if (!modified_)
      modified_ = true;
   return TextBuffer::Delete(index, count);
}

void EivTextBuffer::righttrim()
{
   if (!modified_)
      modified_ = true;
   int bytecnt = strlen(Text());
   char* text = (char *) Text();
   while( isspace(text[bytecnt-1]) && bytecnt)
     Delete(1, (bytecnt--)-1);
}

