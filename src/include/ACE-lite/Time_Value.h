/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Time_Value.h
 * ACE_Time_Value -- a normalized (sec, usec) duration/instant, the subset of
 * ACE's class the consumer code and AceDispatcher use.
 */

#ifndef _acelite_Time_Value_h
#define _acelite_Time_Value_h

#include <sys/time.h>

#define ACE_ONE_SECOND_IN_USECS 1000000L

class ACE_Time_Value {
public:
    ACE_Time_Value() { set(0, 0); }
    ACE_Time_Value(long sec, long usec = 0) { set(sec, usec); }
    ACE_Time_Value(const timeval& tv) { set(tv.tv_sec, tv.tv_usec); }

    // A shared zero duration (consumer code uses it as a poll timeout, e.g.
    // ctrlfunc.c's UpdateFunc).  Defined in reactor.c.
    static const ACE_Time_Value zero;

    void set(long sec, long usec) {
        tv_.tv_sec = sec;
        tv_.tv_usec = usec;
        normalize();
    }

    long sec() const { return tv_.tv_sec; }
    void sec(long s) { tv_.tv_sec = s; }
    long usec() const { return tv_.tv_usec; }
    void usec(long us) { tv_.tv_usec = us; }

    // Fill a timeval (e.g. for select()).
    operator timeval() const { return tv_; }
    const timeval* operator()() const { return &tv_; }

    ACE_Time_Value& operator-=(const ACE_Time_Value& rhs) {
        tv_.tv_sec -= rhs.tv_.tv_sec;
        tv_.tv_usec -= rhs.tv_.tv_usec;
        normalize();
        return *this;
    }
    ACE_Time_Value& operator+=(const ACE_Time_Value& rhs) {
        tv_.tv_sec += rhs.tv_.tv_sec;
        tv_.tv_usec += rhs.tv_.tv_usec;
        normalize();
        return *this;
    }

private:
    // Carry usec into the [0, 1e6) range, like ACE_Time_Value::normalize().
    void normalize() {
        if (tv_.tv_usec >= ACE_ONE_SECOND_IN_USECS) {
            do {
                tv_.tv_sec++;
                tv_.tv_usec -= ACE_ONE_SECOND_IN_USECS;
            } while (tv_.tv_usec >= ACE_ONE_SECOND_IN_USECS);
        } else if (tv_.tv_usec <= -ACE_ONE_SECOND_IN_USECS) {
            do {
                tv_.tv_sec--;
                tv_.tv_usec += ACE_ONE_SECOND_IN_USECS;
            } while (tv_.tv_usec <= -ACE_ONE_SECOND_IN_USECS);
        }
        if (tv_.tv_sec >= 1 && tv_.tv_usec < 0) {
            tv_.tv_sec--;
            tv_.tv_usec += ACE_ONE_SECOND_IN_USECS;
        } else if (tv_.tv_sec < 0 && tv_.tv_usec > 0) {
            tv_.tv_sec++;
            tv_.tv_usec -= ACE_ONE_SECOND_IN_USECS;
        }
    }

    timeval tv_;

    friend ACE_Time_Value operator-(const ACE_Time_Value&, const ACE_Time_Value&);
    friend ACE_Time_Value operator+(const ACE_Time_Value&, const ACE_Time_Value&);
    friend int operator>(const ACE_Time_Value&, const ACE_Time_Value&);
    friend int operator<(const ACE_Time_Value&, const ACE_Time_Value&);
    friend int operator==(const ACE_Time_Value&, const ACE_Time_Value&);
};

inline ACE_Time_Value operator-(const ACE_Time_Value& a, const ACE_Time_Value& b) {
    ACE_Time_Value r(a);
    r -= b;
    return r;
}
inline ACE_Time_Value operator+(const ACE_Time_Value& a, const ACE_Time_Value& b) {
    ACE_Time_Value r(a);
    r += b;
    return r;
}
inline int operator>(const ACE_Time_Value& a, const ACE_Time_Value& b) {
    return a.tv_.tv_sec > b.tv_.tv_sec ||
        (a.tv_.tv_sec == b.tv_.tv_sec && a.tv_.tv_usec > b.tv_.tv_usec);
}
inline int operator<(const ACE_Time_Value& a, const ACE_Time_Value& b) {
    return b > a;
}
inline int operator==(const ACE_Time_Value& a, const ACE_Time_Value& b) {
    return a.tv_.tv_sec == b.tv_.tv_sec && a.tv_.tv_usec == b.tv_.tv_usec;
}

#endif /* _acelite_Time_Value_h */
