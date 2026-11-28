// A + B を出すだけ。読み書きの速さを見る問題なので、fread でまとめて読む。
#include <cstdio>
#include <cstdlib>
signed main() {
  static char buf[1 << 12];
  const size_t n = fread(buf, 1, sizeof(buf), stdin);
  long long v[2] = {0, 0};
  int k = 0;
  bool in = false;
  for (size_t i = 0; i < n && k < 2; ++i) {
    const char c = buf[i];
    if (c >= '0' && c <= '9') v[k] = v[k] * 10 + (c - '0'), in = true;
    else if (in) ++k, in = false;
  }
  printf("%lld\n", v[0] + v[1]);
}
