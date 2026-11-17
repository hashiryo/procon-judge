// arc136-d の期待出力を作る参照実装 (ライブラリに依存しない)。値ごとの個数 c を 10 進 6 桁の桁ごとに累積して、c[x] を桁ごとに
// x 以下の値の個数にする。A_i ごとに c[999999 - A_i] を足し (桁ごとに 9 - 桁 以下の相手)、自分自身との組 (どの桁も 4 以下) を
// 引いて 2 で割る。小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<int> a(n);
  std::vector<long long> c(1000000, 0);
  for (auto &v : a) {
    if (scanf("%d", &v) != 1) return 1;
    ++c[v];
  }
  for (int p = 1; p < 1000000; p *= 10)
    for (int x = 0; x < 1000000; ++x)
      if (x / p % 10) c[x] += c[x - p];
  long long ans = 0;
  for (int v : a) {
    ans += c[999999 - v];
    bool self = true;
    for (int x = v; x; x /= 10) self &= x % 10 <= 4;
    ans -= self;
  }
  printf("%lld\n", ans / 2);
}
