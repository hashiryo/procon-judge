#pragma once
// 愚直解。下限を先に流し、過不足を超頂点の間の流れにして Edmonds-Karp で実行可能な流れを作り、そのあと残余グラフの
// 負閉路を Bellman-Ford で見つけて消すことを、負閉路が無くなるまで繰り返す (Klein の方法)。最後の Bellman-Ford の距離を
// ポテンシャルにする。参照実装 (ネットワーク単体法) と考え方が違う。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

struct Solver {
  struct E {
    int to, rev;
    i64 cap, cost;
  };
  int n;
  const vector<i64> &b;
  const vector<array<i64, 5>> &edges;
  bool ok = false;
  __int128 total = 0;
  vector<i64> pot, fl;

  Solver(int n, const vector<i64> &b, const vector<array<i64, 5>> &edges) : n(n), b(b), edges(edges) {}
  void run() {
    const int m = edges.size(), S = n, T = n + 1, N = n + 2;
    vector<vector<E>> g(N);
    vector<pair<int, int>> pos(m, {-1, -1});
    auto add = [&](int u, int v, i64 cap, i64 cost) {
      g[u].push_back({v, (int)g[v].size(), cap, cost});
      g[v].push_back({u, (int)g[u].size() - 1, 0, -cost});
      return pair<int, int>{u, (int)g[u].size() - 1};
    };
    vector<i64> ex(b.begin(), b.end());
    for (int i = 0; i < m; ++i) {
      const auto &[s, t, l, u, c] = edges[i];
      ex[s] -= l, ex[t] += l;
      if (s != t) pos[i] = add((int)s, (int)t, u - l, c);
    }
    i64 need = 0;
    for (int v = 0; v < n; ++v) {
      if (ex[v] > 0) add(S, v, ex[v], 0), need += ex[v];
      if (ex[v] < 0) add(v, T, -ex[v], 0);
    }
    // Edmonds-Karp で S から T へ流す。
    i64 got = 0;
    for (;;) {
      vector<int> pv(N, -1), pe(N, -1);
      queue<int> q;
      q.push(S), pv[S] = S;
      while (!q.empty() && pv[T] < 0) {
        const int u = q.front();
        q.pop();
        for (int k = 0; k < (int)g[u].size(); ++k)
          if (g[u][k].cap > 0 && pv[g[u][k].to] < 0) pv[g[u][k].to] = u, pe[g[u][k].to] = k, q.push(g[u][k].to);
      }
      if (pv[T] < 0) break;
      i64 d = numeric_limits<i64>::max();
      for (int v = T; v != S; v = pv[v]) d = min(d, g[pv[v]][pe[v]].cap);
      for (int v = T; v != S; v = pv[v]) {
        E &e = g[pv[v]][pe[v]];
        e.cap -= d, g[v][e.rev].cap += d;
      }
      got += d;
    }
    if (got != need) return;
    // 超頂点の辺を除いた残余グラフで、負閉路を消す。自己ループは費用が負なら上限まで流す。
    auto inner = [&](int u, const E &e) { return u < n && e.to < n; };
    vector<i64> dist(n);
    vector<int> pv(n), pe(n);
    for (;;) {
      fill(dist.begin(), dist.end(), 0), fill(pv.begin(), pv.end(), -1);
      int last = -1;
      for (int it = 0; it < n; ++it) {
        last = -1;
        for (int u = 0; u < n; ++u)
          for (int k = 0; k < (int)g[u].size(); ++k) {
            const E &e = g[u][k];
            if (e.cap > 0 && inner(u, e) && dist[u] + e.cost < dist[e.to]) dist[e.to] = dist[u] + e.cost, pv[e.to] = u, pe[e.to] = k, last = e.to;
          }
        if (last < 0) break;
      }
      if (last < 0) break;
      int v = last;
      for (int i = 0; i < n; ++i) v = pv[v];  // 閉路の上の頂点に移る
      i64 d = numeric_limits<i64>::max();
      int u = v;
      do d = min(d, g[pv[u]][pe[u]].cap), u = pv[u];
      while (u != v);
      u = v;
      do {
        E &e = g[pv[u]][pe[u]];
        e.cap -= d, g[u][e.rev].cap += d, u = pv[u];
      } while (u != v);
    }
    ok = true;
    pot.assign(dist.begin(), dist.end());
    fl.resize(m);
    for (int i = 0; i < m; ++i) {
      const auto &[s, t, l, u, c] = edges[i];
      if (pos[i].first < 0) fl[i] = c < 0 ? u : l;
      else {
        const E &e = g[pos[i].first][pos[i].second];
        fl[i] = l + g[e.to][e.rev].cap;
      }
      total += (__int128)fl[i] * c;
    }
  }
  bool feasible() const { return ok; }
  __int128 cost() const { return total; }
  const vector<i64> &potential() const { return pot; }
  const vector<i64> &flow() const { return fl; }
};
