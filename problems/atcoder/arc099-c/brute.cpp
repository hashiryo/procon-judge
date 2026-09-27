// arc099-c の愚直解。都市の分け方 2^N 通りを全部試し、同じ州の 2 都市がどれも道で結ばれている
// ものについて、両端が同じ州にある道を数えて最小を取る。補グラフの二部グラフ判定も、成分ごとの
// 部分和の DP もしないので、UnionFind_Potentialized とは別の考え方になる。N が小さいときだけ使う。
#include <cstdio>
#include <utility>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<std::vector<bool>> road(n, std::vector<bool>(n, false));
  std::vector<std::pair<int, int>> edges(m);
  for (auto& [a, b] : edges) {
    if (scanf("%d %d", &a, &b) != 2) return 1;
    --a, --b;
    road[a][b] = road[b][a] = true;
  }
  long long best = -1;
  for (int mask = 0; mask < (1 << n); ++mask) {  // mask の i ビット目が 1 なら都市 i は Taka
    bool ok = true;
    for (int i = 0; i < n && ok; ++i)
      for (int j = i + 1; j < n && ok; ++j)
        if (((mask >> i) & 1) == ((mask >> j) & 1) && !road[i][j]) ok = false;
    if (!ok) continue;
    long long same = 0;
    for (auto [a, b] : edges) same += ((mask >> a) & 1) == ((mask >> b) & 1);
    if (best < 0 || same < best) best = same;
  }
  printf("%lld\n", best);
}
