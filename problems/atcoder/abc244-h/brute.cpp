// abc244-h の愚直解。質問ごとに、それまでに足した点を全部見て A x + B y の最大を取る。
// 凸包も直線の集合も持たず、整数だけで計算するので、long double で直線を比べる ConvexHullTrick_XY
// とは別の考え方になる。O(Q^2) だが、1 回が掛け算 2 つと足し算なので Q = 2 * 10^5 でも数秒で終わる。
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

int main() {
  int q;
  if (scanf("%d", &q) != 1) return 1;
  std::vector<int32_t> xs(q), ys(q);
  for (int i = 0; i < q; ++i) {
    int x, y, a, b;
    if (scanf("%d %d %d %d", &x, &y, &a, &b) != 4) return 1;
    xs[i] = x, ys[i] = y;
    int64_t best = INT64_MIN;
    for (int j = 0; j <= i; ++j) best = std::max(best, int64_t(a) * xs[j] + int64_t(b) * ys[j]);
    printf("%lld\n", (long long)best);
  }
}
