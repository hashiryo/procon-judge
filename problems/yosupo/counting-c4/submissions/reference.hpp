#pragma once
// 各辺を含む C4 の個数。多重辺は同じ両端の辺をまとめて重み (本数) を持たせ、
// 単純グラフにしてから数える。元の辺と圧縮した辺は 1 対 1 で対応するので、
// 圧縮した辺の答えを元の辺に配ればよい。
//
// 数え方:
//   頂点を次数の小さい順に並べ、各 C4 を「4 頂点のうち最も番号が大きい頂点 v」の
//   回で 1 回だけ数える。C4 を v-w-x-y-v とすると、v から出る 2 歩の道は
//   v->w->x と v->y->x の 2 本なので、2 歩の道 v->w->x ごとに、
//   「w 以外を通る v->x の 2 歩の道」の重みを掛けたものを辺 (v,w) と (w,x) に足す。
//   2 歩の道 v->w->x は番号が v より小さい頂点だけで作るので、
//   各回の仕事は Σ min(次数) で抑えられ、全体で O(N + M sqrt(M))。
#include <algorithm>
#include <numeric>
#include <vector>

struct Solver {
  struct Adj {
    int to;
    long long w;
    int eid;
  };

  int n, m;
  std::vector<int> ca, cb;      // 圧縮した辺の両端 (小さい方, 大きい方)
  std::vector<long long> cw;    // 圧縮した辺の重み (同じ両端の辺の本数)
  std::vector<int> edge_class;  // 元の辺 -> 圧縮した辺の番号
  std::vector<long long> ans;

  Solver(int n_, std::vector<int> a, std::vector<int> b) : n(n_), m((int)a.size()) {
    std::vector<int> lo(m), hi(m), ord(m);
    for (int i = 0; i < m; ++i) {
      lo[i] = std::min(a[i], b[i]);
      hi[i] = std::max(a[i], b[i]);
      ord[i] = i;
    }
    std::sort(ord.begin(), ord.end(), [&](int x, int y) {
      if (lo[x] != lo[y]) return lo[x] < lo[y];
      return hi[x] < hi[y];
    });
    edge_class.assign(m, 0);
    for (int i = 0; i < m; ++i) {
      int e = ord[i];
      if (i == 0 || lo[e] != ca.back() || hi[e] != cb.back()) {
        ca.push_back(lo[e]);
        cb.push_back(hi[e]);
        cw.push_back(0);
      }
      ++cw.back();
      edge_class[e] = (int)ca.size() - 1;
    }
  }

  void run() {
    const int k = (int)ca.size();
    std::vector<int> deg(n, 0);
    for (int i = 0; i < k; ++i) {
      ++deg[ca[i]];
      ++deg[cb[i]];
    }
    std::vector<int> ord(n);
    std::iota(ord.begin(), ord.end(), 0);
    std::sort(ord.begin(), ord.end(), [&](int x, int y) {
      if (deg[x] != deg[y]) return deg[x] < deg[y];
      return x < y;
    });
    std::vector<int> rank(n);
    for (int i = 0; i < n; ++i) rank[ord[i]] = i;

    std::vector<std::vector<Adj>> adj(n);
    for (int i = 0; i < k; ++i) {
      int x = rank[ca[i]], y = rank[cb[i]];
      adj[x].push_back(Adj{y, cw[i], i});
      adj[y].push_back(Adj{x, cw[i], i});
    }
    for (int v = 0; v < n; ++v)
      std::sort(adj[v].begin(), adj[v].end(),
                [](const Adj &p, const Adj &q) { return p.to < q.to; });

    std::vector<long long> c(n, 0), res(k, 0);
    for (int v = n - 1; v >= 0; --v) {
      const auto &av = adj[v];
      int cnt = 0;
      while (cnt < (int)av.size() && av[cnt].to < v) ++cnt;
      // v から 2 歩で行ける頂点 x への道の重みを足す。
      for (int i = 0; i < cnt; ++i) {
        const auto &aw = adj[av[i].to];
        long long w = av[i].w;
        for (int j = 0; j < (int)aw.size() && aw[j].to < v; ++j)
          c[aw[j].to] += w * aw[j].w;
      }
      // 2 歩の道 v->w->x ごとに、w を通らない分を辺 (v,w) と (w,x) に足す。
      for (int i = 0; i < cnt; ++i) {
        const auto &aw = adj[av[i].to];
        int e = av[i].eid;
        long long w = av[i].w;
        for (int j = 0; j < (int)aw.size() && aw[j].to < v; ++j) {
          long long val = c[aw[j].to] - w * aw[j].w;
          res[e] += val * aw[j].w;
          res[aw[j].eid] += val * w;
        }
      }
      // c を戻す。
      for (int i = 0; i < cnt; ++i) {
        const auto &aw = adj[av[i].to];
        for (int j = 0; j < (int)aw.size() && aw[j].to < v; ++j) c[aw[j].to] = 0;
      }
    }

    ans.assign(m, 0);
    for (int e = 0; e < m; ++e) ans[e] = res[edge_class[e]];
  }

  const std::vector<long long> &answers() const { return ans; }
};
