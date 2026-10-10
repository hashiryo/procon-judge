#pragma once
// 期待出力を作る参照実装。今の Library の NetworkSimplex (mylib/optimization/NetworkSimplex.hpp) を、最小化だけにして
// 写したもの。ライブラリも _shared も include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まるため)。
// 期待出力で使うのは費用の和だけで、最適性はハーネスが流量とポテンシャルで確かめる。
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

struct Solver {
  struct Node {
    int par, pred;
    i64 sup, pi;
  };
  struct Edge {
    int u, v;
    i64 low, up, flow, cost;
    int8_t state = 1;
  };
  int n, m = 0;
  vector<Node> ns;
  vector<Edge> es;
  vector<int> bfs, nxt, prv;
  bool ok = false;
  __int128 total = 0;
  vector<i64> pot, fl;

  Solver(int n, const vector<i64> &b, const vector<array<i64, 5>> &edges) : n(n), ns(n + 1) {
    for (int i = 0; i < n; ++i) ns[i].sup = b[i];
    es.reserve(edges.size() + n);
    for (auto &e : edges) es.push_back({(int)e[0], (int)e[1], e[2], e[3], 0, e[4]}), ++m;
  }
  void link(int u, int v) { nxt[u] = v, prv[v] = u; }
  void link(int u, int v, int w) { link(u, v), link(v, w); }
  i64 opp_cost(int e) const { return es[e].cost + ns[es[e].u].pi - ns[es[e].v].pi; }
  void pivot(int in_arc) {
    int u_in = es[in_arc].u, v_in = es[in_arc].v, u, e, a = u_in, b = v_in;
    while (a != b) a = ns[a].par == -1 ? v_in : ns[a].par, b = ns[b].par == -1 ? u_in : ns[b].par;
    if (es[in_arc].state == -1) swap(u_in, v_in);
    int lca = a, side = 0, u_out = -1, i = 0, S = 0;
    i64 delta = es[in_arc].up;
    for (u = u_in; u != lca && delta > 0; u = ns[u].par) {
      e = ns[u].pred;
      i64 d = u == es[e].v ? es[e].up - es[e].flow : es[e].flow;
      if (delta > d) delta = d, u_out = u, side = 1;
    }
    for (u = v_in; u != lca; u = ns[u].par) {
      e = ns[u].pred;
      i64 d = u == es[e].u ? es[e].up - es[e].flow : es[e].flow;
      if (delta >= d) delta = d, u_out = u, side = -1;
    }
    if (delta > 0) {
      es[in_arc].flow += delta *= es[in_arc].state;
      for (u = es[in_arc].u; u != lca; u = ns[u].par) e = ns[u].pred, es[e].flow += u == es[e].u ? -delta : delta;
      for (u = es[in_arc].v; u != lca; u = ns[u].par) e = ns[u].pred, es[e].flow += u == es[e].u ? delta : -delta;
    }
    if (side == 0) {
      es[in_arc].state *= -1;
      return;
    }
    int out_arc = ns[u_out].pred, p;
    es[in_arc].state = 0, es[out_arc].state = es[out_arc].flow ? -1 : 1;
    if (side == -1) swap(u_in, v_in);
    for (u = u_in; u != u_out; u = ns[u].par) bfs[S++] = u;
    // 元の Library の 1 行 (ns[p= ns[u].par].par= u= bfs[i], ...) は、C++17 で右辺の u= bfs[i] が先に評価される。
    for (i = S; i--;) {
      u = bfs[i];
      p = ns[u].par;
      ns[p].par = u, ns[p].pred = ns[u].pred, link(prv[p], nxt[p]), link(prv[u + n + 1], p, u + n + 1);
    }
    link(prv[u_in], nxt[u_in]), link(prv[v_in + n + 1], u_in, v_in + n + 1);
    ns[u_in].par = v_in, ns[u_in].pred = in_arc;
    const i64 pi_delta = u_in == es[in_arc].u ? -opp_cost(in_arc) : opp_cost(in_arc);
    for (i = 0, S = 1, bfs[0] = u_in; i < S; i++) {
      ns[u = bfs[i]].pi += pi_delta;
      for (int v = nxt[u + n + 1]; v != u + n + 1; v = nxt[v]) bfs[S++] = v;
    }
  }
  void calc() {
    i64 inf_cost = 1;
    for (int e = 0; e < m; e++) {
      es[e].flow = 0, es[e].state = 1, es[e].up -= es[e].low, ns[es[e].u].sup -= es[e].low, ns[es[e].v].sup += es[e].low;
      inf_cost += es[e].cost < 0 ? -es[e].cost : es[e].cost;
    }
    ns[n] = {-1, -1, 0, 0}, es.resize(m + n), bfs.resize(n + 1);
    nxt.resize(2 * n + 2), iota(nxt.begin() + n + 1, nxt.end(), n + 1);
    prv.resize(2 * n + 2), iota(prv.begin() + n + 1, prv.end(), n + 1);
    for (int u = 0, e = m; u < n; u++, e++) {
      ns[u].par = n, ns[u].pred = e, link(prv[n + n + 1], u, n + n + 1);
      if (const i64 supply = ns[u].sup; supply >= 0) {
        ns[u].pi = -inf_cost;
        es[e] = {u, n, 0, supply, supply, inf_cost, 0};
      } else {
        ns[u].pi = inf_cost;
        es[e] = {n, u, 0, -supply, -supply, inf_cost, 0};
      }
    }
    const int block_size = max(int(ceil(sqrt(m + n))), min(10, n + 1));
    for (int e = 0, in_arc, cnt, seen;; pivot(in_arc)) {
      i64 minimum = 0;
      for (in_arc = -1, cnt = block_size, seen = m + n; seen--; e = e + 1 == m + n ? 0 : e + 1) {
        if (minimum > es[e].state * opp_cost(e)) minimum = es[e].state * opp_cost(e), in_arc = e;
        if (--cnt == 0 && minimum < 0) break;
        if (cnt == 0) cnt = block_size;
      }
      if (in_arc == -1) break;
    }
    for (int e = 0; e < m; e++) es[e].flow += es[e].low, es[e].up += es[e].low, ns[es[e].u].sup += es[e].low, ns[es[e].v].sup -= es[e].low;
  }
  void run() {
    i64 sum = 0;
    for (int u = 0; u < n; u++) sum += ns[u].sup;
    if (sum != 0) return;
    calc();
    for (int e = m; e < m + n; e++)
      if (es[e].flow != 0) return;
    ok = true;
    pot.resize(n), fl.resize(m);
    for (int i = 0; i < n; ++i) pot[i] = ns[i].pi;
    for (int e = 0; e < m; ++e) fl[e] = es[e].flow, total += (__int128)es[e].flow * es[e].cost;
  }
  bool feasible() const { return ok; }
  __int128 cost() const { return total; }
  const vector<i64> &potential() const { return pot; }
  const vector<i64> &flow() const { return fl; }
};
