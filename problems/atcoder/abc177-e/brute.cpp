// abc177-e の愚直解。全部の組 (i, j) の最大公約数を 1 つずつ計算して pairwise かを決め、
// 全体の最大公約数で setwise と not を分ける。倍数の和で数える畳み込み (gcd_convolve) を使わないので、
// 提出とは別の考え方になる。O(N^2 log A) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

long long gcd(long long a, long long b) {
  while (b) {
    long long t = a % b;
    a = b, b = t;
  }
  return a;
}

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> a(n);
  for (auto &v : a)
    if (scanf("%lld", &v) != 1) return 1;
  bool pairwise = true;
  for (int i = 0; i < n && pairwise; ++i)
    for (int j = i + 1; j < n; ++j)
      if (gcd(a[i], a[j]) != 1) {
        pairwise = false;
        break;
      }
  long long g = 0;
  for (long long v : a) g = gcd(g, v);
  puts(pairwise ? "pairwise coprime" : g == 1 ? "setwise coprime" : "not coprime");
}
