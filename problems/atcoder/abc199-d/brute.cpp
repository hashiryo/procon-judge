// abc199-d の愚直解。頂点 1, 2, ..., N の順に 3 色のどれかを置いていき、番号の小さい隣と同じ色になったら
// そこで打ち切る。最後まで置けた塗り方を数える。独立集合への分け方を数えないので、
// colorings_using_exactly_k_colors_num とは別の考え方になる。小さいグラフか、辺の多いグラフでだけ使う。
#include <cstdio>
#include <vector>

int n;
std::vector<std::vector<int>> lower;  // lower[v]: v より番号の小さい隣
int color[20];

long long count(int v) {
  if (v == n) return 1;
  long long total = 0;
  for (int c = 0; c < 3; ++c) {
    bool ok = true;
    for (int u : lower[v]) ok = ok && color[u] != c;
    if (!ok) continue;
    color[v] = c;
    total += count(v + 1);
  }
  return total;
}

int main() {
  int m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  lower.assign(n, {});
  for (int i = 0; i < m; ++i) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    --a, --b;
    if (a < b) lower[b].push_back(a);
    else lower[a].push_back(b);
  }
  printf("%lld\n", count(0));
}
