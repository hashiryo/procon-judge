// T 個の A + B。T <= 1e6、A, B <= 1e18 なので和は 2e18 で 64 bit に収まる。
// 読みはまとめ読みのストリーム、書きはバッファに溜めて最後に 1 回。
#include <cstdio>
#include <cstdlib>

namespace io {
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
inline bool rd(unsigned long long &x) {
  int c = gc();
  while (c < '0' || c > '9') {
    if (c == -1) return false;
    c = gc();
  }
  x = 0;
  while (c >= '0' && c <= '9') x = x * 10 + (unsigned)(c - '0'), c = gc();
  return true;
}
static char ob[1 << 22];
static size_t op = 0;
inline void flush() { fwrite(ob, 1, op, stdout), op = 0; }
inline void wu(unsigned long long x) {
  if (op + 24 > sizeof(ob)) flush();
  if (!x) ob[op++] = '0';
  char t[24];
  int n = 0;
  while (x) t[n++] = (char)('0' + x % 10), x /= 10;
  while (n) ob[op++] = t[--n];
  ob[op++] = '\n';
}
}  // namespace io

signed main() {
  unsigned long long t, a, b;
  if (!io::rd(t)) return 0;
  for (unsigned long long i = 0; i < t; ++i) {
    io::rd(a), io::rd(b);
    io::wu(a + b);
  }
  io::flush();
}
