#pragma once
// 無向グラフのオイラー路。次数が奇数の頂点が 0 個か 2 個で、辺のある頂点が 1 つの連結成分に
// 入っていることが条件。構成は Hierholzer を行きがけの深さ優先で回し、通った辺を使い切る。
#include <vector>

struct Solver {
  int n;
  std::vector<int> uu, vv;
  bool good = false;
  std::vector<int> vs, es;

  Solver(int n_, std::vector<int> u, std::vector<int> v)
      : n(n_), uu(std::move(u)), vv(std::move(v)) {}

  void run() {
    const int m = (int)uu.size();
    vs.clear(), es.clear(), good = false;
    if (m == 0) {
      good = true;
      vs = {0};
      return;
    }
    std::vector<int> deg(n, 0), head(n, -1), nxt(2 * m), to(2 * m), eid(2 * m);
    for (int i = 0; i < m; ++i) {
      ++deg[uu[i]], ++deg[vv[i]];
      to[2 * i] = vv[i], eid[2 * i] = i, nxt[2 * i] = head[uu[i]], head[uu[i]] = 2 * i;
      to[2 * i + 1] = uu[i], eid[2 * i + 1] = i, nxt[2 * i + 1] = head[vv[i]];
      head[vv[i]] = 2 * i + 1;
    }
    int s = -1, odd = 0;
    for (int v = 0; v < n; ++v) {
      if (deg[v] & 1) ++odd, s = v;
      if (s == -1 && deg[v]) s = v;
    }
    if (odd == 0) {
      for (int v = 0; v < n; ++v)
        if (deg[v]) {
          s = v;
          break;
        }
    } else if (odd != 2) {
      return;
    }
    std::vector<signed char> used(m, 0), seen(n, 0);
    std::vector<int> q{s};
    seen[s] = 1;
    for (size_t qi = 0; qi < q.size(); ++qi)
      for (int e = head[q[qi]]; e != -1; e = nxt[e])
        if (!seen[to[e]]) seen[to[e]] = 1, q.push_back(to[e]);
    for (int v = 0; v < n; ++v)
      if (deg[v] && !seen[v]) return;
    std::vector<int> ptr = head;
    std::vector<std::pair<int, int>> stk;
    stk.reserve(m + 1);
    stk.emplace_back(s, -1);
    std::vector<int> ov, oe;
    ov.reserve(m + 1), oe.reserve(m);
    while (!stk.empty()) {
      const int x = stk.back().first;
      int &p = ptr[x];
      while (p != -1 && used[eid[p]]) p = nxt[p];
      if (p != -1) {
        const int e = p;
        p = nxt[e];
        used[eid[e]] = 1;
        stk.emplace_back(to[e], eid[e]);
      } else {
        ov.push_back(x);
        if (stk.back().second != -1) oe.push_back(stk.back().second);
        stk.pop_back();
      }
    }
    if ((int)ov.size() != m + 1) return;
    vs.assign(ov.rbegin(), ov.rend());
    es.assign(oe.rbegin(), oe.rend());
    good = true;
  }

  bool ok() const { return good; }
  const std::vector<int> &vertex() const { return vs; }
  const std::vector<int> &edge() const { return es; }
};
