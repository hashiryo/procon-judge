#pragma once
// 補グラフの連結成分。未訪問の頂点を双方向リストに持ち、幅優先の各頂点で隣を印してから
// リストを 1 回なめ、印の付いていない頂点を取る。取り出す回数は N 回、飛ばす回数は
// 辺の本数の 2 倍までなので、合わせて O(N + M)。
#include <vector>

struct Solver {
  int n;
  std::vector<int> head, nxt, to;
  std::vector<std::vector<int>> comps;

  Solver(int n_, const std::vector<int> &a, const std::vector<int> &b) : n(n_) {
    const int m = (int)a.size();
    head.assign(n, -1);
    nxt.resize(2 * m);
    to.resize(2 * m);
    for (int i = 0; i < m; ++i) {
      to[2 * i] = b[i], nxt[2 * i] = head[a[i]], head[a[i]] = 2 * i;
      to[2 * i + 1] = a[i], nxt[2 * i + 1] = head[b[i]], head[b[i]] = 2 * i + 1;
    }
  }

  void run() {
    comps.clear();
    std::vector<int> nx(n), pv(n);
    for (int i = 0; i < n; ++i) nx[i] = i + 1 < n ? i + 1 : -1, pv[i] = i - 1;
    int first = n ? 0 : -1;
    std::vector<int> mark(n, -1), q;
    int stamp = 0;
    while (first != -1) {
      const int s = first;
      // リストから外す
      first = nx[s];
      if (first != -1) pv[first] = -1;
      comps.emplace_back();
      auto &c = comps.back();
      c.push_back(s);
      q.assign(1, s);
      for (size_t qi = 0; qi < q.size(); ++qi) {
        const int u = q[qi];
        ++stamp;  // u ごとに印を付け直す
        for (int e = head[u]; e != -1; e = nxt[e]) mark[to[e]] = stamp;
        for (int v = first; v != -1;) {
          const int nv = nx[v];
          if (mark[v] != stamp) {
            c.push_back(v);
            q.push_back(v);
            if (pv[v] != -1) nx[pv[v]] = nv;
            else first = nv;
            if (nv != -1) pv[nv] = pv[v];
          }
          v = nv;
        }
      }
    }
  }

  const std::vector<std::vector<int>> &components() const { return comps; }
};
