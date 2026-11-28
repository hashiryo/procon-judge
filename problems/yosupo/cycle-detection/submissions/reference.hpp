#pragma once
// 有向グラフの閉路を 1 つ見つける。深さ優先で、いま道の上にある頂点へ戻る辺を見つけたら、
// その頂点から今の頂点までの木の辺を逆順に並べ、最後にその辺を足す。
#include <vector>

struct Solver {
  int n;
  std::vector<int> head, nxt, to;
  std::vector<int> cyc;

  Solver(int n_, const std::vector<int> &u, const std::vector<int> &v) : n(n_) {
    const int m = (int)u.size();
    head.assign(n, -1);
    nxt.resize(m);
    to.resize(m);
    for (int i = 0; i < m; ++i) to[i] = v[i], nxt[i] = head[u[i]], head[u[i]] = i;
  }

  void run() {
    cyc.clear();
    const int m = (int)to.size();
    std::vector<signed char> col(n, 0);  // 0 未訪問、1 道の上、2 済み
    std::vector<int> pe(n, -1);          // 木の親の辺
    std::vector<int> vs, es, it;         // DFS の道 (頂点、その頂点に入る辺、次に見る辺)
    vs.reserve(n);
    es.reserve(n);
    it.reserve(n);
    for (int s = 0; s < n; ++s) {
      if (col[s]) continue;
      col[s] = 1;
      vs.push_back(s), es.push_back(-1), it.push_back(head[s]);
      while (!vs.empty()) {
        const int x = vs.back();
        int &e = it.back();
        if (e == -1) {
          col[x] = 2;
          vs.pop_back(), es.pop_back(), it.pop_back();
          continue;
        }
        const int ei = e;
        e = nxt[e];
        const int y = to[ei];
        if (col[y] == 2) continue;
        if (col[y] == 1) {
          // y から x までの道の辺を逆順に並べ、最後に ei を足す。
          cyc.clear();
          for (int t = (int)vs.size() - 1; t >= 0 && vs[t] != y; --t) cyc.push_back(es[t]);
          for (int i = 0, j = (int)cyc.size() - 1; i < j; ++i, --j) std::swap(cyc[i], cyc[j]);
          cyc.push_back(ei);
          return;
        }
        pe[y] = ei;
        col[y] = 1;
        vs.push_back(y), es.push_back(ei), it.push_back(head[y]);
      }
    }
  }

  const std::vector<int> &cycle() const { return cyc; }
};
