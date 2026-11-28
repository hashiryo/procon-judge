#pragma once
// 有向グラフのオイラー路。出次数と入次数の差が 1 の頂点が高々 1 つずつで、辺のある頂点が
// 1 つの弱連結成分に入っていることが条件。構成は Hierholzer を行きがけの深さ優先で回す。
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
    std::vector<int> out(n, 0), in(n, 0), head(n, -1), nxt(m);
    for (int i = 0; i < m; ++i) {
      ++out[uu[i]], ++in[vv[i]];
      nxt[i] = head[uu[i]], head[uu[i]] = i;
    }
    int s = -1, e = -1;
    for (int v = 0; v < n; ++v) {
      const int d = out[v] - in[v];
      if (d == 1) {
        if (s != -1) return;
        s = v;
      } else if (d == -1) {
        if (e != -1) return;
        e = v;
      } else if (d != 0) {
        return;
      }
    }
    if ((s == -1) != (e == -1)) return;
    if (s == -1) {
      for (int v = 0; v < n; ++v)
        if (out[v]) {
          s = v;
          break;
        }
    }
    // 弱連結性: 辺のある頂点がすべて s から (向きを無視して) 届くこと。
    {
      std::vector<int> h2(n, -1), n2(2 * m), t2(2 * m);
      for (int i = 0; i < m; ++i) {
        t2[2 * i] = vv[i], n2[2 * i] = h2[uu[i]], h2[uu[i]] = 2 * i;
        t2[2 * i + 1] = uu[i], n2[2 * i + 1] = h2[vv[i]], h2[vv[i]] = 2 * i + 1;
      }
      std::vector<signed char> seen(n, 0);
      std::vector<int> q{s};
      seen[s] = 1;
      for (size_t qi = 0; qi < q.size(); ++qi)
        for (int x = h2[q[qi]]; x != -1; x = n2[x])
          if (!seen[t2[x]]) seen[t2[x]] = 1, q.push_back(t2[x]);
      for (int v = 0; v < n; ++v)
        if ((out[v] || in[v]) && !seen[v]) return;
    }
    std::vector<int> ptr = head, st;
    std::vector<std::pair<int, int>> stk;  // (頂点, 入ってきた辺)
    stk.reserve(m + 1);
    stk.emplace_back(s, -1);
    std::vector<int> ov, oe;
    ov.reserve(m + 1), oe.reserve(m);
    while (!stk.empty()) {
      const int x = stk.back().first;
      int &p = ptr[x];
      if (p != -1) {
        const int ei = p;
        p = nxt[ei];
        stk.emplace_back(vv[ei], ei);
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
