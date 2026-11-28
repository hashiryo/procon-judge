#pragma once
// 非負の重みの有向最短路。Dijkstra を二分ヒープで回し、各頂点に入ってきた辺を覚えて経路を復元する。
#include <queue>
#include <vector>

struct Solver {
  int n, s, t;
  std::vector<int> head, nxt, to, ea;
  std::vector<long long> w;
  bool reached = false;
  long long dist = 0;
  std::vector<int> pathv;

  Solver(int n_, int s_, int t_, const std::vector<int> &a, const std::vector<int> &b,
         const std::vector<long long> &c)
      : n(n_), s(s_), t(t_), ea(a) {
    const int m = (int)a.size();
    head.assign(n, -1);
    nxt.resize(m);
    to.resize(m);
    w.resize(m);
    for (int i = 0; i < m; ++i) {
      to[i] = b[i], w[i] = c[i], nxt[i] = head[a[i]], head[a[i]] = i;
    }
  }

  void run() {
    const long long INF = (long long)1 << 62;
    std::vector<long long> d(n, INF);
    std::vector<int> pe(n, -1);
    using P = std::pair<long long, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    d[s] = 0;
    pq.emplace(0, s);
    while (!pq.empty()) {
      auto [dv, v] = pq.top();
      pq.pop();
      if (dv != d[v]) continue;
      for (int e = head[v]; e != -1; e = nxt[e]) {
        const int u = to[e];
        if (d[v] + w[e] < d[u]) {
          d[u] = d[v] + w[e];
          pe[u] = e;
          pq.emplace(d[u], u);
        }
      }
    }
    pathv.clear();
    reached = d[t] != INF;
    if (!reached) return;
    dist = d[t];
    for (int v = t; v != s; v = ea[pe[v]]) pathv.push_back(v);
    pathv.push_back(s);
    for (int i = 0, j = (int)pathv.size() - 1; i < j; ++i, --j) std::swap(pathv[i], pathv[j]);
  }

  bool ok() const { return reached; }
  long long weight() const { return dist; }
  const std::vector<int> &path() const { return pathv; }
};
