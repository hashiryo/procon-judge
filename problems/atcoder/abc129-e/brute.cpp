// abc129-e の愚直解。a + b <= L を満たす組 (a, b) を全部調べ、a + b == a xor b のものを数える。
// 桁ごとの状態を持つオートマトンで数えないので、提出とは別の考え方になる。L が小さいときだけ使う。
#include <cstdio>

int main() {
  char s[64];
  if (scanf("%63s", s) != 1) return 1;
  long long L = 0;
  for (char* p = s; *p; ++p) L = L * 2 + (*p - '0');
  long long count = 0;
  for (long long a = 0; a <= L; ++a)
    for (long long b = 0; a + b <= L; ++b)
      if (a + b == (a ^ b)) ++count;
  printf("%lld\n", count % 1000000007);
}
