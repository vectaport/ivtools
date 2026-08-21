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
#include <sstream>
#include <time.h>
#include <limits.h>

/* Sub-second units need 64 bits: nanoseconds since the epoch is ~1.8e18,
   microseconds ~1.8e15, milliseconds ~1.8e12, all past a 32-bit long.  Where
   long is narrower there is no integer type here that can hold them, and the
   choice would be between a wrapped number that looks like a time and a nil
   that every caller then has to test for -- so say so at build time instead.
   Nothing in this tree builds ILP32 today; if something ever does, this stops
   it with a reason rather than letting it compute wrong timestamps. */
#if LONG_MAX < 9223372036854775807LL
#error "comterp time(): the :ms, :us and :ns keywords require a 64-bit long"
#endif

#define TITLE "TimeFunc"

/*****************************************************************************/

int DateObj::_symid= -1;

DateObj::DateObj(const char* datestr) {
  std::istringstream in(datestr);
  _date = new Date(in);
}

DateObj::DateObj(long datenum) {
  _date = new Date(datenum);
}

DateObj::DateObj(DateObj *dateobj) {
  _date = new Date(*dateobj->date());
}

DateObj::DateObj() {
  _date = new Date();
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
  } else if (datev.is_null()) {
    dateobj = new DateObj();
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

/*****************************************************************************/

TimeFunc::TimeFunc(ComTerp* comterp) : ComFunc(comterp) {}

void TimeFunc::execute() {
  static int ms_sym = symbol_add("ms");
  static int us_sym = symbol_add("us");
  static int ns_sym = symbol_add("ns");
  ComValue msv(stack_key(ms_sym));
  ComValue usv(stack_key(us_sym));
  ComValue nsv(stack_key(ns_sym));
  reset_stack();

  /* CLOCK_REALTIME, not the CLOCK_MONOTONIC used for comeditor.c's watchdog:
     this is a wall-clock reading meant to be compared with dates and other
     machines' clocks, so it must follow an NTP correction rather than ignore
     one.  A single reading serves every unit, so the keywords cannot disagree
     about which instant they describe. */
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long sec = (long)ts.tv_sec;
  long nsec = (long)ts.tv_nsec;

  long result;
  if (nsv.is_true())
    result = sec * 1000000000L + nsec;
  else if (usv.is_true())
    result = sec * 1000000L + nsec / 1000L;
  else if (msv.is_true())
    result = sec * 1000L + nsec / 1000000L;
  else
    result = sec;

  ComValue retval(result);
  push_stack(retval);
}
