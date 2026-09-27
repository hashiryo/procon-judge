// abc369-g の愚直解。青木君が選ぶ頂点の集合 S を 2^N 通り全部試す。頂点 1 から出て S を全部通って
// 1 に戻る最短の歩道の長さは、1 と S を結ぶ最小の部分木 (1 から各頂点への道の和集合) の辺の長さの和の
// 2 倍なので、それを |S| ごとに最大にする。区分線形凸関数も長い道への分解も使わないので、提出とは別の
// 考え方になる。O(2^N) なので、小さい木でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int n;
std::vector<unsigned> up;  // up[v]: v から 1 への道にある頂点 (1 を除く) の集合
std::vector<long long> low_sum, high_sum, best;

void rec(int v, unsigned used, int count) {
  if (v == n) {
    long long w = low_sum[used & 1023] + high_sum[used >> 10];
    best[count] = std::max(best[count], 2 * w);
    return;
  }
  rec(v + 1, used, count);
  rec(v + 1, used | up[v], count + 1);
}

int main() {
  if (scanf("%d", &n) != 1 || n > 20) return 1;
  std::vector<std::vector<std::pair<int, long long>>> adj(n);
  for (int i = 0; i < n - 1; ++i) {
    int u, v;
    long long l;
    if (scanf("%d %d %lld", &u, &v, &l) != 3) return 1;
    adj[u - 1].push_back({v - 1, l}), adj[v - 1].push_back({u - 1, l});
  }
  // 頂点 1 (番号 0) を根にして、親への辺の長さと、根までの道を求める。
  std::vector<int> parent(n, -1), order = {0};
  std::vector<long long> to_parent(n, 0);
  up.assign(n, 0);
  parent[0] = 0;
  for (size_t i = 0; i < order.size(); ++i)
    for (auto [x, l] : adj[order[i]])
      if (parent[x] < 0) parent[x] = order[i], to_parent[x] = l, up[x] = up[order[i]] | 1u << x, order.push_back(x);
  // 頂点の集合 (ビット) ごとの、親への辺の長さの和。下位 10 ビットと上位 10 ビットに分けて表にする。
  low_sum.assign(1024, 0), high_sum.assign(1024, 0);
  for (int s = 0; s < 1024; ++s)
    for (int b = 0; b < 10; ++b)
      if (s >> b & 1) {
        if (b < n) low_sum[s] += to_parent[b];
        if (b + 10 < n) high_sum[s] += to_parent[b + 10];
      }
  best.assign(n + 1, 0);
  rec(0, 0, 0);
  for (int k = 1; k <= n; ++k) printf("%lld\n", best[k]);
}
