
/* Time.h -- declarations for class Time 
	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

/* Log:	Time.h,v 
 * Revision 3.11  92/07/26  14:33:59  sandy
 * include directive in form '#include <nihcl/foo.h>'
 * 
 * Revision 3.10  91/02/18  21:08:49  kgorlen
 * Release for 3rd printing of 1st edition.
 * 
 * Revision 3.1  90/12/22  08:58:38  kgorlen
 * Change NESTED_TYPES to NO_NESTED_TYPES.
 * 
 * Revision 3.0  90/05/20  00:21:46  kgorlen
 * Release for 1st edition.
 * 
*/

#ifndef	TIME_H
#define	TIME_H


#if defined(SYSV) && ! defined(hpux)

#include <time.h>

#define TIME_ZONE timezone
#define DST_OBSERVED daylight
#define BASE_CLASSES BASE::desc()
#define MEMBER_CLASSES
#define VIRTUAL_BASE_CLASSES Object::desc()

#endif

//moved from Time.c

#if defined(BSD) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)

#include <sys/time.h>
  #if defined(__NetBSD__)
    #include </usr/include/sys/time.h>
  #endif
 
#endif


#if defined(hpux)
  #include <time.h>
#endif


#if defined(linux) || defined(__sun) || defined(__alpha) || defined(__CYGWIN__)
#include <sys/time.h>

  #if defined(__DECCXX) || (defined(__sun) && !defined(__svr4__))
    extern "C" {
    int gettimeofday(struct timeval *tp, struct timezone *tzp);
     }
  #endif
#endif 

// END of edits


/*
  Modified by Vectaport, Inc.
*/

#include <OS/types.h>
#include <iostream.h>
#include_next <time.h>

#define NO_NESTED_TYPES 1

class Date;

#ifdef NO_NESTED_TYPES
typedef unsigned short hourTy;
typedef unsigned short minuteTy;
typedef unsigned short secondTy;
typedef unsigned long clockTy;
#endif

class Time {

public:			// type definitions
#ifndef NO_NESTED_TYPES
	typedef unsigned short hourTy;
	typedef unsigned short minuteTy;
	typedef unsigned short secondTy;
	typedef unsigned long clockTy;
#endif
private:
	clockTy sec;			/* seconds since 1/1/1901 */
	boolean isDST() const;
	Time localTime() const;
private:		// static member functions
	static Time localTime(const Date& date, hourTy h=0, minuteTy m=0, secondTy s=0);
	static Time beginDST(unsigned year);
	static Time endDST(unsigned year);
public:
	Time();				// current time 
	Time(clockTy s)			{ sec = s; }
	Time(hourTy h, minuteTy m, secondTy s =0, boolean dst =false);
	Time(const Date&, hourTy h =0, minuteTy m =0, secondTy s=0, boolean dst =false);
	Date date() const;
	boolean operator<(const Time& t) const	{ return sec < t.sec; }
	boolean operator<=(const Time& t) const	{ return sec <= t.sec; }
	boolean operator>(const Time& t) const	{ return sec > t.sec; }
	boolean operator>=(const Time& t) const	{ return sec >= t.sec; }
	boolean operator==(const Time& t) const	{ return sec == t.sec; }
	boolean operator!=(const Time& t) const	{ return sec != t.sec; }
	friend Time operator+(const Time& t, long s)	{ return Time(t.sec+s); }
	friend Time operator+(long s, const Time& t)	{ return Time(t.sec+s); }
	long operator-(const Time& t) const	{ return sec - t.sec; }
	Time operator-(long s) const	{ return Time(sec-s); }
	void operator+=(long s)		{ sec += s; }
	void operator-=(long s)		{ sec -= s; }
	boolean between(const Time& a, const Time& b) const;
	hourTy hour() const;		// hour in local time 
	hourTy hourGMT() const;		// hour in GMT 
	minuteTy minute() const;	// minute in local time 
	minuteTy minuteGMT() const;	// minute in GMT 
	secondTy second() const;	// second in local time or GMT 
	clockTy	seconds() const		{ return sec; }

#undef min
#undef max

	Time max(const Time&) const;
	Time min(const Time&) const;
	virtual void printOn(ostream& strm =cout) const;

	friend ostream& operator << (ostream& s, const Time&);
};

#endif /* TIMEH */
