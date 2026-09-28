// abc330-f の愚直解。一辺 m を 0 から 10^9 の範囲で二分探索し (m が大きいほど費用は減る)、各 m で
// 正方形の左端 L の候補 (X_i と X_i - m。費用は L について凸な折れ線で、最小は折れ目にある) を全部試して、
// 各点を [L, L + m] に入れる移動回数を直接足す。y も同じ。区分線形凸関数を作らないので、提出とは別の考え方になる。
// 1 つの m で O(N^2) なので、N が 300 までのときだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

using i64 = long long;

// v の全部を幅 m の区間に入れる最小の移動回数。
i64 axis_cost(const std::vector<i64>& v, i64 m) {
  i64 best = -1;
  for (i64 base : v)
    for (i64 l : {base, base - m}) {
      i64 c = 0;
      for (i64 p : v) c += std::max<i64>(0, l - p) + std::max<i64>(0, p - (l + m));
      if (best < 0 || c < best) best = c;
    }
  return best;
}

int main() {
  int n;
  i64 k;
  if (scanf("%d %lld", &n, &k) != 2) return 1;
  std::vector<i64> x(n), y(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%lld %lld", &x[i], &y[i]) != 2) return 1;
  i64 lo = -1, hi = 1000000000;  // 一辺 10^9 なら全部の点が入る
  while (hi - lo > 1) {
    i64 m = (lo + hi) / 2;
    (axis_cost(x, m) + axis_cost(y, m) <= k ? hi : lo) = m;
  }
  printf("%lld\n", hi);
}
