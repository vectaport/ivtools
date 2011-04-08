#ifndef _iv_iostream_h_
#define _iv_iostream_h_
#include_next <iostream.h>

#if __GNUG__>=3
#if 0
#include <unistd.h>

// from a posting to libstdc++@gcc.gnu.org by Carlo Wood
// Quick and dirty unbuffered file descriptor streambuf.
class fdbuf : public std::basic_streambuf<char, std::char_traits<char> > {
public:
  typedef std::char_traits<char> traits_type;
  typedef traits_type::int_type int_type;
private:
  int M_fd;
public:
  fdbuf(int fd) : M_fd(fd) { }
protected:
  virtual int_type overflow(int_type c = traits_type::eof())
  {
    if (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      char cp[1];
      *cp = c;
      if (write(M_fd, cp, 1) != 1)
        return traits_type::eof();
    }
    return 0;
  }
  // This would be needed if it was buffered.
  // virtual std::streamsize xsputn(char const* s, std::streamsize num) { return write(M_fd, s, num); }
};

// Unbuffered file descriptor stream.
class ofdstream : public std::ostream {
private:
  mutable fdbuf M_fdbuf;
public:
  explicit
  ofdstream(int fd) : std::ostream(&M_fdbuf), M_fdbuf(fd) { }
  fdbuf* rdbuf(void) const { return &M_fdbuf; }
};
#endif
#endif

#endif
