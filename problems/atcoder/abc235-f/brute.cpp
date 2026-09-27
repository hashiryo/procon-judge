// abc235-f の愚直解。1 から N までの整数を全部たどり、使っている数字の集合を調べて、C を全部含むものを足す。
// 桁ごとの状態を持つ Automaton の積と DP を使わないので、提出とは別の考え方になる。O(N log N) なので、
// 小さい N でだけ使う。
#include <cstdio>
#include <cstring>

int main() {
  static char buf[10010];
  int m;
  if (scanf("%10005s %d", buf, &m) != 2) return 1;
  if (strlen(buf) > 12) return 1;  // 愚直解では扱えない大きさ
  int need = 0;
  for (int i = 0; i < m; ++i) {
    int c;
    if (scanf("%d", &c) != 1) return 1;
    need |= 1 << c;
  }
  long long n = 0;
  for (char* p = buf; *p; ++p) n = n * 10 + (*p - '0');
  const long long P = 998244353;
  long long ans = 0;
  for (long long x = 1; x <= n; ++x) {
    int used = 0;
    for (long long y = x; y > 0; y /= 10) used |= 1 << (y % 10);
    if ((used & need) == need) ans = (ans + x) % P;
  }
  printf("%lld\n", ans);
}
