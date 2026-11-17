// arc100-e の期待出力を作る参照実装 (ライブラリに依存しない)。S ごとに S の部分集合の添字の値の上位 2 個を 1 bit ずつの
// ゼータ変換で求め、K 以下の S での 2 個の和の最大を前から取る。小さい入力では brute.cpp と突き合わせてある
// (pj testdata crosscheck)。
#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  const int m = 1 << n;
  std::vector<std::pair<long long, long long>> t(m);
  for (auto &p : t) {
    if (scanf("%lld", &p.first) != 1) return 1;
    p.second = 0;
  }
  for (int b = 1; b < m; b <<= 1)
    for (int s = 0; s < m; ++s)
      if (s & b) {
        auto &x = t[s];
        const auto y = t[s ^ b];
        long long v[4] = {x.first, x.second, y.first, y.second};
        std::sort(v, v + 4, [](long long p, long long q) { return p > q; });
        x = {v[0], v[1]};
      }
  long long best = 0;
  for (int k = 1; k < m; ++k) {
    best = std::max(best, t[k].first + t[k].second);
    printf("%lld\n", best);
  }
}
