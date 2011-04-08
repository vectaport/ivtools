#ifndef leak_checker_h
#define leak_checker_h

#include <stream.h>

class LeakChecker {
public:
    void create() { _alive++; }
    void destroy() { _alive--; }

    LeakChecker(const char* c) : _alive(0), _class(c) {}
    ~LeakChecker();
private:
    int _alive;
    const char* _class;
};

inline LeakChecker::~LeakChecker() {
    cerr << "LEAKCHECKER: " << _class << ", " << _alive << "\n";
}

#endif
