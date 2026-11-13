#pragma once
// 期待出力を作る参照実装。教科書どおりの逐次最短路で、速さより確かさを取った形。ライブラリも _shared も include しない
// (期待出力のキャッシュの鍵はこのファイルの中身だけで決まるため)。
//
// slope を呼ぶたびに、まず前の呼び出しのポテンシャルから始めて、残余グラフのすべての辺で Bellman-Ford の緩和 (キューを
// 使う形) を回し、縮約費用 c + p_u - p_v を 0 以上にする (初めの負の費用と、あとから足した辺のため)。新しい頂点の初めの
// 値は、そのときのポテンシャルの最大にする。そのあとは、Dijkstra (std::priority_queue) で s からの縮約費用の最短路を
// 最後まで求め、すべての頂点のポテンシャルに min(距離, t の距離) を足し、最短路に沿って流せるだけ流すことを繰り返す。
// 道を 1 本流すごとに点を 1 つ足し、傾きの同じ点はまとめない (ハーネスがまとめる)。
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

struct Solver {
  struct Arc {
    int to, rev;
    i64 cap, cost;
  };
  int n;
  vector<vector<Arc>> g;
  vector<pair<int, int>> pos;  // 辺 i の弧の場所 (頂点、添字)
  vector<i64> p;               // ポテンシャル
  int known = 0;               // p を持っている頂点の数

  explicit Solver(int n) : n(n), g(n) {}
  int add_vertex() { return g.emplace_back(), n++; }
  int add_edge(int from, int to, i64 cap, i64 cost) {
    pos.push_back({from, (int)g[from].size()});
    g[from].push_back({to, (int)g[to].size() + (from == to), cap, cost});
    g[to].push_back({from, (int)g[from].size() - 1, 0, -cost});
    return (int)pos.size() - 1;
  }
  i64 flow(int i) const {
    const Arc &a = g[pos[i].first][pos[i].second];
    return g[a.to][a.rev].cap;
  }

  void repair() {
    i64 mx = 0;
    for (int v = 0; v < known; ++v) mx = max(mx, p[v]);
    p.resize(n, mx);
    known = n;
    deque<int> q;
    vector<char> inq(n, 1);
    vector<int> cnt(n, 0);
    for (int v = 0; v < n; ++v) q.push_back(v);
    while (!q.empty()) {
      const int u = q.front();
      q.pop_front();
      inq[u] = 0;
      for (const Arc &a : g[u])
        if (a.cap > 0 && p[u] + a.cost < p[a.to]) {
          p[a.to] = p[u] + a.cost;
          if (!inq[a.to]) {
            if (++cnt[a.to] > n) {
              fprintf(stderr, "reference: 残余グラフに負閉路がある\n");
              exit(1);
            }
            inq[a.to] = 1, q.push_back(a.to);
          }
        }
    }
  }

  vector<pair<i64, i64>> slope(int s, int t, i64 limit) {
    repair();
    vector<pair<i64, i64>> res = {{0, 0}};
    if (s == t) return res;
    constexpr i64 INF = numeric_limits<i64>::max();
    vector<i64> dist(n);
    vector<pair<int, int>> prev(n);
    i64 fl = 0, cost = 0;
    while (fl < limit) {
      fill(dist.begin(), dist.end(), INF);
      using P = pair<i64, int>;
      priority_queue<P, vector<P>, greater<P>> pq;
      dist[s] = 0, pq.push({0, s});
      while (!pq.empty()) {
        const auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;
        for (int k = 0; k < (int)g[u].size(); ++k) {
          const Arc &a = g[u][k];
          if (a.cap <= 0) continue;
          const i64 rc = a.cost + p[u] - p[a.to];
          if (rc < 0) {
            fprintf(stderr, "reference: 縮約費用が負\n");
            exit(1);
          }
          if (d + rc < dist[a.to]) dist[a.to] = d + rc, prev[a.to] = {u, k}, pq.push({d + rc, a.to});
        }
      }
      if (dist[t] == INF) break;
      for (int v = 0; v < n; ++v) p[v] += min(dist[v], dist[t]);
      i64 c = limit - fl, pc = 0;
      for (int v = t; v != s; v = prev[v].first) {
        const Arc &a = g[prev[v].first][prev[v].second];
        c = min(c, a.cap), pc += a.cost;
      }
      for (int v = t; v != s; v = prev[v].first) {
        Arc &a = g[prev[v].first][prev[v].second];
        a.cap -= c, g[v][a.rev].cap += c;
      }
      fl += c, cost += c * pc;
      res.push_back({fl, cost});
    }
    return res;
  }
};
