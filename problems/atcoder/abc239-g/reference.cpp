// 期待出力を作る参照実装。頂点 v を入口 v と出口 v + N に分け、入口から出口へ容量 c_v (1 と N は無限) の辺、元の辺ごとに
// 両向きに出口から入口へ容量無限の辺を張り、1 の出口から N の入口への最大流を素朴な Dinic で求める。壁は、残余グラフで
// 入口に届いて出口に届かない頂点。ライブラリを include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まる)。
// 答えの頂点の集合は checker.cpp が確かめるので、期待出力で判定に使うのは最適値だけ。
#include <array>
#include <cstdio>
#include <queue>
#include <vector>
using namespace std;
using i64 = long long;
struct E { int to, rev; i64 cap; };
vector<vector<E>> g;
vector<int> lv, it;
void add(int u, int v, i64 c) { g[u].push_back({v, (int)g[v].size(), c}), g[v].push_back({u, (int)g[u].size() - 1, 0}); }
i64 dfs(int u, int t, i64 f) {
  if (u == t) return f;
  for (int& i = it[u]; i < (int)g[u].size(); ++i) {
    E& e = g[u][i];
    if (e.cap > 0 && lv[e.to] == lv[u] + 1)
      if (i64 d = dfs(e.to, t, min(f, e.cap)); d > 0) return e.cap -= d, g[e.to][e.rev].cap += d, d;
  }
  return 0;
}
int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  vector<array<int, 2>> es(m);
  for (auto& [a, b] : es) if (scanf("%d %d", &a, &b) != 2) return 1;
  vector<i64> c(n + 1);
  for (int i = 1; i <= n; ++i) if (scanf("%lld", &c[i]) != 1) return 1;
  const i64 INF = 1LL << 60;
  g.assign(2 * n + 2, {});
  for (int v = 1; v <= n; ++v) add(v, v + n, v == 1 || v == n ? INF : c[v]);
  for (auto [a, b] : es) add(a + n, b, INF), add(b + n, a, INF);
  const int s = 1 + n, t = n;
  i64 flow = 0;
  for (;;) {
    lv.assign(2 * n + 2, -1);
    queue<int> q;
    lv[s] = 0, q.push(s);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (auto& e : g[u]) if (e.cap > 0 && lv[e.to] < 0) lv[e.to] = lv[u] + 1, q.push(e.to);
    }
    if (lv[t] < 0) break;
    it.assign(2 * n + 2, 0);
    for (i64 f; (f = dfs(s, t, INF)) > 0;) flow += f;
  }
  vector<int> wall;
  for (int v = 2; v < n; ++v) if (lv[v] >= 0 && lv[v + n] < 0) wall.push_back(v);
  printf("%lld\n%d\n", flow, (int)wall.size());
  for (size_t i = 0; i < wall.size(); ++i) printf("%d%c", wall[i], i + 1 == wall.size() ? '\n' : ' ');
  if (wall.empty()) printf("\n");
}
