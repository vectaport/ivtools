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

#include <ComTerp/timefunc.h>
#include <Time/Date.h>
#include <strstream>

#define TITLE "TimeFunc"

/*****************************************************************************/

int DateObj::_symid= -1;

DateObj::DateObj(const char* datestr) {
  std::istrstream in(datestr);
  _date = new Date(in);
}

DateObj::DateObj(long datenum) {
  _date = new Date(datenum);
}

DateObj::~DateObj() {
  delete _date;
}

/*****************************************************************************/

DateFunc::DateFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DateFunc::execute() {
  ComValue datev(stack_arg(0));
  static int day_sym = symbol_add("day");
  ComValue dayv(stack_key(day_sym));
  static int month_sym = symbol_add("month");
  ComValue monthv(stack_key(month_sym));
  static int year_sym = symbol_add("year");
  ComValue yearv(stack_key(year_sym));
  static int daymo_sym = symbol_add("daymo");
  ComValue daymov(stack_key(daymo_sym));
  static int weekday_sym = symbol_add("weekday");
  ComValue weekdayv(stack_key(weekday_sym));
  reset_stack();

  DateObj* dateobj = NULL;
  boolean fresh = false;
  if (datev.is_num()) {
    dateobj = new DateObj(datev.long_val());
    fresh = true;
  } else if (datev.is_string()) {
    dateobj = new DateObj(datev.string_ptr());
    fresh = true;
  } else {
    dateobj = (DateObj*)datev.geta(DateObj::class_symid());
  }

  if (dayv.is_true()) {
    ComValue retval(dateobj->date()->day());
    push_stack(retval);
    if (fresh) delete dateobj;
    return;
  }

  if (monthv.is_true()) {
    ComValue retval(dateobj->date()->month());
    push_stack(retval);
    if (fresh) delete dateobj;
    return;
  }

  if (yearv.is_true()) {
    ComValue retval(dateobj->date()->year());
    push_stack(retval);
    if (fresh) delete dateobj;
    return;
  }
  
  if (daymov.is_true()) {
    ComValue retval(dateobj->date()->dayOfMonth());
    push_stack(retval);
    if (fresh) delete dateobj;
    return;
  }

  if (weekdayv.is_true()) {
    ComValue retval(Date::nameOfDay(dateobj->date()->weekDay()));
    push_stack(retval);
    if (fresh) delete dateobj;
    return;
  }

  ComValue retval(DateObj::class_symid(), (void*)dateobj);
  push_stack(retval);

}
