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
  DateObj(DateObj*); // copy
  DateObj(); // today
  virtual ~DateObj();

  Date *date() {return _date;}

  DateObj(int day, const char* monthName, int year); // from a calendar breakdown, e.g. a TimeObj's date

 protected:
  Date *_date;

  CLASS_SYMID("DateObj");
};

//: A calendar instant anchored at construction -- wall-clock seconds since
// the epoch (raw()) plus the UTC offset captured with it (tzoff()), so a
// stored or round-tripped TimeObj keeps the offset it had at capture
// rather than picking up whatever zone the reading process happens to run
// in. mono() is a monotonic reading, meaningful only for comparing two
// live time() captures within the same run, and is zeroed on every other
// construction path. hour/minute/second/year/month/day are all computed
// on demand from raw()+tzoff(), never stored redundantly. The Unix epoch
// date (1970-01-01 UTC) is the sentinel a bare hr:min:sec construction
// lands on -- printOn() and date() read it as "no date info" rather than
// carrying a separate flag for that. A TimeObj with delta() true is instead
// a duration -- raw() holds elapsed seconds, not a calendar instant, and
// printOn() decomposes it on a separate path (see delta()).
class TimeObj {
 public:
  TimeObj(int hour, int minute, int second); // hr:min:sec only, tzoff 0 -- lands on the epoch date when hour is under 24
  TimeObj(const struct timespec& raw, long tzoff); // a specific captured or computed instant
  TimeObj(); // now -- dual clock capture plus the local UTC offset at capture
  virtual ~TimeObj();

  int hour() const;
  int minute() const;
  int second() const;
  int year() const;
  int month() const;
  int day() const;

  const struct timespec& raw() const {return _raw;}
  const struct timespec& mono() const {return _mono;}
  long tzoff() const {return _tzoff;}

  // number of zero-padded fractional-second groups (0-3: none, ms, ms+us,
  // ms+us+ns) printOn() appends -- a display precision chosen at
  // capture/parse time, not part of the calendar breakdown itself.
  int precision() const {return _precision;}
  void precision(int groups) {_precision = groups;}

  void printOn(ostream& out) const;

  // sets the monotonic reading directly -- for time()'s :mono keyword
  // given an explicit value, constructing or resetting a TimeObj outside
  // any live clock_gettime() capture.
  void mono(const struct timespec& m) {_mono = m;}

  // true for a duration (elapsed time, not anchored to any date) rather
  // than a calendar instant -- selects printOn()'s decomposition path.
  boolean delta() const {return _delta;}
  void delta(boolean d) {_delta = d;}

 protected:
  void breakdown(struct tm&) const;

  struct timespec _raw;
  struct timespec _mono;
  long _tzoff;
  int _precision;
  boolean _delta;

  CLASS_SYMID("TimeObj");
};

//: date makes date from days since 1/1/1901, a string, or a TimeObj's
// calendar date -- nil if that TimeObj carries no date (the epoch-date
// sentinel, see TimeObj above).
class DateFunc : public ComFunc {
public:
    DateFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "dateobj|int = %s([num|str|dateobj|timeobj] :day :month :year :daymo :weekday) -- create date from days since 1/1/1901, a string, or a TimeObj's date"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":day       return day of year in dateobj",
	":month     return month of year in dateobj",
	":year      return year in dateobj",
	":daymo     return day of month in dateobj",
	":weekday   return name of weekday in dateobj",
	nil
      };
      return keys;
    }
};

//: time returns the current instant as a TimeObj by default, or an
// integer clock dump with :raw/:mono; given a value instead (:raw N,
// :mono N), :raw/:mono construct a new TimeObj from N nanoseconds, or
// reset that field on a positional TimeObj rather than dumping it --
// the same nanosecond count :raw :ns/:mono :ns dumps, so the two round
// trip exactly. Given a
// TimeObj, reads a field off it (:hr/:min/:sec) or sets its printed
// fractional precision (:ms/:us/:ns); given a DateObj, returns noon UTC
// that date -- the inverse of date()'s TimeObj-to-DateObj conversion.
// Given a colon list instead (2 to 10 fields), vets it into a TimeObj
// (colonlist_to_timeobj()) -- ':' itself never does this, so time() is the
// explicit ask that can warn loudly at this call site on a bad literal
// rather than falling back silently. A short list (2, 3 or 4 fields) auto-
// detects instant vs. duration from its leading value: a plausible year
// reads as YEAR:MON[:day[:hr]], anything else as min:sec, hr:min:sec or
// days:h:m:s; 5 fields is always yrs:days:h:m:s; 7-to-10 is always the
// full y:Mon|1-12:d:h:m:s[:ms:us:ns]:TZ instant. With no positional
// argument, :hr/:min/:sec read that field off a fresh capture, the same
// precedent date()'s own field keywords use over today's date when no
// positional DateObj is given.
// Sub-second units need 64 bits -- milliseconds since the epoch already
// exceed a 32-bit int -- so the :raw/:mono integer dump is always a long,
// seconds included rather than changing type with the keyword.
class TimeFunc : public ComFunc {
public:
    TimeFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "timeobj|long = %s([timeobj|dateobj|min:sec|YEAR:MON|hr:min:sec|YEAR:MON:day|days:h:m:s|YEAR:MON:day:hr|yrs:days:h:m:s|y:Mon|1-12:d:h:m:s[:ms:us:ns]:TZ] :hr :min :sec :yr :mo :day :tz :raw [long] :mono [long] :delta [true|false] :ms :us :ns) -- returns or inspects a TimeObj or duration, lands a DateObj at noon, parses a 2-to-10-field colon list as an instant or duration"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":hr        hour of a TimeObj",
	":min       minute of a TimeObj",
	":sec       second of a TimeObj",
	":yr        year of a TimeObj; nil if dateless",
	":mo        month of a TimeObj; nil if dateless",
	":day       day of a TimeObj; nil if dateless",
	":tz        UTC offset of a TimeObj, as a signed +/-HHMM integer",
	":raw [long] seconds since the epoch: an actual date, comparable with",
	"           date() and with another machine.  Given a value, constructs",
	"           or resets that field from nanoseconds since the epoch",
	"           instead of dumping it.",
	":mono [long] a monotonic reading instead: no epoch, so not a date and",
	"           not comparable with one, but safe for measuring how long",
	"           something took -- it cannot step backwards.  Given a value,",
	"           constructs or resets that field from nanoseconds since",
	"           boot, the same way :raw does.",
	":delta [true|false] whether a TimeObj is a duration, not a calendar",
	"           instant.  Bare inspects it; given a value, builds a new",
	"           TimeObj with that flag set rather than mutating this one.",
	":ms        millisecond precision",
	":us        microsecond precision",
	":ns        nanosecond precision",
	nil
      };
      return keys;
    }
};

#endif /* !defined(_datefunc_h) */

