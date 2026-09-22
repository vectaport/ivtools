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
  _delta = false;
}

TimeObj::TimeObj(const struct timespec& raw, long tzoff) {
  _raw = raw;
  _mono.tv_sec = 0;
  _mono.tv_nsec = 0;
  _tzoff = tzoff;
  _precision = 0;
  _delta = false;
}

TimeObj::TimeObj() {
  clock_gettime(CLOCK_REALTIME, &_raw);
  clock_gettime(CLOCK_MONOTONIC, &_mono);
  struct tm tmval;
  localtime_r(&_raw.tv_sec, &tmval);
  _tzoff = tmval.tm_gmtoff;
  _precision = 0;
  _delta = false;
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
  int h = tmval.tm_hour;
  return (!_delta && h==0) ? 24 : h;
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

/* the 4th year of the cycle is a 366-day year, the rest 365 -- a fixed
   synthetic calendar, not the real proleptic-Gregorian leap rule, since a
   duration's years/days breakdown isn't anchored to any actual date. */
static int cycle_year_days(int year_index_1based) {
  return (year_index_1based % 4 == 0) ? 366 : 365;
}

/* decomposes total_days into completed synthetic years and the remaining
   days -- the inverse of years_to_days(). */
static void days_to_years(long total_days, long& years, long& days) {
  long y = 0;
  long remaining = total_days;
  while (remaining >= cycle_year_days((int)(y+1))) {
    remaining -= cycle_year_days((int)(y+1));
    y++;
  }
  years = y;
  days = remaining;
}

/* total days spanned by 'years' completed synthetic years -- the inverse
   of days_to_years(). */
static long years_to_days(long years) {
  long total = 0;
  for (long y=1; y<=years; y++) total += cycle_year_days((int)y);
  return total;
}

void TimeObj::printOn(ostream& out) const {
  if (_delta) {
    /* a duration isn't anchored to any calendar instant, so its days/years
       come from a synthetic 4-year cycle rather than gmtime_r(). A negative
       duration decomposes by magnitude (never a per-field negative, which
       would print "-1:-30" for -90 seconds instead of the one leading sign
       "-1:30" a reader expects) -- one leading "-" on the whole value. */
    long total = (long)_raw.tv_sec;
    boolean negative = total < 0;
    if (negative) total = -total;
    long sec = total % 60; total /= 60;
    long min = total % 60; total /= 60;
    long hr = total % 24; total /= 24;
    long years, days;
    days_to_years(total, years, days);

    boolean show_years = years != 0;
    boolean show_days = show_years || days != 0;
    boolean show_hr = show_days || hr != 0;

    if (negative) out << "-";
    if (show_years) out << years << ":";
    if (show_days) out << days << ":";
    if (show_hr) out << hr << ":";
    out << min << ":" << sec;
    return;
  }

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
  out << (tmval.tm_hour==0 ? 24 : tmval.tm_hour) << ":" << tmval.tm_min << ":" << tmval.tm_sec;

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

/* defined below, alongside time()'s own colon-list vetting */
static DateObj* colonlist_to_dateobj(ComTerp* comterp, AttributeValueList* avl, int linenum);

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
  TimeObj* timeobj = NULL;
  if (datev.is_num()) {
    dateobj = new DateObj(datev.long_val());
    fresh = true;
  } else if (datev.is_string()) {
    dateobj = new DateObj(datev.string_ptr());
    fresh = true;
  } else if (datev.is_timeobj()) {
    timeobj = (TimeObj*)datev.geta(TimeObj::class_symid());
  } else if (datev.is_array() && datev.coloned()) {
    /* year:month[:day], not the day-led dd-mmm-yy string Date's own
       stream parser expects -- see colonlist_to_dateobj() below */
    int linenum = funcstate() ? funcstate()->linenum() : 0;
    dateobj = colonlist_to_dateobj(comterp(), datev.array_val(), linenum);
    if (!dateobj) {
      push_stack(ComValue::nullval());
      return;
    }
    fresh = true;
  } else if (datev.is_null()) {
    dateobj = new DateObj();
  } else {
    dateobj = (DateObj*)datev.geta(DateObj::class_symid());
  }

  if (timeobj) {
    /* no date info: a duration has no calendar date at all, and the
       epoch date is the sentinel for a dateless instant -- see TimeObj */
    if (timeobj->delta() ||
        (timeobj->year()==1970 && timeobj->month()==1 && timeobj->day()==1)) {
      push_stack(ComValue::nullval());
      return;
    }
    dateobj = new DateObj(timeobj->day(), Date::nameOfMonth(timeobj->month()), timeobj->year());
    fresh = true;
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

/* the range representable by a signed 64-bit nanosecond count since the
   epoch -- the leading field of a short colon list reads as a calendar
   year only within this range, distinguishing YEAR:MON:... from a
   same-shaped duration. */
static boolean is_plausible_year(long v) { return v >= 1677 && v <= 2262; }

/* real proleptic-Gregorian leap-year rule, for validating a calendar day
   against its actual month/year -- distinct from days_to_years()'s
   synthetic 4-year duration cycle below, which this never touches. */
static boolean is_leap_year(int yr) {
  return (yr%4==0 && yr%100!=0) || yr%400==0;
}

static int days_in_month(int mon, int yr) {
  static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (mon==2 && is_leap_year(yr)) return 29;
  return days[mon-1];
}

/* accepts a bare month-name symbol (looked up via Date::numberOfMonth())
   or a plain 1-12 integer; warns and returns false on anything else.
   cmdname labels the warning for whichever command is doing the parsing. */
static boolean parse_month(ComValue& v, int& mon, int linenum, const char* cmdname = "time()") {
  if (v.type()==ComValue::SymbolType) {
    mon = Date::numberOfMonth(symbol_pntr((int)v.symbol_val()));
    if (mon==0) {
      std::cerr << "WARNING:  " << cmdname << ": unrecognized month name -- line "
                << linenum << "\n";
      return false;
    }
  } else if (v.type()==ComValue::IntType) {
    mon = v.int_val();
    if (mon<1 || mon>12) {
      std::cerr << "WARNING:  " << cmdname << ": month " << mon << " out of range (1..12) -- line "
                << linenum << "\n";
      return false;
    }
  } else {
    std::cerr << "WARNING:  " << cmdname << ": month must be a bare month name (e.g. Sep) or a number 1..12 -- line "
              << linenum << "\n";
    return false;
  }
  return true;
}

/* the i'th colon-list element, resolved as a variable reference the same
   way every non-month field of a colon list is -- lookup_symval() needs
   an lvalue, so the raw element is named first. */
static ComValue resolve_elem(ComTerp* comterp, AttributeValueList* avl, int i) {
  ComValue v(*avl->Get(i));
  return comterp->lookup_symval(v);
}

/* a year-led short colon list's instant: noon UTC when hour<0, else the
   given hour with minute/second zero -- tzoff is always 0, the same as
   time()'s DateObj-to-TimeObj noon conversion. */
static TimeObj* year_led_instant(int yr, int mon, int day, int hour) {
  struct tm tmval = {0};
  tmval.tm_year = yr - 1900;
  tmval.tm_mon = mon - 1;
  tmval.tm_mday = day;
  tmval.tm_hour = hour>=0 ? hour : 12;
  struct timespec raw;
  raw.tv_sec = timegm(&tmval);
  raw.tv_nsec = 0;
  return new TimeObj(raw, 0);
}

/* a year-led 5-field instant (y:Mon:d:h:m, no seconds or TZ field): read as
   a local wall-clock time via mktime() rather than UTC, since there is no
   TZ field to say otherwise -- unlike the shorter year-led forms above,
   which always land at tzoff 0.  A second localtime_r() on the resulting
   epoch time captures the authoritative tm_gmtoff for that date, the same
   construct-then-read pattern :raw N's valued form uses. */
static TimeObj* year_led_local_instant(int yr, int mon, int day, int hour, int minute) {
  struct tm tmval = {0};
  tmval.tm_year = yr - 1900;
  tmval.tm_mon = mon - 1;
  tmval.tm_mday = day;
  tmval.tm_hour = hour;
  tmval.tm_min = minute;
  tmval.tm_sec = 0;
  tmval.tm_isdst = -1;
  struct timespec raw;
  raw.tv_sec = mktime(&tmval);
  raw.tv_nsec = 0;
  struct tm tzcheck;
  localtime_r(&raw.tv_sec, &tzcheck);
  return new TimeObj(raw, tzcheck.tm_gmtoff);
}

/* time()'s vetting of a colon list into a TimeObj -- an explicit ask,
   unlike ':' itself, so a bad literal warns at this exact call site
   instead of silently falling back to the list it arrived as.  A 2-, 3-
   or 4-element list auto-detects instant vs. duration from its first
   element: a plausible year (is_plausible_year()) reads as
   YEAR:MON[:day[:hr]], anything else as min:sec, hr:min:sec or
   days:hr:min:sec, a duration (delta() true).  Landing hr:min:sec on the
   epoch date makes it a dateless TimeObj when hour is under 24; at or
   past 24 it reads back folded onto the following calendar day, the same
   as any other TimeObj's single gmtime_r-based breakdown would.  A
   5-element list auto-detects the same way: a plausible leading year
   reads as the local-time instant YEAR:MON:day:hr:min
   (year_led_local_instant()), anything else as the duration
   years:days:hr:min:sec.  A 7-to-10-element list is a full
   y:Mon:d:h:m:s[:ms:us:ns]:TZ timestamp -- the inverse of what
   printOn() emits, down to the trailing numeric HHMM offset (unary-plus-
   or unary-minus-prefixed, printOn() always emitting one or the
   other). */
static TimeObj* colonlist_to_timeobj(ComTerp* comterp, AttributeValueList* avl, int linenum) {
  int n = avl->Number();

  if (n == 2) {
    ComValue v0(resolve_elem(comterp, avl, 0));

    if (v0.type()==ComValue::IntType && is_plausible_year(v0.long_val())) {
      int yr = v0.int_val();
      ComValue monv(*avl->Get(1));
      int mon;
      if (!parse_month(monv, mon, linenum)) return nil;
      return year_led_instant(yr, mon, 1, -1);
    }

    ComValue v1(resolve_elem(comterp, avl, 1));
    if (v0.type()!=ComValue::IntType || v1.type()!=ComValue::IntType) {
      std::cerr << "WARNING:  time(): min:sec must be plain integers -- line "
                << linenum << "\n";
      return nil;
    }
    long mn = v0.long_val();
    long sc = v1.long_val();
    if (mn<0) {
      std::cerr << "WARNING:  time(): minute " << mn << " is negative -- line "
                << linenum << "\n";
      return nil;
    }
    if (sc<0 || sc>59) {
      std::cerr << "WARNING:  time(): second " << sc << " out of range (0..59) -- line "
                << linenum << "\n";
      return nil;
    }
    struct timespec raw;
    raw.tv_sec = mn*60 + sc;
    raw.tv_nsec = 0;
    TimeObj* t = new TimeObj(raw, 0);
    t->delta(true);
    return t;
  }

  if (n == 3) {
    ComValue v0(resolve_elem(comterp, avl, 0));

    if (v0.type()==ComValue::IntType && is_plausible_year(v0.long_val())) {
      int yr = v0.int_val();
      ComValue monv(*avl->Get(1));
      int mon;
      if (!parse_month(monv, mon, linenum)) return nil;

      ComValue dayv(resolve_elem(comterp, avl, 2));
      if (dayv.type()!=ComValue::IntType) {
        std::cerr << "WARNING:  time(): day must be a plain integer -- line "
                  << linenum << "\n";
        return nil;
      }
      int day = dayv.int_val();
      if (day<1 || day>31) {
        std::cerr << "WARNING:  time(): day " << day << " out of range (1..31) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (day > days_in_month(mon, yr)) {
        std::cerr << "WARNING:  time(): day " << day << " does not exist in that month/year -- line "
                  << linenum << "\n";
        return nil;
      }
      return year_led_instant(yr, mon, day, -1);
    }

    ComValue hrv(*avl->Get(0));
    ComValue mnv(*avl->Get(1));
    ComValue scv(*avl->Get(2));
    hrv = comterp->lookup_symval(hrv);
    mnv = comterp->lookup_symval(mnv);
    scv = comterp->lookup_symval(scv);
    if (hrv.type()!=ComValue::IntType || mnv.type()!=ComValue::IntType ||
        scv.type()!=ComValue::IntType) {
      std::cerr << "WARNING:  time(): hr:min:sec must be plain integers -- line "
                << linenum << "\n";
      return nil;
    }

    int hr = hrv.int_val();
    int mn = mnv.int_val();
    int sc = scv.int_val();
    if (hr<0) {
      std::cerr << "WARNING:  time(): hour " << hr << " is negative -- line "
                << linenum << "\n";
      return nil;
    }
    if (mn<0 || mn>59) {
      std::cerr << "WARNING:  time(): minute " << mn << " out of range (0..59) -- line "
                << linenum << "\n";
      return nil;
    }
    if (sc<0 || sc>59) {
      std::cerr << "WARNING:  time(): second " << sc << " out of range (0..59) -- line "
                << linenum << "\n";
      return nil;
    }

    TimeObj* t = new TimeObj(hr, mn, sc);
    t->delta(true);
    return t;
  }

  if (n == 4) {
    ComValue v0(resolve_elem(comterp, avl, 0));

    if (v0.type()==ComValue::IntType && is_plausible_year(v0.long_val())) {
      int yr = v0.int_val();
      ComValue monv(*avl->Get(1));
      int mon;
      if (!parse_month(monv, mon, linenum)) return nil;

      ComValue dayv(resolve_elem(comterp, avl, 2));
      if (dayv.type()!=ComValue::IntType) {
        std::cerr << "WARNING:  time(): day must be a plain integer -- line "
                  << linenum << "\n";
        return nil;
      }
      int day = dayv.int_val();
      if (day<1 || day>31) {
        std::cerr << "WARNING:  time(): day " << day << " out of range (1..31) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (day > days_in_month(mon, yr)) {
        std::cerr << "WARNING:  time(): day " << day << " does not exist in that month/year -- line "
                  << linenum << "\n";
        return nil;
      }

      ComValue hrv(resolve_elem(comterp, avl, 3));
      if (hrv.type()!=ComValue::IntType) {
        std::cerr << "WARNING:  time(): hour must be a plain integer -- line "
                  << linenum << "\n";
        return nil;
      }
      int hour = hrv.int_val();
      if (hour<1 || hour>24) {
        std::cerr << "WARNING:  time(): hour " << hour << " out of range (1..24) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (hour==24) hour=0;
      return year_led_instant(yr, mon, day, hour);
    }

    ComValue hrv(resolve_elem(comterp, avl, 1));
    ComValue mnv(resolve_elem(comterp, avl, 2));
    ComValue scv(resolve_elem(comterp, avl, 3));
    if (v0.type()!=ComValue::IntType || hrv.type()!=ComValue::IntType ||
        mnv.type()!=ComValue::IntType || scv.type()!=ComValue::IntType) {
      std::cerr << "WARNING:  time(): days:hr:min:sec must be plain integers -- line "
                << linenum << "\n";
      return nil;
    }
    long days = v0.long_val();
    int hr = hrv.int_val();
    int mn = mnv.int_val();
    int sc = scv.int_val();
    if (days<0) {
      std::cerr << "WARNING:  time(): days " << days << " is negative -- line "
                << linenum << "\n";
      return nil;
    }
    if (hr<0 || hr>23) {
      std::cerr << "WARNING:  time(): hour " << hr << " out of range (0..23) -- line "
                << linenum << "\n";
      return nil;
    }
    if (mn<0 || mn>59) {
      std::cerr << "WARNING:  time(): minute " << mn << " out of range (0..59) -- line "
                << linenum << "\n";
      return nil;
    }
    if (sc<0 || sc>59) {
      std::cerr << "WARNING:  time(): second " << sc << " out of range (0..59) -- line "
                << linenum << "\n";
      return nil;
    }
    struct timespec raw;
    raw.tv_sec = ((days*24 + hr)*60 + mn)*60 + sc;
    raw.tv_nsec = 0;
    TimeObj* t = new TimeObj(raw, 0);
    t->delta(true);
    return t;
  }

  if (n == 5) {
    ComValue v0(resolve_elem(comterp, avl, 0));

    if (v0.type()==ComValue::IntType && is_plausible_year(v0.long_val())) {
      int yr = v0.int_val();
      ComValue monv(*avl->Get(1));
      int mon;
      if (!parse_month(monv, mon, linenum)) return nil;

      ComValue dayv(resolve_elem(comterp, avl, 2));
      if (dayv.type()!=ComValue::IntType) {
        std::cerr << "WARNING:  time(): day must be a plain integer -- line "
                  << linenum << "\n";
        return nil;
      }
      int day = dayv.int_val();
      if (day<1 || day>31) {
        std::cerr << "WARNING:  time(): day " << day << " out of range (1..31) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (day > days_in_month(mon, yr)) {
        std::cerr << "WARNING:  time(): day " << day << " does not exist in that month/year -- line "
                  << linenum << "\n";
        return nil;
      }

      ComValue hrv(resolve_elem(comterp, avl, 3));
      if (hrv.type()!=ComValue::IntType) {
        std::cerr << "WARNING:  time(): hour must be a plain integer -- line "
                  << linenum << "\n";
        return nil;
      }
      int hour = hrv.int_val();
      if (hour<1 || hour>24) {
        std::cerr << "WARNING:  time(): hour " << hour << " out of range (1..24) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (hour==24) hour=0;

      ComValue mnv0(resolve_elem(comterp, avl, 4));
      if (mnv0.type()!=ComValue::IntType) {
        std::cerr << "WARNING:  time(): minute must be a plain integer -- line "
                  << linenum << "\n";
        return nil;
      }
      int minute = mnv0.int_val();
      if (minute<0 || minute>59) {
        std::cerr << "WARNING:  time(): minute " << minute << " out of range (0..59) -- line "
                  << linenum << "\n";
        return nil;
      }

      return year_led_local_instant(yr, mon, day, hour, minute);
    } else {
      ComValue daysv(resolve_elem(comterp, avl, 1));
      ComValue hrv(resolve_elem(comterp, avl, 2));
      ComValue mnv(resolve_elem(comterp, avl, 3));
      ComValue scv(resolve_elem(comterp, avl, 4));
      if (v0.type()!=ComValue::IntType || daysv.type()!=ComValue::IntType ||
          hrv.type()!=ComValue::IntType || mnv.type()!=ComValue::IntType ||
          scv.type()!=ComValue::IntType) {
        std::cerr << "WARNING:  time(): yrs:days:hr:min:sec must be plain integers -- line "
                  << linenum << "\n";
        return nil;
      }
      long years = v0.long_val();
      int days = daysv.int_val();
      int hr = hrv.int_val();
      int mn = mnv.int_val();
      int sc = scv.int_val();
      if (years<0) {
        std::cerr << "WARNING:  time(): years " << years << " is negative -- line "
                  << linenum << "\n";
        return nil;
      }
      if (days<0 || days>366) {
        std::cerr << "WARNING:  time(): days " << days << " out of range (0..366) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (hr<0 || hr>23) {
        std::cerr << "WARNING:  time(): hour " << hr << " out of range (0..23) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (mn<0 || mn>59) {
        std::cerr << "WARNING:  time(): minute " << mn << " out of range (0..59) -- line "
                  << linenum << "\n";
        return nil;
      }
      if (sc<0 || sc>59) {
        std::cerr << "WARNING:  time(): second " << sc << " out of range (0..59) -- line "
                  << linenum << "\n";
        return nil;
      }
      long total_days = years_to_days(years) + days;
      struct timespec raw;
      raw.tv_sec = ((total_days*24 + hr)*60 + mn)*60 + sc;
      raw.tv_nsec = 0;
      TimeObj* t = new TimeObj(raw, 0);
      t->delta(true);
      return t;
    }
  }

  if (n < 7 || n > 10) {
    std::cerr << "WARNING:  time() needs a 2-to-5-element instant or duration list "
                 "or a 7-to-10-element y:Mon:d:h:m:s[:ms:us:ns]:TZ list, got "
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
    std::cerr << "WARNING:  time(): year must be a plain integer -- line "
              << linenum << "\n";
    return nil;
  }
  int yr = elems[0].int_val();

  int mon;
  if (!parse_month(elems[1], mon, linenum)) return nil;

  if (elems[2].type()!=ComValue::IntType) {
    std::cerr << "WARNING:  time(): day must be a plain integer -- line "
              << linenum << "\n";
    return nil;
  }
  int day = elems[2].int_val();
  if (day<1 || day>31) {
    std::cerr << "WARNING:  time(): day " << day << " out of range (1..31) -- line "
              << linenum << "\n";
    return nil;
  }
  if (day > days_in_month(mon, yr)) {
    std::cerr << "WARNING:  time(): day " << day << " does not exist in that month/year -- line "
              << linenum << "\n";
    return nil;
  }

  if (elems[3].type()!=ComValue::IntType || elems[4].type()!=ComValue::IntType ||
      elems[5].type()!=ComValue::IntType) {
    std::cerr << "WARNING:  time(): h:m:s must be plain integers -- line "
              << linenum << "\n";
    return nil;
  }
  int hr = elems[3].int_val();
  int mn = elems[4].int_val();
  int sc = elems[5].int_val();
  if (hr<1 || hr>24) {
    std::cerr << "WARNING:  time(): hour " << hr << " out of range (1..24) -- line "
              << linenum << "\n";
    return nil;
  }
  if (hr==24) hr=0;
  if (mn<0 || mn>59) {
    std::cerr << "WARNING:  time(): minute " << mn << " out of range (0..59) -- line "
              << linenum << "\n";
    return nil;
  }
  if (sc<0 || sc>59) {
    std::cerr << "WARNING:  time(): second " << sc << " out of range (0..59) -- line "
              << linenum << "\n";
    return nil;
  }

  int fracgroups = n - 7;
  int frac[3] = {0, 0, 0};
  for (int g=0; g<fracgroups; g++) {
    if (elems[6+g].type()!=ComValue::IntType) {
      std::cerr << "WARNING:  time(): fractional-second group must be a plain integer -- line "
                << linenum << "\n";
      return nil;
    }
    int v = elems[6+g].int_val();
    if (v<0 || v>999) {
      std::cerr << "WARNING:  time(): fractional-second group " << v
                << " out of range (0..999) -- line " << linenum << "\n";
      return nil;
    }
    frac[g] = v;
  }

  ComValue tzv = elems[n-1];
  if (tzv.type()!=ComValue::IntType) {
    std::cerr << "WARNING:  time(): trailing TZ field must be a plain integer HHMM offset -- line "
              << linenum << "\n";
    return nil;
  }
  int tzval = tzv.int_val();
  int tzsign = tzval<0 ? -1 : 1;
  int atz = tzval<0 ? -tzval : tzval;
  int tzh = atz/100;
  int tzm = atz%100;
  if (tzm>59) {
    std::cerr << "WARNING:  time(): TZ minutes " << tzm << " out of range (0..59) -- line "
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

/* date()'s own colon-list vetting -- year:month[:day], with no duration
   reading to disambiguate against (unlike time()'s colon lists, which
   share a field count with min:sec/hr:min:sec/etc.), so every field here
   is unambiguously a calendar field: no year-plausibility heuristic, and
   any integer year is accepted, not just the range time()'s int64-
   nanosecond raw representation happens to need. */
static DateObj* colonlist_to_dateobj(ComTerp* comterp, AttributeValueList* avl, int linenum) {
  int n = avl->Number();
  if (n != 2 && n != 3) {
    std::cerr << "WARNING:  date(): needs a 2- or 3-element YEAR:MON[:day] list, got "
              << n << " element(s) -- line " << linenum << "\n";
    return nil;
  }

  ComValue yrv(resolve_elem(comterp, avl, 0));
  if (yrv.type()!=ComValue::IntType) {
    std::cerr << "WARNING:  date(): year must be a plain integer -- line "
              << linenum << "\n";
    return nil;
  }
  int yr = yrv.int_val();
  if (yr<0 || yr>USHRT_MAX) {
    std::cerr << "WARNING:  date(): year " << yr << " out of range (0.."
              << USHRT_MAX << ") -- line " << linenum << "\n";
    return nil;
  }

  ComValue monv(*avl->Get(1));
  int mon;
  if (!parse_month(monv, mon, linenum, "date()")) return nil;

  int day = 1;
  if (n == 3) {
    ComValue dayv(resolve_elem(comterp, avl, 2));
    if (dayv.type()!=ComValue::IntType) {
      std::cerr << "WARNING:  date(): day must be a plain integer -- line "
                << linenum << "\n";
      return nil;
    }
    day = dayv.int_val();
    if (day<1 || day>31) {
      std::cerr << "WARNING:  date(): day " << day << " out of range (1..31) -- line "
                << linenum << "\n";
      return nil;
    }
    if (day > days_in_month(mon, yr)) {
      std::cerr << "WARNING:  date(): day " << day << " does not exist in that month/year -- line "
                << linenum << "\n";
      return nil;
    }
  }

  return new DateObj(day, Date::nameOfMonth(mon), yr);
}

/* a timespec scaled to the unit :ns/:us/:ms ask for, seconds otherwise --
   shared by a fresh clock_gettime() capture and a raw/mono reading pulled
   back out of an existing TimeObj, so both round the same way; also the
   full-nsec-precision reading numfunc.c's DateObj/TimeObj +/- operators
   use, so it is declared in timefunc.h rather than kept file-local. */
long timespec_scaled(const struct timespec& ts, boolean ns, boolean us, boolean ms) {
  long sec = (long)ts.tv_sec;
  long nsec = (long)ts.tv_nsec;
  if (ns)
    return sec * 1000000000L + nsec;
  else if (us)
    return sec * 1000000L + nsec / 1000L;
  else if (ms)
    return sec * 1000L + nsec / 1000000L;
  else
    return sec;
}

/* nanoseconds since epoch/boot back to a timespec -- the inverse of
   timespec_scaled(ts, ns=true, ...).  tv_nsec stays in [0, 999999999]
   for a negative count too, by borrowing a second into tv_sec rather
   than letting C++'s truncating % leave tv_nsec negative. Declared in
   timefunc.h alongside timespec_scaled(), for the same reason. */
struct timespec nsec_to_timespec(long nsec_since) {
  struct timespec ts;
  ts.tv_sec = (time_t)(nsec_since / 1000000000L);
  ts.tv_nsec = nsec_since % 1000000000L;
  if (ts.tv_nsec < 0) {
    ts.tv_nsec += 1000000000L;
    ts.tv_sec -= 1;
  }
  return ts;
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
  static int year_sym = symbol_add("yr");
  static int month_sym = symbol_add("mo");
  static int day_sym = symbol_add("day");
  static int zone_sym = symbol_add("tz");
  ComValue yearv(stack_key(year_sym));
  ComValue monthv(stack_key(month_sym));
  ComValue dayv(stack_key(day_sym));
  ComValue zonev(stack_key(zone_sym));
  static int ms_sym = symbol_add("ms");
  static int us_sym = symbol_add("us");
  static int ns_sym = symbol_add("ns");
  static int mono_sym = symbol_add("mono");
  static int raw_sym = symbol_add("raw");
  static int delta_sym = symbol_add("delta");
  ComValue msv(stack_key(ms_sym));
  ComValue usv(stack_key(us_sym));
  ComValue nsv(stack_key(ns_sym));
  ComValue monov(stack_key(mono_sym));
  ComValue rawv(stack_key(raw_sym));
  /* blankval() as dflt is a sentinel no real :delta value can equal --
     unlike trueval() (indistinguishable from an explicit ":delta true")
     or nullval() (is_null() is is_unknown(), the same as "absent"). */
  ComValue deltav(stack_key(delta_sym, false, ComValue::blankval()));
  int linenum = funcstate() ? funcstate()->linenum() : 0;
  reset_stack();

  // cumulative, matching printOn()'s fractional-group display: :ns implies
  // ms+us+ns, :us implies ms+us, :ms is ms alone.
  int fracgroups = nsv.is_true() ? 3 : usv.is_true() ? 2 : msv.is_true() ? 1 : 0;

  /* stack_key()'s dflt/nil split separates bare (:raw alone), valued
     (:raw N), and absent -- is_num() means valued, is_null() means absent. */
  boolean raw_present = !rawv.is_null();
  boolean mono_present = !monov.is_null();
  boolean raw_valued = rawv.is_num();
  boolean mono_valued = monov.is_num();
  boolean delta_valued = !deltav.is_null() && !deltav.is_blank();

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
  } else if (!(raw_present || mono_present)) {
    /* no positional TimeObj/DateObj/colon-list and no raw/mono keyword at
       all -- capture now.  This is the same precedent date()'s own field
       keywords use over today's date when no positional DateObj is given,
       extended to time()'s own bare capture. */
    timeobj = new TimeObj();
    owns = true;
  }

  if (raw_valued || mono_valued) {
    /* :raw/:mono given a value rebuilds the TimeObj: mono zeroes on a raw
       reset (no longer the same live capture) but survives a mono-only reset. */
    struct timespec raw;
    long tzoff;
    if (raw_valued) {
      raw = nsec_to_timespec(rawv.long_val());
      struct tm tmval;
      localtime_r(&raw.tv_sec, &tmval);
      tzoff = tmval.tm_gmtoff;
    } else if (timeobj) {
      raw = timeobj->raw();
      tzoff = timeobj->tzoff();
    } else {
      raw.tv_sec = 0;
      raw.tv_nsec = 0;
      tzoff = 0;
    }

    struct timespec mono;
    if (mono_valued) {
      mono = nsec_to_timespec(monov.long_val());
    } else if (timeobj && !raw_valued) {
      mono = timeobj->mono();
    } else {
      mono.tv_sec = 0;
      mono.tv_nsec = 0;
    }

    TimeObj* result = new TimeObj(raw, tzoff);
    result->mono(mono);
    /* :raw/:mono reset the clock reading, not the kind -- a duration
       resetting its :raw stays a duration, at whatever precision it had. */
    if (timeobj) {
      result->delta(timeobj->delta());
      result->precision(timeobj->precision());
    }
    if (owns) delete timeobj;
    ComValue retval(TimeObj::class_symid(), (void*)result);
    push_stack(retval);
    return;
  }

  if (delta_valued) {
    /* :delta given a value builds a new TimeObj with that flag set,
       rather than mutating a caller-supplied one -- the same
       non-mutation precedent as :raw/:mono given a value.  With no
       positional TimeObj at all, a fresh capture is the base to override
       rather than the zero epoch :raw/:mono's own valued form starts
       from, since a bare capture is otherwise never a duration. */
    if (!timeobj) {
      timeobj = new TimeObj();
      owns = true;
    }
    TimeObj* result = new TimeObj(timeobj->raw(), timeobj->tzoff());
    result->mono(timeobj->mono());
    result->precision(timeobj->precision());
    result->delta(deltav.is_true());
    if (owns) delete timeobj;
    ComValue retval(TimeObj::class_symid(), (void*)result);
    push_stack(retval);
    return;
  }

  if (timeobj) {
    /* owns: no other holder, so free after reading a scalar field, or
       after building a display-precision copy below */
    if (rawv.is_true() || monov.is_true()) {
      const struct timespec& ts = monov.is_true() ? timeobj->mono() : timeobj->raw();
      long result = timespec_scaled(ts, nsv.is_true(), usv.is_true(), msv.is_true());
      ComValue retval(result);
      push_stack(retval);
      if (owns) delete timeobj;
    } else if (hourv.is_true()) {
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
    } else if (yearv.is_true() || monthv.is_true() || dayv.is_true()) {
      /* no date info: a duration has no calendar date at all, and the
         epoch date is the sentinel for a dateless instant -- see
         printOn() -- so both answer nil here, the same as date(t) does */
      if (timeobj->delta() ||
          (timeobj->year()==1970 && timeobj->month()==1 && timeobj->day()==1)) {
        push_stack(ComValue::nullval());
      } else if (yearv.is_true()) {
        ComValue retval(timeobj->year());
        push_stack(retval);
      } else if (monthv.is_true()) {
        ComValue retval(timeobj->month());
        push_stack(retval);
      } else {
        ComValue retval(timeobj->day());
        push_stack(retval);
      }
      if (owns) delete timeobj;
    } else if (zonev.is_true()) {
      /* the same signed +/-HHMM shape printOn() uses for the TZ field,
         as a number rather than zero-padded text */
      long off = timeobj->tzoff();
      long aoff = off<0 ? -off : off;
      int tzh = (int)(aoff/3600);
      int tzm = (int)((aoff%3600)/60);
      int magnitude = tzh*100+tzm;
      int result = off<0 ? -magnitude : magnitude;
      ComValue retval(result);
      push_stack(retval);
      if (owns) delete timeobj;
    } else if (deltav.is_blank()) {
      ComValue retval(timeobj->delta() ? ComValue::trueval() : ComValue::falseval());
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
         precision request returns a fresh TimeObj at the same instant,
         carrying over its kind and monotonic reading unchanged */
      TimeObj* copy = new TimeObj(timeobj->raw(), timeobj->tzoff());
      copy->delta(timeobj->delta());
      copy->mono(timeobj->mono());
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
  long result = timespec_scaled(ts, nsv.is_true(), usv.is_true(), msv.is_true());

  ComValue retval(result);
  push_stack(retval);
}
