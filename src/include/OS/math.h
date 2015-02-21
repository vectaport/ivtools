/*
 * Copyright (c) 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#ifndef os_math_h
#define os_math_h

#include <OS/enter-scope.h>

/*
 * Common math operations on built-in types.
 */

#undef min
#undef max

#define declare_binary_minmax(Type) \
    static Type min(Type a, Type b); \
    static Type max(Type a, Type b)

#define implement_binary_minmax(Type) \
    inline Type Math::min(Type a, Type b) { return a < b ? a : b; } \
    inline Type Math::max(Type a, Type b) { return a > b ? a : b; }

#define declare_4_minmax(Type) \
    static Type min(Type a, Type b, Type c, Type d); \
    static Type max(Type a, Type b, Type c, Type d)

/*
 * Compiler isn't smart enough to figure out how to do a 4-way min inline
 * with single nested if-then-else.
 */

#define implement_4_minmax(Type) \
    inline Type Math::min(Type a, Type b, Type c, Type d) { \
	Type r1 = min(a, b), r2 = min(c, d); \
	    return min(r1, r2); \
	} \
\
    inline Type Math::max(Type a, Type b, Type c, Type d) { \
	Type r1 = max(a, b), r2 = max(c, d); \
	return max(r1, r2); \
    }

class Math {
public:
    declare_binary_minmax(int);
    declare_binary_minmax(unsigned);
    declare_binary_minmax(long);
    declare_binary_minmax(unsigned long);
    declare_binary_minmax(float);
    declare_binary_minmax(double);
    declare_4_minmax(int);
    declare_4_minmax(float);
    declare_4_minmax(double);

    static int abs(int);
    static long abs(long);
    static double abs(double);

    static int round(float);
    static int round(double);

    static boolean equal(float x, float y, float e);
    static boolean equal(double x, double y, double e);
};

implement_binary_minmax(int)
implement_binary_minmax(unsigned)
implement_binary_minmax(long)
implement_binary_minmax(unsigned long)
implement_binary_minmax(float)
implement_binary_minmax(double)

implement_4_minmax(int)
implement_4_minmax(float)
implement_4_minmax(double)

inline int Math::round(float x) { return x > 0 ? int(x+0.5) : -int(-x+0.5); }
inline int Math::round(double x) { return x > 0 ? int(x+0.5) : -int(-x+0.5); }

inline boolean Math::equal(float x, float y, float e) {
    return x - y < e && y - x < e;
}

inline boolean Math::equal(double x, double y, double e) {
    return x - y < e && y - x < e;
}

#ifndef M_PI
const double M_PI = 3.14159265358979323846;		// per CRC handbook, 14th. ed.
#endif
#ifndef M_PI_2
const double M_PI_2 = (M_PI/2.0);				// PI/2
#endif
#ifndef M2_PI
const double M2_PI = (M_PI*2.0);				// PI*2
#endif

inline float degrees(float rad) { return rad * (float)(180 / M_PI); }
inline float radians(float deg) { return deg * (float)(M_PI / 180); }

inline double degrees(double rad) { return rad * (180 / M_PI); }
inline double radians(double deg) { return deg * (M_PI / 180); }

#endif
