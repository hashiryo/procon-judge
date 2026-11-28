#pragma once
// 木の直径。頂点 0 から最も遠い頂点 u を求め、u から最も遠い頂点 v までを直径とする。
// 2 回の幅優先で、親をたどって経路を復元する。
#include <vector>

struct Solver {
  int n;
  std::vector<int> head, nxt, to;
  std::vector<long long> w;
  long long best = 0;
  std::vector<int> pathv;

  Solver(int n_, const std::vector<int> &a, const std::vector<int> &b,
         const std::vector<long long> &c)
      : n(n_) {
    const int m = n_ - 1;
    head.assign(n, -1);
    nxt.resize(2 * m);
    to.resize(2 * m);
    w.resize(2 * m);
    for (int i = 0; i < m; ++i) {
      to[2 * i] = b[i], w[2 * i] = c[i], nxt[2 * i] = head[a[i]], head[a[i]] = 2 * i;
      to[2 * i + 1] = a[i], w[2 * i + 1] = c[i], nxt[2 * i + 1] = head[b[i]];
      head[b[i]] = 2 * i + 1;
    }
  }

  // s から最も遠い頂点と、その距離。parent をたどれるように par も返す。
  int bfs(int s, std::vector<long long> &dist, std::vector<int> &par) const {
    dist.assign(n, -1);
    par.assign(n, -1);
    std::vector<int> q;
    q.reserve(n);
    q.push_back(s);
    dist[s] = 0;
    int far = s;
    for (size_t qi = 0; qi < q.size(); ++qi) {
      const int v = q[qi];
      if (dist[v] > dist[far]) far = v;
      for (int e = head[v]; e != -1; e = nxt[e]) {
        const int u = to[e];
        if (dist[u] >= 0) continue;
        dist[u] = dist[v] + w[e];
        par[u] = v;
        q.push_back(u);
      }
    }
    return far;
  }

  void run() {
    std::vector<long long> d;
    std::vector<int> p;
    const int u = bfs(0, d, p);
    const int v = bfs(u, d, p);
    best = d[v];
    pathv.clear();
    for (int x = v; x != -1; x = p[x]) pathv.push_back(x);
    for (int i = 0, j = (int)pathv.size() - 1; i < j; ++i, --j) std::swap(pathv[i], pathv[j]);
  }

  long long weight() const { return best; }
  const std::vector<int> &path() const { return pathv; }
};
