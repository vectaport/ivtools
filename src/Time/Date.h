#ifndef	DATE_H
#define	DATE_H

/* Date.h -- declarations for Gregorian calendar dates

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

/* Log:	Date.h,v 
 * Revision 3.11  92/07/26  14:32:05  sandy
 * include directive in form '#include <nihcl/foo.h>'
 * 
 * Revision 3.10  91/02/18  21:06:10  kgorlen
 * Release for 3rd printing of 1st edition.
 * 
 * Revision 3.1  90/12/22  08:47:10  kgorlen
 * Change NESTED_TYPES to NO_NESTED_TYPES.
 * 
 * Revision 3.0  90/05/20  00:19:24  kgorlen
 * Release for 1st edition.
 * 
*/

/*
  Modified by Vectaport, Inc.
*/

#include <OS/types.h>
#include <iostream.h>

#define NO_NESTED_TYPES 1

#ifdef NO_NESTED_TYPES
typedef unsigned short dayTy;
typedef unsigned short monthTy;
typedef unsigned short yearTy;
typedef unsigned long  julTy;
#endif

class Date {
public:			// type definitions
#ifndef NO_NESTED_TYPES
	typedef unsigned short dayTy;
	typedef unsigned short monthTy;
	typedef unsigned short yearTy;
	typedef unsigned long  julTy;
#endif
public:			// static member functions
	static dayTy dayOfWeek(const char* dayName);
	static boolean dayWithinMonth(monthTy month, dayTy day, yearTy year);
	static dayTy daysInYear(yearTy year);
	static julTy jday(monthTy m, dayTy d, yearTy y);
	static boolean leapYear(yearTy year);
	static const char* nameOfDay(dayTy weekDayNumber);
	static const char* nameOfMonth(monthTy monthNumber);
	static monthTy numberOfMonth(const char* monthName);
private:		// private member variables
        julTy julnum;   // Julian Day Number (Not same as Julian date.  Jan. 29, 1988 
                        // is not the same as 88029 in Julian Day Number.)
	Date(julTy j)                   { julnum = j; }
	julTy parseDate(istream&);
public:
	Date();				// current date 
	Date(long dayCount);
	Date(long dayCount, yearTy referenceYear);
	Date(dayTy newDay, const char* monthName, yearTy newYear);
	Date(istream&);			// read date from stream 
	boolean operator<(const Date& date) const	{ return julnum < date.julnum; }
	boolean operator<=(const Date& date) const	{ return julnum <= date.julnum; }
	boolean operator>(const Date& date) const	{ return date < *this; }
	boolean operator>=(const Date& date) const	{ return date <= *this; }
	boolean operator==(const Date& date) const	{ return julnum == date.julnum; }
	boolean operator!=(const Date& date) const	{ return julnum != date.julnum; }
	friend Date operator+(const Date& dt, int dd)	{ return Date(dt.julnum + dd); }
	friend Date operator+(int dd, const Date& dt)	{ return Date(dt.julnum + dd); }
	int operator-(const Date& dt) const	{ return (int)(julnum - dt.julnum); }
	Date operator-(int dd) const		{ return Date(julnum - dd); }
	void operator+=(int dd)			{ julnum += dd; }
	void operator-=(int dd)			{ julnum -= dd; }
	boolean between(const Date& d1, const Date& d2) const {
		return julnum >= d1.julnum && julnum <= d2.julnum;
	}
	dayTy day() const;
	dayTy dayOfMonth() const;
	dayTy firstDayOfMonth() const		{ return firstDayOfMonth(month()); }
	dayTy firstDayOfMonth(monthTy month) const;
	boolean leap() const;
	Date max(const Date& dt) const { 
	        if (dt.julnum > julnum) return dt;
		else return *this;
        }
	void mdy(monthTy&,dayTy&,yearTy&) const;
	Date min(const Date& dt) const {
	        if (dt.julnum < julnum) return dt;
		else return *this;
        }
	monthTy	month() const;
	const char* nameOfMonth() const;
	Date previous(const char* dayName) const;
	dayTy weekDay() const;
	yearTy year() const;
	virtual void printOn(ostream& strm) const;
	void scanFrom(istream& strm);
};

#endif
