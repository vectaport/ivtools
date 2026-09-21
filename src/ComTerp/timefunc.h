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
// carrying a separate flag for that.
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

 protected:
  void breakdown(struct tm&) const;

  struct timespec _raw;
  struct timespec _mono;
  long _tzoff;
  int _precision;

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
// integer clock dump with :raw/:mono. Given a TimeObj, reads a field off
// it (:hr/:min/:sec) or sets its printed fractional precision
// (:ms/:us/:ns); given a DateObj, returns noon UTC that date -- the
// inverse of date()'s TimeObj-to-DateObj conversion. Given a plain
// hr:min:sec or full y:Mon:d:h:m:s[:ms:us:ns]:TZ colon list instead,
// vets it into a TimeObj (colonlist_to_timeobj()) -- ':' itself never
// does this, so time() is the explicit ask that can warn loudly at this
// call site on a bad literal rather than falling back silently. With no
// positional argument, :hr/:min/:sec read that field off a fresh
// capture, the same precedent date()'s own field keywords use over
// today's date when no positional DateObj is given.
// Sub-second units need 64 bits -- milliseconds since the epoch already
// exceed a 32-bit int -- so the :raw/:mono integer dump is always a long,
// seconds included rather than changing type with the keyword.
class TimeFunc : public ComFunc {
public:
    TimeFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "timeobj|long = %s([timeobj|dateobj|hr:min:sec|y:Mon:d:h:m:s[:ms:us:ns]:TZ] :hr :min :sec :yr :mo :day :zn :raw :mono :ms :us :ns) -- returns or inspects a TimeObj, lands a DateObj at noon, parses a colon separated list for time formats"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":hr        hour of a TimeObj/DateObj argument, or of a fresh capture with none",
	":min       minute of a TimeObj/DateObj argument, or of a fresh capture with none",
	":sec       second of a TimeObj/DateObj argument, or of a fresh capture with none",
	":yr        calendar year of a TimeObj/DateObj argument, or of a fresh capture with",
	"           none; nil if the TimeObj carries no date (the epoch-date sentinel)",
	":mo        calendar month (1-12) of a TimeObj/DateObj argument, or of a fresh",
	"           capture with none; nil if the TimeObj carries no date",
	":day       calendar day of month of a TimeObj/DateObj argument, or of a fresh",
	"           capture with none; nil if the TimeObj carries no date",
	":zn        UTC zone offset of a TimeObj/DateObj argument, or of a fresh capture",
	"           with none, as a signed +/-HHMM integer (e.g. -700, 200)",
	":raw       seconds since the epoch: an actual date, comparable with",
	"           date() and with another machine.  The default clock, and",
	"           what a unit keyword on its own implies for the integer dump",
	":mono      a monotonic reading instead: no epoch, so not a date and",
	"           not comparable with one, but safe for measuring how long",
	"           something took -- it cannot step backwards",
	":ms        with :raw/:mono, milliseconds rather than seconds; otherwise",
	"           the returned TimeObj prints one fractional-second group",
	":us        with :raw/:mono, microseconds rather than seconds; otherwise",
	"           the returned TimeObj prints two fractional-second groups",
	":ns        with :raw/:mono, nanoseconds rather than seconds, at the",
	"           clock's real resolution; otherwise three fractional-second groups",
	nil
      };
      return keys;
    }
};

#endif /* !defined(_datefunc_h) */

