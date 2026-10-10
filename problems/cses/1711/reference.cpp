// 期待出力を作る参照実装。転送装置を容量 1 の辺にして、1 から n への最大流を素朴な Dinic で求め、流れた辺を 1 から
// たどって道を作る (流れの保存から、使っていない流れた辺をたどれば必ず n に着く。部屋の再訪は許される)。
// ライブラリを include しない。道の組は checker.cpp が確かめるので、期待出力で判定に使うのは道の数だけ。
#include <cstdio>
#include <queue>
#include <vector>
using namespace std;
struct E { int to, rev, cap; };
vector<vector<E>> g;
vector<int> lv, it;
int dfs(int u, int t) {
  if (u == t) return 1;
  for (int& i = it[u]; i < (int)g[u].size(); ++i) {
    E& e = g[u][i];
    if (e.cap > 0 && lv[e.to] == lv[u] + 1 && dfs(e.to, t)) return e.cap -= 1, g[e.to][e.rev].cap += 1, 1;
  }
  return 0;
}
int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  g.assign(n + 1, {});
  vector<pair<int, int>> fw(m);  // 辺 i の (頂点, 隣接の位置)
  for (int i = 0; i < m; ++i) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    fw[i] = {a, (int)g[a].size()};
    g[a].push_back({b, (int)g[b].size(), 1}), g[b].push_back({a, (int)g[a].size() - 1, 0});
  }
  int k = 0;
  for (;;) {
    lv.assign(n + 1, -1);
    queue<int> q;
    lv[1] = 0, q.push(1);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (auto& e : g[u]) if (e.cap > 0 && lv[e.to] < 0) lv[e.to] = lv[u] + 1, q.push(e.to);
    }
    if (lv[n] < 0) break;
    it.assign(n + 1, 0);
    while (dfs(1, n)) ++k;
  }
  // 流れた辺 (容量が 0 になった元の向きの辺) を頂点ごとに並べ、1 からたどる。
  vector<vector<int>> out(n + 1);
  for (int i = 0; i < m; ++i) {
    auto [a, p] = fw[i];
    if (g[a][p].cap == 0) out[a].push_back(g[a][p].to);
  }
  printf("%d\n", k);
  for (int r = 0; r < k; ++r) {
    vector<int> route{1};
    for (int v = 1; v != n;) {
      int w = out[v].back();
      out[v].pop_back();
      route.push_back(w), v = w;
    }
    printf("%d\n", (int)route.size());
    for (size_t i = 0; i < route.size(); ++i) printf("%d%c", route[i], i + 1 == route.size() ? '\n' : ' ');
  }
}
