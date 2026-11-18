#pragma once
// 参照実装。交換グラフを毎回作り、要素の長さ (I に無い x は -w(x)、I にある y は w(y)) の和と辺の数の組で最小の路を
// Bellman-Ford (待ち行列) で求めて 1 本ずつ増やす。提出が使う _shared/matroid/ を読まず、森と色の扱いもここに書く。
// 交換グラフの辺: y が x の森の閉路 (x の両端を結ぶ木の道) にあれば y → x、x と同じ色の y が上限まで使われていれば x → y。
#include <algorithm>
#include <deque>
#include <limits>
#include <numeric>
#include <vector>

struct Solver {
  int n;
  const vector<array<int, 2>> &e;
  const vector<int> &color, &cap;
  const vector<i64> &w;
  vector<i64> best;
  vector<int> I;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap, const vector<i64> &w)
      : n(n), e(edges), color(color), cap(cap), w(w) {}

  void run() {
    const int m = e.size();
    const i64 INF = numeric_limits<i64>::max() / 4;
    vector<char> in(m);
    best.assign(1, 0);
    while (true) {
      // 森を根付き木にする (BFS)
      vector<vector<pair<int, int>>> g(n);
      for (int y : I) g[e[y][0]].push_back({e[y][1], y}), g[e[y][1]].push_back({e[y][0], y});
      vector<int> comp(n, -1), dep(n), pe(n, -1), pv(n, -1);
      for (int r = 0; r < n; ++r) {
        if (comp[r] >= 0) continue;
        vector<int> q{r};
        comp[r] = r;
        for (size_t h = 0; h < q.size(); ++h)
          for (auto [v, y] : g[q[h]])
            if (comp[v] < 0) comp[v] = r, dep[v] = dep[q[h]] + 1, pe[v] = y, pv[v] = q[h], q.push_back(v);
      }
      vector<int> cnt(cap.size());
      vector<vector<int>> mem(cap.size());
      for (int y : I) ++cnt[color[y]], mem[color[y]].push_back(y);
      vector<vector<int>> adj(m);
      vector<char> src(m), snk(m);
      for (int x = 0; x < m; ++x) {
        if (in[x]) continue;
        int u = e[x][0], v = e[x][1];
        if (comp[u] != comp[v]) src[x] = 1;
        else
          while (u != v) {
            if (dep[u] < dep[v]) swap(u, v);
            adj[pe[u]].push_back(x);
            u = pv[u];
          }
        if (cnt[color[x]] < cap[color[x]]) snk[x] = 1;
        else
          for (int y : mem[color[x]]) adj[x].push_back(y);
      }
      vector<i64> d(m, INF);
      vector<int> hop(m), par(m, -1);
      vector<char> inq(m);
      deque<int> q;
      for (int x = 0; x < m; ++x)
        if (src[x]) d[x] = -w[x], inq[x] = 1, q.push_back(x);
      while (!q.empty()) {
        const int u = q.front();
        q.pop_front();
        inq[u] = 0;
        for (int v : adj[u]) {
          const i64 nd = d[u] + (in[v] ? w[v] : -w[v]);
          if (nd < d[v] || (nd == d[v] && hop[u] + 1 < hop[v])) {
            d[v] = nd, hop[v] = hop[u] + 1, par[v] = u;
            if (!inq[v]) inq[v] = 1, q.push_back(v);
          }
        }
      }
      int t = -1;
      for (int x = 0; x < m; ++x)
        if (snk[x] && d[x] < INF && (t < 0 || d[x] < d[t] || (d[x] == d[t] && hop[x] < hop[t]))) t = x;
      if (t < 0) break;
      for (int v = t; v >= 0; v = par[v]) in[v] ^= 1;
      I.clear();
      for (int x = 0; x < m; ++x)
        if (in[x]) I.push_back(x);
      best.push_back(best.back() - d[t]);
    }
  }

  vector<i64> answer() const { return best; }
  vector<int> chosen() const { return I; }
};
