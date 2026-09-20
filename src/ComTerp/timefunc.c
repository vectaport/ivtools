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
#include <Attribute/attrlist.h>
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

int TimeObj::_symid = -1;

TimeObj::TimeObj(int hour, int minute, int second)
  : _hour(hour), _minute(minute), _second(second) {
}

TimeObj::~TimeObj() {
}

void TimeObj::printOn(ostream& out) const {
  /* unpadded: a leading-zero two-digit literal like "08" fails to
     re-parse (ERR_BADOCT -- 8 and 9 aren't octal digits), so zero-padding
     minute/second would break round-tripping back through the scanner */
  out << _hour << ":" << _minute << ":" << _second;
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

/* time()'s vetting of a plain hr:min:sec colon list into a TimeObj -- an
   explicit ask, unlike ':' itself, so a bad literal warns at this exact
   call site instead of silently falling back to the list it arrived as.
   Minute and second are bounded to a clock face; hour is left unbounded
   so an elapsed duration (25:00:00) still constructs. */
static TimeObj* colonlist_to_timeobj(ComTerp* comterp, AttributeValueList* avl, int linenum) {
  if (avl->Number() != 3) {
    std::cout << "WARNING:  time() needs a 3-element hr:min:sec list, got "
              << avl->Number() << " element(s) -- line " << linenum << "\n";
    return nil;
  }

  ComValue hrv(*avl->Get(0));
  ComValue mnv(*avl->Get(1));
  ComValue scv(*avl->Get(2));
  hrv = comterp->lookup_symval(hrv);
  mnv = comterp->lookup_symval(mnv);
  scv = comterp->lookup_symval(scv);
  if (hrv.type()!=ComValue::IntType || mnv.type()!=ComValue::IntType ||
      scv.type()!=ComValue::IntType) {
    std::cout << "WARNING:  time(): hr:min:sec must be plain integers -- line "
              << linenum << "\n";
    return nil;
  }

  int hr = hrv.int_val();
  int mn = mnv.int_val();
  int sc = scv.int_val();
  if (hr<0) {
    std::cout << "WARNING:  time(): hour " << hr << " is negative -- line "
              << linenum << "\n";
    return nil;
  }
  if (mn<0 || mn>59) {
    std::cout << "WARNING:  time(): minute " << mn << " out of range (0..59) -- line "
              << linenum << "\n";
    return nil;
  }
  if (sc<0 || sc>59) {
    std::cout << "WARNING:  time(): second " << sc << " out of range (0..59) -- line "
              << linenum << "\n";
    return nil;
  }

  return new TimeObj(hr, mn, sc);
}

TimeFunc::TimeFunc(ComTerp* comterp) : ComFunc(comterp) {}

void TimeFunc::execute() {
  ComValue timev(stack_arg(0));
  static int hour_sym = symbol_add("hour");
  static int minute_sym = symbol_add("minute");
  static int second_sym = symbol_add("second");
  ComValue hourv(stack_key(hour_sym));
  ComValue minutev(stack_key(minute_sym));
  ComValue secondv(stack_key(second_sym));
  static int ms_sym = symbol_add("ms");
  static int us_sym = symbol_add("us");
  static int ns_sym = symbol_add("ns");
  static int mono_sym = symbol_add("mono");
  static int raw_sym = symbol_add("raw");
  ComValue msv(stack_key(ms_sym));
  ComValue usv(stack_key(us_sym));
  ComValue nsv(stack_key(ns_sym));
  ComValue monov(stack_key(mono_sym));
  ComValue rawv(stack_key(raw_sym));
  boolean anykey = msv.is_true() || usv.is_true() || nsv.is_true()
                   || monov.is_true() || rawv.is_true();
  int linenum = funcstate() ? funcstate()->linenum() : 0;
  reset_stack();

  /* ':' never inspects what it builds -- this is the explicit site that
     vets a colon-list argument into a TimeObj */
  boolean built_here = false;
  if (timev.is_array() && timev.coloned()) {
    TimeObj* built = colonlist_to_timeobj(comterp(), timev.array_val(), linenum);
    if (!built) {
      push_stack(ComValue::nullval());
      return;
    }
    timev = ComValue(TimeObj::class_symid(), (void*)built);
    built_here = true;
  }

  if (timev.is_timeobj()) {
    TimeObj* timeobj = (TimeObj*)timev.geta(TimeObj::class_symid());
    /* built_here: no other owner, so free after reading a scalar field */
    if (hourv.is_true()) {
      ComValue retval(timeobj->hour());
      push_stack(retval);
      if (built_here) delete timeobj;
    } else if (minutev.is_true()) {
      ComValue retval(timeobj->minute());
      push_stack(retval);
      if (built_here) delete timeobj;
    } else if (secondv.is_true()) {
      ComValue retval(timeobj->second());
      push_stack(retval);
      if (built_here) delete timeobj;
    } else
      push_stack(timev);
    return;
  }

  /* the bare call is reserved for a future TimeObj return;
     answering a plain number now would entrench the wrong type */
  if (!anykey) {
    std::cout << "WARNING:  time() without a keyword is reserved for a TimeObj return, not yet implemented -- use time(:raw) for the epoch reading or time(:mono) for a monotonic one -- line " << linenum << "\n";
    push_stack(ComValue::nullval());
    return;
  }

  /* CLOCK_REALTIME is a comparable wall-clock date but can step backwards;
     CLOCK_MONOTONIC (:mono) is epoch-less but safe for measuring intervals */
  struct timespec ts;
  clock_gettime(monov.is_true() ? CLOCK_MONOTONIC : CLOCK_REALTIME, &ts);
  // a single reading (ts) feeds every returned unit, so the keywords
  // can't disagree about which instant they describe.
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
