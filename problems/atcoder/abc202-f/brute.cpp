// abc202-f の愚直解。3 点以上の部分集合を全部試し、凸包を Andrew の方法で作って、
// 面積の 2 倍を頂点の座標から直接求め、偶数なら数える。凸包の中の点を 2 のべきで数える DP も、
// 左端の点から上側と下側をたどることもしないので、提出とは別の考え方になる。O(2^N N) なので、N が 20 までのときだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

using i64 = long long;

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::pair<i64, i64>> p(n);
  for (auto& [x, y] : p)
    if (scanf("%lld %lld", &x, &y) != 2) return 1;
  std::sort(p.begin(), p.end());
  auto cross = [&](int o, int a, int b) {
    return (p[a].first - p[o].first) * (p[b].second - p[o].second) - (p[a].second - p[o].second) * (p[b].first - p[o].first);
  };
  const i64 MOD = 1000000007;
  i64 count = 0;
  std::vector<int> sub, hull;
  for (int mask = 0; mask < (1 << n); ++mask) {
    if (__builtin_popcount(mask) < 3) continue;
    sub.clear();
    for (int i = 0; i < n; ++i)
      if (mask >> i & 1) sub.push_back(i);  // x で並んだ順
    // 下側と上側をつないで反時計回りの凸包を作る。
    hull.assign(2 * sub.size(), 0);
    int k = 0;
    for (int i : sub) {
      while (k >= 2 && cross(hull[k - 2], hull[k - 1], i) <= 0) --k;
      hull[k++] = i;
    }
    for (int j = (int)sub.size() - 2, t = k + 1; j >= 0; --j) {
      int i = sub[j];
      while (k >= t && cross(hull[k - 2], hull[k - 1], i) <= 0) --k;
      hull[k++] = i;
    }
    --k;  // 最後の点は最初の点と同じ
    i64 area2 = 0;
    for (int i = 0; i < k; ++i) {
      auto [x1, y1] = p[hull[i]];
      auto [x2, y2] = p[hull[(i + 1) % k]];
      area2 += x1 * y2 - x2 * y1;
    }
    if (area2 % 2 == 0) count = (count + 1) % MOD;
  }
  printf("%lld\n", count);
}
