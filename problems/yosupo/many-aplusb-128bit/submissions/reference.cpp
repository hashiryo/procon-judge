// T 個の A + B。A, B は絶対値 1e37 までなので __int128 で受ける。
#include <cstdio>
#include <cstdlib>

namespace io {
using i128 = __int128_t;
static const int SZ = 1 << 16;
static char ib[SZ];
static int ip = 0, in = 0;
inline int gc() {
  if (ip == in) {
    in = (int)fread(ib, 1, SZ, stdin), ip = 0;
    if (in <= 0) return -1;
  }
  return ib[ip++];
}
inline bool rd(i128 &x) {
  int c = gc();
  while (c != '-' && (c < '0' || c > '9')) {
    if (c == -1) return false;
    c = gc();
  }
  const bool neg = c == '-';
  if (neg) c = gc();
  x = 0;
  while (c >= '0' && c <= '9') x = x * 10 + (c - '0'), c = gc();
  if (neg) x = -x;
  return true;
}
static char ob[1 << 22];
static size_t op = 0;
inline void flush() { fwrite(ob, 1, op, stdout), op = 0; }
inline void w(i128 x) {
  if (op + 48 > sizeof(ob)) flush();
  if (x == 0) {
    ob[op++] = '0', ob[op++] = '\n';
    return;
  }
  if (x < 0) ob[op++] = '-', x = -x;
  char t[48];
  int n = 0;
  while (x) t[n++] = (char)('0' + (int)(x % 10)), x /= 10;
  while (n) ob[op++] = t[--n];
  ob[op++] = '\n';
}
}  // namespace io

signed main() {
  unsigned long long t;
  {
    int c = io::gc();
    while (c < '0' || c > '9') c = io::gc();
    t = 0;
    while (c >= '0' && c <= '9') t = t * 10 + (unsigned)(c - '0'), c = io::gc();
  }
  for (unsigned long long i = 0; i < t; ++i) {
    io::i128 a, b;
    io::rd(a), io::rd(b);
    io::w(a + b);
  }
  io::flush();
}
