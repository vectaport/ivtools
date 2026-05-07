/*
 * Copyright (c) 2026 Scott E. Johnston
 * Copyright (c) 1994-1999 Vectaport Inc.
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

#ifndef vecstream_h
#define vecstream_h

class vecstream {
 public:
 vecstream() : _rpos(0) {_good = true;}
 vecstream(const char* buf, int len) : _buf(buf, buf+len), _rpos(0)
    {_good = true;}
  
  void write(const char* buf, int len) {
    _buf.insert(_buf.end(), buf, buf+len);
  }
  
  void get(char* buf, int len) {
    int avail = _buf.size() - _rpos;
    int n = min(len-1, avail);
    // stop at newline
    for (int i=0; i<n; i++) {
      if (_buf[_rpos+i] == '\n') { n=i; break; }
    }
    memcpy(buf, _buf.data()+_rpos, n);
    buf[n] = '\0';
    _rpos += n;
  }
  
  void get(char& c) {
    if (_rpos < (int)_buf.size())
      c = _buf[_rpos++];
    else
      _good = false;
  }

  void good(boolean val) { _good = val; }
  boolean good() { return _good && _rpos < (int)_buf.size(); }
  boolean eof() { return _rpos >= (int)_buf.size(); }
  
  const char* str() { return _buf.data(); }
  int pcount() { return (int)_buf.size(); }
  int tellg() { return _rpos; }
  int tellp() { return (int)_buf.size(); }
  void seekp(int pos) { _buf.resize(pos); }
  void freeze(int) {}  // no-op, compatibility only
  
 private:
  std::vector<char> _buf;
  int _rpos;
  boolean _good;
};

#endif
