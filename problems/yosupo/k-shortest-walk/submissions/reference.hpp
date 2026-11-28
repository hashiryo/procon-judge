#pragma once
// k 番目までの s-t ウォーク (同じ頂点を何度通ってもよい)。Eppstein の方法。
//
// 1. t から逆辺で Dijkstra して d[v] = dist(v, t) を出す。d[s] が無限なら全部 -1。
// 2. t へ向かう最短路の木を作る。v の木の辺は d[v] = w + d[head] を満たす 1 本。
// 3. 木の辺でない辺 e = (u -> v) を「横道」と呼び、δ(e) = w + d[v] - d[u] >= 0 を持たせる。
// 4. 各頂点 v について、v から t への木の道の上の横道を全部入れた永続左ist ヒープ H_T(v) を作る。
//    H_T(v) = meld(v から出る横道のヒープ, H_T(parent(v))) を木の順 (t から外へ) に。
//    ヒープの節は横道の辺番号を持ち、続きのヒープは綴じるときに H_T(head) を引く。
// 5. 列挙: 最初の答えは d[s]。優先度付きキューに (d[s] + δ(H_T(s) の根), 根) を入れ、
//    取り出した (cost, h) を答えに足し、左の子・右の子・続きのヒープの根を push する。
//    子や続きに移るときは cost - δ(h) + δ(移り先)。
#include <algorithm>
#include <queue>
#include <vector>

struct Solver {
  int n, s, t, k, m;
  std::vector<int> ea, eb;
  std::vector<long long> ec;
  std::vector<long long> ans;

  struct HN {
    long long d;
    int l, r, e, rk;
  };
  std::vector<HN> hp;  // 0 は空

  Solver(int n_, int s_, int t_, int k_, std::vector<int> a, std::vector<int> b,
         std::vector<long long> c)
      : n(n_), s(s_), t(t_), k(k_), m((int)a.size()), ea(std::move(a)), eb(std::move(b)),
        ec(std::move(c)) {}

  int meld(int x, int y) {
    if (!x) return y;
    if (!y) return x;
    if (hp[x].d > hp[y].d) std::swap(x, y);
    HN nd = hp[x];
    nd.r = meld(hp[x].r, y);
    const int rl = nd.l ? hp[nd.l].rk : 0, rr = nd.r ? hp[nd.r].rk : 0;
    if (rl < rr) std::swap(nd.l, nd.r);
    nd.rk = (nd.r ? hp[nd.r].rk : 0) + 1;
    hp.push_back(nd);
    return (int)hp.size() - 1;
  }

  void run() {
    ans.clear();
    const long long INF = (long long)1 << 62;
    // 逆辺で t から Dijkstra
    std::vector<int> rhead(n, -1), rnxt(m), rto(m);
    std::vector<long long> rw(m);
    for (int i = 0; i < m; ++i) {
      rto[i] = ea[i], rw[i] = ec[i], rnxt[i] = rhead[eb[i]], rhead[eb[i]] = i;
    }
    std::vector<long long> d(n, INF);
    std::vector<int> te(n, -1);  // v の木の辺 (Dijkstra が緩めた辺)
    {
      using P = std::pair<long long, int>;
      std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
      d[t] = 0;
      pq.emplace(0, t);
      while (!pq.empty()) {
        auto [dv, v] = pq.top();
        pq.pop();
        if (dv != d[v]) continue;
        for (int e = rhead[v]; e != -1; e = rnxt[e]) {
          const int u = rto[e];
          if (d[v] + rw[e] < d[u]) d[u] = d[v] + rw[e], te[u] = e, pq.emplace(d[u], u);
        }
      }
    }
    if (d[s] == INF) {
      ans.assign(k, -1);
      return;
    }
    // 木の順 (t から外へ)
    std::vector<int> thead(n, -1), tnxt(n), tchild(n);
    for (int v = 0; v < n; ++v)
      if (v != t && te[v] != -1) tchild[v] = eb[te[v]], tnxt[v] = thead[tchild[v]], thead[tchild[v]] = v;
    std::vector<int> order;  // 親が先
    order.push_back(t);
    for (size_t i = 0; i < order.size(); ++i)
      for (int c = thead[order[i]]; c != -1; c = tnxt[c]) order.push_back(c);
    // 各頂点から出る横道のヒープ (辺の番号を持つ)
    std::vector<int> ehead(n, -1), enxt(m);
    for (int e = 0; e < m; ++e) {
      if (te[ea[e]] == e) continue;
      if (d[ea[e]] == INF || d[eb[e]] == INF) continue;
      enxt[e] = ehead[ea[e]], ehead[ea[e]] = e;
    }
    hp.assign(1, HN{0, 0, 0, 0, 0});
    hp.reserve((size_t)m * 20 + 16);
    std::vector<int> root(n, 0);
    for (int v : order) {
      int cur = (v == t) ? 0 : root[tchild[v]];
      // v から出る横道をすべて入れる (δ の小さい順に積む)
      std::vector<int> es;
      for (int e = ehead[v]; e != -1; e = enxt[e]) es.push_back(e);
      std::sort(es.begin(), es.end(),
                [&](int x, int y) { return ec[x] + d[eb[x]] - d[ea[x]] < ec[y] + d[eb[y]] - d[ea[y]]; });
      for (int e : es) {
        hn_new(ec[e] + d[eb[e]] - d[ea[e]], e);
        cur = meld(cur, (int)hp.size() - 1);
      }
      root[v] = cur;
    }
    // 列挙
    using Q = std::pair<long long, int>;
    std::priority_queue<Q, std::vector<Q>, std::greater<Q>> pq;
    ans.push_back(d[s]);
    if (root[s]) pq.emplace(d[s] + hp[root[s]].d, root[s]);
    while ((int)ans.size() < k) {
      if (pq.empty()) {
        ans.push_back(-1);
        continue;
      }
      auto [c, h] = pq.top();
      pq.pop();
      ans.push_back(c);
      const long long dh = hp[h].d;
      if (hp[h].l) pq.emplace(c - dh + hp[hp[h].l].d, hp[h].l);
      if (hp[h].r) pq.emplace(c - dh + hp[hp[h].r].d, hp[h].r);
      const int cont = root[eb[hp[h].e]];
      if (cont) pq.emplace(c + hp[cont].d, cont);  // 次の位置の横道は足す
    }
  }

  void hn_new(long long dd, int e) { hp.push_back(HN{dd, 0, 0, e, 1}); }

  const std::vector<long long> &answer() const { return ans; }
};
