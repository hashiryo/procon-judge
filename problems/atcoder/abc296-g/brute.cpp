// abc296-g の愚直解。点ごとに多角形の辺を全部見て、辺の向きと点への向きの外積の符号を調べる。
// 反時計回りの凸多角形なので、どれかが負なら OUT、そうでなくどれかが 0 なら ON、全部正なら IN。
// 外積は __int128 で計算する。上側と下側の凸包に分けて二分探索する提出とは別の考え方になる。
// O(NQ) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

using i128 = __int128;

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> x(n), y(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%lld %lld", &x[i], &y[i]) != 2) return 1;
  int q;
  if (scanf("%d", &q) != 1) return 1;
  while (q--) {
    long long a, b;
    if (scanf("%lld %lld", &a, &b) != 2) return 1;
    bool negative = false, zero = false;
    for (int i = 0; i < n; ++i) {
      int j = (i + 1) % n;
      i128 c = (i128)(x[j] - x[i]) * (b - y[i]) - (i128)(y[j] - y[i]) * (a - x[i]);
      if (c < 0) negative = true;
      if (c == 0) zero = true;
    }
    puts(negative ? "OUT" : zero ? "ON" : "IN");
  }
}
