// arc062-d の愚直解。K^M 通りの塗り方を全部作り、単純な輪それぞれについて色を 1 つずらした塗り方と
// Union-Find でまとめ、まとまりの数を数える (問題文の「同じと見なす」関係そのもの)。
// 輪は、いちばん小さい頂点から、それより大きい頂点だけを通る単純な道を全部たどって見つける (向き違いも両方入れる)。
// 2 重連結成分に分けず、数珠の数え上げも重複組合せも使わないので、提出とは別の考え方になる。
// 塗り方を全部持つので、小さい入力でだけ使う。
#include <cstdio>
#include <utility>
#include <vector>

int n, m, k;
std::vector<std::vector<std::pair<int, int>>> adj;  // (隣の頂点, 辺の番号)
std::vector<std::vector<int>> cycles;               // 輪に沿った辺の番号の列
std::vector<int> path;                              // start からの道の辺
std::vector<bool> on_path;

void find_cycles(int start, int v) {
  for (auto [u, e] : adj[v]) {
    if (u == start && path.size() >= 2) {
      cycles.push_back(path), cycles.back().push_back(e);
    } else if (u > start && !on_path[u]) {
      on_path[u] = true, path.push_back(e);
      find_cycles(start, u);
      on_path[u] = false, path.pop_back();
    }
  }
}

std::vector<int> parent;
int root(int x) {
  while (parent[x] != x) x = parent[x] = parent[parent[x]];
  return x;
}

int main() {
  if (scanf("%d %d %d", &n, &m, &k) != 3) return 1;
  adj.resize(n + 1), on_path.resize(n + 1);
  for (int i = 0; i < m; ++i) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    adj[a].push_back({b, i}), adj[b].push_back({a, i});
  }
  for (int s = 1; s <= n; ++s) on_path[s] = true, find_cycles(s, s), on_path[s] = false;
  long long total = 1;
  std::vector<long long> pw(m + 1, 1);
  for (int i = 0; i < m; ++i) {
    pw[i + 1] = pw[i] * k, total = pw[i + 1];
    if (total > 20000000) return fprintf(stderr, "K^M が大きすぎます\n"), 2;
  }
  parent.resize(total);
  for (int c = 0; c < total; ++c) parent[c] = c;
  std::vector<int> color(m);
  for (long long c = 0; c < total; ++c) {
    for (int i = 0; i < m; ++i) color[i] = c / pw[i] % k;
    for (const auto& cyc : cycles) {
      // 輪の上で e_i の色を e_(i+1) へ、最後の辺の色を最初の辺へ移す。
      long long next = c;
      for (size_t i = 0; i < cyc.size(); ++i) {
        int from = cyc[i], to = cyc[(i + 1) % cyc.size()];
        next += (long long)(color[from] - color[to]) * pw[to];
      }
      parent[root(c)] = root(next);
    }
  }
  long long classes = 0;
  for (long long c = 0; c < total; ++c) classes += root(c) == c;
  printf("%lld\n", classes % 1000000007);
}
