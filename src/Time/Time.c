/* Time.c -- implementation of class Time

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2033
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-1111
	uucp: uunet!nih-csl!kgorlen
	Internet: kgorlen@alw.nih.gov
	December, 1985

Function:
	
Provides an object that represents a Time, stored as the number of
seconds since January 1, 1901, GMT.

/*
  Modified by Vectaport Inc.  - added operator << plus other small stuff.
*/

#include <Time/Date.h>
#include <Time/Time.h>
#include <iomanip.h>

#include <stdio.h>

const unsigned VERSION =2;

#define	THIS	Time
#define	BASE	Object

#if defined(SYSV) && ! defined(hpux)

#include <time.h>

#define TIME_ZONE timezone
#define DST_OBSERVED daylight
#define BASE_CLASSES BASE::desc()
#define MEMBER_CLASSES
#define VIRTUAL_BASE_CLASSES Object::desc()

#endif


/************************ end of edits ************************************************/
static long TIME_ZONE;          /* seconds west of GMT */
static int DST_OBSERVED;        /* flags U.S. daylight saving time observed */

static void inittimezone() {
	struct timeval tval;            /* see <sys/time.h> */
	struct timezone tz;             /* see <sys/time.h> */
	gettimeofday(&tval,&tz);
	TIME_ZONE = 60*(tz.tz_minuteswest);
	DST_OBSERVED = tz.tz_dsttime;
}

#define BASE_CLASSES BASE::desc()
#define MEMBER_CLASSES
#define VIRTUAL_BASE_CLASSES Object::desc()


static const unsigned long seconds_in_day = 24*60*60;
static const Date refDate(0L);
static const Date maxDate(31,"Dec",2036);
	
Time Time::localTime(const Date& date, hourTy h, minuteTy m, secondTy s)
/*
	Return a local Time for the specified Standard Time date, hour, minute,
	and second.
*/
{
	clockTy t = (seconds_in_day*(date-refDate) + 60L*60L*h + 60L*m + s);
	if ( !date.between(refDate,maxDate) || (TIME_ZONE < 0 && t < -TIME_ZONE) )
	    fprintf(stderr,"Date range error %d %s %d\n", date.dayOfMonth(),date.nameOfMonth(),date.year());
	
	return Time(t);
}

Time::Time()
/*
	Construct a Time for this instant.
*/
{
	sec = time(0);
	unsigned long nsecs = 2177452800Lu;	/* seconds from 1/1/01 to 1/1/70 */
	sec += nsecs;
}

Time::Time(hourTy h, minuteTy m, secondTy s, boolean dst)
/*
	Construct a Time for today at the specified (local) hour, minute, and
	second.
*/
{
	sec = Time(Date(),h,m,s,dst).sec;
}


Time::Time(const Date& date, hourTy h, minuteTy m, secondTy s, boolean dst)
/*
	Construct a Time for the specified (local) Date, hour, minute, and
	second.
*/
{
	sec = localTime(date,h,m,s).sec-3600;
	if (isDST()) {
		sec += 3600;
		if (isDST() || dst) sec -= 3600;
	}
	else {
		sec += 3600;
		if (isDST()) 
		    fprintf(stderr,"Bad time error %d %s %d %2d:%2d:%2d %s\n",
			    date.dayOfMonth(),date.nameOfMonth(),date.year(),
			    h,m,s,(dst?"DST":""));
		    }
	sec += TIME_ZONE;				// adjust to GMT 
}

Date Time::date() const
/*
	Convert a Time to a local Date
*/
{
//	return Date((int)(localTime().sec/seconds_in_day));	4.2 cc bug
	long daycount = (long)(localTime().sec/seconds_in_day);
	return Date(daycount);
}

boolean Time::between(const Time& a, const Time& b) const
{
	return *this >= a && *this <= b;
}

#if defined(NO_NESTED_TYPES) || defined(ATT_R21)
hourTy Time::hour() const
#else
Time::hourTy Time::hour() const
#endif
/*
	Return the hour of this Time in local time; i.e., adjust for
	time zone and Daylight Savings Time.
*/
{
	return localTime().hourGMT();
}

#if defined(NO_NESTED_TYPES) || defined(ATT_R21)
hourTy Time::hourGMT() const
#else
Time::hourTy Time::hourGMT() const
#endif
/*
	Return the hour of this Time in GMT.
*/
{
	return (sec % 86400) / 3600;
}

Time Time::beginDST(unsigned year)
/*
	Return the local Standard Time at which Daylight Savings Time
	begins in the specified year.
*/
{
	if (year==1974) {
		return localTime(Date(6,"Jan",1974),2);
	}
	if (year==1975) {
		return localTime(Date(23,"Feb",1975),2);
	}
	if (year<=1986) {
		return localTime(Date(30,"Apr",year).previous("Sun"),2);
	}
	else  {
	    	return localTime(Date(31,"Mar",year).previous("Sun")+7,2);
	}
}

Time Time::endDST(unsigned year)
/*
	Return the local Standard Time at which Daylight Savings Time
	ends in the specified year.
*/
{
	return localTime(Date(31,"Oct",year).previous("Sun"),2-1);
}

boolean Time::isDST() const
/*
	Return YES if this local Standard Time should be adjusted
	for Daylight Savings Time.
*/
{
//	unsigned year = Date((unsigned)(this->sec/seconds_in_day)).year();  4.2 cc bug
	long daycount = (long)(this->sec/seconds_in_day);
	unsigned year = Date(daycount).year();
#ifdef BUGbC3035
// sorry, not implemented: temporary of class <name> with destructor needed in <expr> expression
	if (DST_OBSERVED && *this >= beginDST(year) && *this < endDST(year))
		return YES;
#else
	if (DST_OBSERVED)
		if (*this >= beginDST(year))
			if (*this < endDST(year)) return true;
#endif
	return false;
}

Time Time::localTime() const
/*
	Adjusts this GM Time for local time zone and Daylight Savings Time.
*/
{
	Time local_time(sec-TIME_ZONE);
	if (local_time.isDST()) local_time.sec += 3600;
	return local_time;
}

#if defined(NO_NESTED_TYPES) || defined(ATT_R21)
minuteTy Time::minute() const
#else
Time::minuteTy Time::minute() const
#endif
/*
	Return the minute of this Time in local time; i.e., adjust
	for time zone and Daylight Savings Time.
*/
{
	return localTime().minuteGMT();
}

#if defined(NO_NESTED_TYPES) || defined(ATT_R21)
minuteTy Time::minuteGMT() const
#else
Time::minuteTy Time::minuteGMT() const
#endif
/*
	Return the minute of this Time in GMT.
*/
{
	return ((sec % 86400) % 3600) / 60;
}

#if defined(NO_NESTED_TYPES) || defined(ATT_R21)
secondTy Time::second() const
#else
Time::secondTy Time::second() const
#endif
/*
	Return the second of this Time.
*/
{
	return ((sec % 86400) % 3600) % 60;
}

Time Time::max(const Time& t) const
{
	if (t < *this) return *this;
	return t;
}

Time Time::min(const Time& t) const
{
	if (t > *this) return *this;
	return t;
}

void Time::printOn(ostream& strm) const
{
	register unsigned hh = hour();
	this->date().printOn(strm);
 	strm << ' ' << ((hh <= 12) ? hh : hh-12) << ':';
 	strm << setfill('0') << setw(2) << minute() << ':';
 	strm << setfill('0') << setw(2) << second() << ' ';
	if (hh < 12) strm << "am";
	else strm << "pm";
}

ostream& operator<< (ostream& out, const Time& tv) {
  tv.printOn(out);
  return out;
}
