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
#include <iomanip>
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

DateObj::DateObj(int day, const char* monthName, int year) {
  _date = new Date((dayTy)day, monthName, (yearTy)year);
}

/*****************************************************************************/

int TimeObj::_symid = -1;

TimeObj::TimeObj(int hour, int minute, int second) {
  _raw.tv_sec = hour*3600 + minute*60 + second;
  _raw.tv_nsec = 0;
  _mono.tv_sec = 0;
  _mono.tv_nsec = 0;
  _tzoff = 0;
  _precision = 0;
}

TimeObj::TimeObj(const struct timespec& raw, long tzoff) {
  _raw = raw;
  _mono.tv_sec = 0;
  _mono.tv_nsec = 0;
  _tzoff = tzoff;
  _precision = 0;
}

TimeObj::TimeObj() {
  clock_gettime(CLOCK_REALTIME, &_raw);
  clock_gettime(CLOCK_MONOTONIC, &_mono);
  struct tm tmval;
  localtime_r(&_raw.tv_sec, &tmval);
  _tzoff = tmval.tm_gmtoff;
  _precision = 0;
}

TimeObj::~TimeObj() {
}

/* the calendar breakdown of this instant in its captured offset, not the
   live process zone -- gmtime_r on _raw shifted by _tzoff, never
   localtime_r, so a stored/round-tripped TimeObj keeps the offset it had
   at capture regardless of what zone later reads it back */
void TimeObj::breakdown(struct tm& tmval) const {
  time_t t = (time_t)(_raw.tv_sec + _tzoff);
  gmtime_r(&t, &tmval);
}

int TimeObj::hour() const {
  struct tm tmval;
  breakdown(tmval);
  return tmval.tm_hour;
}

int TimeObj::minute() const {
  struct tm tmval;
  breakdown(tmval);
  return tmval.tm_min;
}

int TimeObj::second() const {
  struct tm tmval;
  breakdown(tmval);
  return tmval.tm_sec;
}

int TimeObj::year() const {
  struct tm tmval;
  breakdown(tmval);
  return tmval.tm_year + 1900;
}

int TimeObj::month() const {
  struct tm tmval;
  breakdown(tmval);
  return tmval.tm_mon + 1;
}

int TimeObj::day() const {
  struct tm tmval;
  breakdown(tmval);
  return tmval.tm_mday;
}

void TimeObj::printOn(ostream& out) const {
  struct tm tmval;
  breakdown(tmval);
  int yr = tmval.tm_year + 1900;
  int mon = tmval.tm_mon + 1;
  int mday = tmval.tm_mday;

  /* the epoch date is the "no date info" sentinel: an hr:min:sec-only
     construction lands here deliberately (tzoff 0, raw under a day), so
     date fields drop from the outside in as each one matches its epoch
     value, leaving bare h:m:s for a dateless TimeObj */
  boolean dated = !(yr==1970 && mon==1 && mday==1);
  if (dated) {
    out << yr << ":";
    out.write(Date::nameOfMonth(mon), 3);
    out << ":" << mday << ":";
  }

  /* unpadded: a leading-zero literal like "08" fails to re-parse
     (ERR_BADOCT -- 8 and 9 aren't octal digits), so hour/minute/second
     stay bare ints */
  out << tmval.tm_hour << ":" << tmval.tm_min << ":" << tmval.tm_sec;

  if (_precision > 0) {
    long nsec = _raw.tv_nsec;
    int ms = (int)(nsec / 1000000);
    int us = (int)((nsec / 1000) % 1000);
    int ns = (int)(nsec % 1000);
    char fill = out.fill('0');
    out << ":" << std::setw(3) << ms;
    if (_precision > 1) out << ":" << std::setw(3) << us;
    if (_precision > 2) out << ":" << std::setw(3) << ns;
    out.fill(fill);
  }

  if (dated) {
    /* always signed and zero-padded to 4 digits (-0700, +0200) --
       the sign re-parses through unary minus/plus */
    long off = _tzoff;
    long aoff = off < 0 ? -off : off;
    int tzh = (int)(aoff / 3600);
    int tzm = (int)((aoff % 3600) / 60);
    char fill = out.fill('0');
    out << ":" << (off < 0 ? "-" : "+") << std::setw(4) << (tzh*100 + tzm);
    out.fill(fill);
  }
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
  } else if (datev.is_timeobj()) {
    TimeObj* timeobj = (TimeObj*)datev.geta(TimeObj::class_symid());
    /* the epoch date is TimeObj's "no date info" sentinel -- see TimeObj */
    if (timeobj->year()==1970 && timeobj->month()==1 && timeobj->day()==1) {
      push_stack(ComValue::nullval());
      return;
    }
    dateobj = new DateObj(timeobj->day(), Date::nameOfMonth(timeobj->month()), timeobj->year());
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

/* time()'s vetting of a colon list into a TimeObj -- an explicit ask,
   unlike ':' itself, so a bad literal warns at this exact call site
   instead of silently falling back to the list it arrived as.  A plain
   3-element list is hr:min:sec: minute and second bounded to a clock
   face, hour only checked non-negative at construction.  Landing on the
   epoch date makes it a dateless TimeObj when hour is under 24; at or
   past 24 it reads back folded onto the following calendar day, the same
   as any other TimeObj's single gmtime_r-based breakdown would.  A
   7-to-10-element list is a full y:Mon:d:h:m:s[:ms:us:ns]:TZ timestamp --
   the inverse of what printOn() emits, down to the trailing numeric HHMM
   offset (unary-plus- or unary-minus-prefixed, printOn() always emitting
   one or the other). */
static TimeObj* colonlist_to_timeobj(ComTerp* comterp, AttributeValueList* avl, int linenum) {
  int n = avl->Number();

  if (n == 3) {
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

  if (n < 7 || n > 10) {
    std::cout << "WARNING:  time() needs a 3-element hr:min:sec list or a "
                 "7-to-10-element y:Mon:d:h:m:s[:ms:us:ns]:TZ list, got "
              << n << " element(s) -- line " << linenum << "\n";
    return nil;
  }

  /* the month field stays an unresolved symbol, same as ':' itself never
     resolving an identifier operand -- a month name is never meant to be
     a bound variable, so it is the one element not run through
     lookup_symval() (which would otherwise answer nil for it, "Sep"
     never being assigned anything) */
  ComValue elems[10];
  for (int i=0; i<n; i++) {
    elems[i] = ComValue(*avl->Get(i));
    if (i != 1) elems[i] = comterp->lookup_symval(elems[i]);
  }

  if (elems[0].type()!=ComValue::IntType) {
    std::cout << "WARNING:  time(): year must be a plain integer -- line "
              << linenum << "\n";
    return nil;
  }
  int yr = elems[0].int_val();

  if (elems[1].type()!=ComValue::SymbolType) {
    std::cout << "WARNING:  time(): month must be a bare month name (e.g. Sep) -- line "
              << linenum << "\n";
    return nil;
  }
  int mon = Date::numberOfMonth(symbol_pntr((int)elems[1].symbol_val()));
  if (mon==0) {
    std::cout << "WARNING:  time(): unrecognized month name -- line "
              << linenum << "\n";
    return nil;
  }

  if (elems[2].type()!=ComValue::IntType) {
    std::cout << "WARNING:  time(): day must be a plain integer -- line "
              << linenum << "\n";
    return nil;
  }
  int day = elems[2].int_val();
  if (day<1 || day>31) {
    std::cout << "WARNING:  time(): day " << day << " out of range (1..31) -- line "
              << linenum << "\n";
    return nil;
  }

  if (elems[3].type()!=ComValue::IntType || elems[4].type()!=ComValue::IntType ||
      elems[5].type()!=ComValue::IntType) {
    std::cout << "WARNING:  time(): h:m:s must be plain integers -- line "
              << linenum << "\n";
    return nil;
  }
  int hr = elems[3].int_val();
  int mn = elems[4].int_val();
  int sc = elems[5].int_val();
  if (hr<0 || hr>23) {
    std::cout << "WARNING:  time(): hour " << hr << " out of range (0..23) -- line "
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

  int fracgroups = n - 7;
  int frac[3] = {0, 0, 0};
  for (int g=0; g<fracgroups; g++) {
    if (elems[6+g].type()!=ComValue::IntType) {
      std::cout << "WARNING:  time(): fractional-second group must be a plain integer -- line "
                << linenum << "\n";
      return nil;
    }
    int v = elems[6+g].int_val();
    if (v<0 || v>999) {
      std::cout << "WARNING:  time(): fractional-second group " << v
                << " out of range (0..999) -- line " << linenum << "\n";
      return nil;
    }
    frac[g] = v;
  }

  ComValue tzv = elems[n-1];
  if (tzv.type()!=ComValue::IntType) {
    std::cout << "WARNING:  time(): trailing TZ field must be a plain integer HHMM offset -- line "
              << linenum << "\n";
    return nil;
  }
  int tzval = tzv.int_val();
  int tzsign = tzval<0 ? -1 : 1;
  int atz = tzval<0 ? -tzval : tzval;
  int tzh = atz/100;
  int tzm = atz%100;
  if (tzm>59) {
    std::cout << "WARNING:  time(): TZ minutes " << tzm << " out of range (0..59) -- line "
              << linenum << "\n";
    return nil;
  }
  long tzoff = tzsign * (long)(tzh*3600 + tzm*60);

  /* the inverse of printOn()'s breakdown: reinterpret the calendar fields
     as UTC via timegm(), then remove the stated offset to land back on
     the actual epoch second they represent */
  struct tm tmval = {0};
  tmval.tm_year = yr - 1900;
  tmval.tm_mon = mon - 1;
  tmval.tm_mday = day;
  tmval.tm_hour = hr;
  tmval.tm_min = mn;
  tmval.tm_sec = sc;
  time_t naive = timegm(&tmval);

  struct timespec raw;
  raw.tv_sec = naive - tzoff;
  raw.tv_nsec = frac[0]*1000000L + frac[1]*1000L + frac[2];

  TimeObj* result = new TimeObj(raw, tzoff);
  result->precision(fracgroups);
  return result;
}

TimeFunc::TimeFunc(ComTerp* comterp) : ComFunc(comterp) {}

void TimeFunc::execute() {
  ComValue timev(stack_arg(0));
  static int hour_sym = symbol_add("hr");
  static int minute_sym = symbol_add("min");
  static int second_sym = symbol_add("sec");
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
  int linenum = funcstate() ? funcstate()->linenum() : 0;
  reset_stack();

  // cumulative, matching printOn()'s fractional-group display: :ns implies
  // ms+us+ns, :us implies ms+us, :ms is ms alone.
  int fracgroups = nsv.is_true() ? 3 : usv.is_true() ? 2 : msv.is_true() ? 1 : 0;

  TimeObj* timeobj = nil;
  boolean owns = false;

  if (timev.is_dateobj()) {
    /* noon UTC that date -- the inverse of date()'s TimeObj-to-DateObj
       conversion */
    DateObj* dateobj = (DateObj*)timev.geta(DateObj::class_symid());
    struct tm tmval = {0};
    tmval.tm_year = dateobj->date()->year() - 1900;
    tmval.tm_mon = dateobj->date()->month() - 1;
    tmval.tm_mday = dateobj->date()->dayOfMonth();
    tmval.tm_hour = 12;
    struct timespec raw;
    raw.tv_sec = timegm(&tmval);
    raw.tv_nsec = 0;
    timeobj = new TimeObj(raw, 0);
    owns = true;
  } else if (timev.is_array() && timev.coloned()) {
    /* ':' never inspects what it builds -- this is the explicit site
       that vets a colon-list argument into a TimeObj */
    timeobj = colonlist_to_timeobj(comterp(), timev.array_val(), linenum);
    if (!timeobj) {
      push_stack(ComValue::nullval());
      return;
    }
    owns = true;
  } else if (timev.is_timeobj()) {
    timeobj = (TimeObj*)timev.geta(TimeObj::class_symid());
  } else if (!(rawv.is_true() || monov.is_true())) {
    /* no positional TimeObj/DateObj/colon-list and no raw/mono integer
       dump asked -- capture now.  This is the same precedent date()'s own
       field keywords use over today's date when no positional DateObj is
       given, extended to time()'s own bare capture. */
    timeobj = new TimeObj();
    owns = true;
  }

  if (timeobj) {
    /* owns: no other holder, so free after reading a scalar field, or
       after building a display-precision copy below */
    if (hourv.is_true()) {
      ComValue retval(timeobj->hour());
      push_stack(retval);
      if (owns) delete timeobj;
    } else if (minutev.is_true()) {
      ComValue retval(timeobj->minute());
      push_stack(retval);
      if (owns) delete timeobj;
    } else if (secondv.is_true()) {
      ComValue retval(timeobj->second());
      push_stack(retval);
      if (owns) delete timeobj;
    } else if (owns) {
      /* an explicit :ms/:us/:ns overrides; otherwise keep whatever
         precision the value already carries -- a parsed colon list keeps
         the fractional groups it was given, a fresh capture or a noon
         conversion default to none */
      if (fracgroups > 0) timeobj->precision(fracgroups);
      ComValue retval(TimeObj::class_symid(), (void*)timeobj);
      push_stack(retval);
    } else if (fracgroups > 0) {
      /* a caller-supplied TimeObj is never mutated in place -- a display
         precision request returns a fresh TimeObj at the same instant */
      TimeObj* copy = new TimeObj(timeobj->raw(), timeobj->tzoff());
      copy->precision(fracgroups);
      ComValue retval(TimeObj::class_symid(), (void*)copy);
      push_stack(retval);
    } else
      push_stack(timev);
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
