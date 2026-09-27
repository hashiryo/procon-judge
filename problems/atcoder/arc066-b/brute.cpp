// arc066-b の愚直解。a + b <= N を満たす a, b を全部試し、出てきた (a xor b, a + b) の組を数える。
// a xor b <= a + b なので u <= N は自動で満たす。桁ごとのオートマトンも漸化式も使わないので、
// 提出とは別の考え方になる。O(N^2) なので、小さい N でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  long long n;
  if (scanf("%lld", &n) != 1 || n > 20000) return 1;
  // (v, u) を見たかどうかを 1 ビットずつ持つ。
  const long long w = n + 1;
  std::vector<unsigned long long> seen((w * w + 63) / 64);
  long long count = 0;
  for (long long a = 0; a <= n; ++a)
    for (long long b = 0; a + b <= n; ++b) {
      long long k = (a + b) * w + (a ^ b);
      if (!(seen[k >> 6] >> (k & 63) & 1)) seen[k >> 6] |= 1ULL << (k & 63), ++count;
    }
  printf("%lld\n", count % 1000000007);
}
