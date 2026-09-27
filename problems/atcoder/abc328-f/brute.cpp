// abc328-f の愚直解。i ごとに S ∪ {i} の式を辺にしたグラフを作り、成分ごとに幅優先で X を決め直して、
// 全部の式を満たすかを確かめる。前の i の結果を持ち越さずに毎回はじめから解くので、
// UnionFind_Potentialized とは別の考え方になる。O(Q (N + Q)) なので小さい入力でだけ使う。
#include <cstdio>
#include <queue>
#include <vector>

struct Query {
  int a, b;
  long long d;  // X_a - X_b = d
};

int n;

bool good(const std::vector<Query>& qs) {
  std::vector<std::vector<std::pair<int, long long>>> g(n);  // (行き先, X_to - X_from)
  for (const Query& q : qs) g[q.a].push_back({q.b, -q.d}), g[q.b].push_back({q.a, q.d});
  std::vector<bool> seen(n, false);
  std::vector<long long> x(n, 0);
  for (int s = 0; s < n; ++s) {
    if (seen[s]) continue;
    seen[s] = true;
    std::queue<int> bfs;
    bfs.push(s);
    while (!bfs.empty()) {
      int u = bfs.front();
      bfs.pop();
      for (auto [v, w] : g[u])
        if (!seen[v]) seen[v] = true, x[v] = x[u] + w, bfs.push(v);
    }
  }
  for (const Query& q : qs)
    if (x[q.a] - x[q.b] != q.d) return false;
  return true;
}

int main() {
  int q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<Query> all(q), s;
  for (auto& e : all) {
    if (scanf("%d %d %lld", &e.a, &e.b, &e.d) != 3) return 1;
    --e.a, --e.b;
  }
  std::vector<int> picked;
  for (int i = 0; i < q; ++i) {
    s.push_back(all[i]);
    if (good(s)) picked.push_back(i + 1);
    else s.pop_back();
  }
  for (size_t i = 0; i < picked.size(); ++i) printf(i ? " %d" : "%d", picked[i]);
  puts("");
}
