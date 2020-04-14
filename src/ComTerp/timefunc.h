/*
 * Copyright (c) 2019 Scott E. Johnston
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
 * time funcs
 */

#if !defined(_timefunc_h)
#define _timefunc_h

#include <ComTerp/comfunc.h>
#include <Time/Date.h>

class DateObj {
 public:
  DateObj(const char* datestr);
  DateObj(long datenum); // 1/1/1901 is day zero
  virtual ~DateObj();

  Date *date() {return _date;}

 protected:  
  Date *_date;

  CLASS_SYMID("DateObj");
};

//: date makes date from days since 1/1/1901 or string.
class DateFunc : public ComFunc {
public:
    DateFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dateobj|int = %s(num|str|dateobj :day :month :year :daymo) -- create date from days since 1/1/1901 or string"; }
};

#endif /* !defined(_datefunc_h) */

