#pragma once
// 根を 0 に固定した木。頂点 v から根へ向かう辺の合成を (P_v, Q_v) と書くと P(v) = P_v a_v + Q_v。
// 辺 e (親 p、子 c) の関数を (b, c) から (b', c') に変えると、部分木 c の (P_v, Q_v) は一斉に
//   (P_v, Q_v) <- (m P_v, m Q_v + d),  m = b' / b,  d = P_p (c' - m c) + (1 - m) Q_p
// へ変わる (P_p, Q_p は親 p のいまの値)。部分木は Euler tour 上で連続な区間なので、
// 区間への 1 次変換と区間和を持つ遅延セグメント木に載せる。葉は (P_v, Q_v) も覚え、
// a_w の変更は V_w = P_w x + Q_w への 1 点更新にする。1 クエリ O(log N)。
#include <array>
#include <utility>
#include <vector>

#include "pj.hpp"

struct Solver {
  static constexpr u32 MOD = 998244353;

  int n;
  std::vector<u32> a, B, C;
  const std::vector<std::array<u32, 4>> &qs;

  std::vector<int> par, par_edge, tin, sub, child_of_edge;
  std::vector<u32> Pv, Qv;  // 根から頂点への辺の合成 (P_v, Q_v)
  std::vector<int> vert;    // Euler tour の位置 -> 頂点

  // 遅延セグメント木。値は V_v = P_v a_v + Q_v で、遅延は (P_v, Q_v) への 1 次変換 (m, d)。
  std::vector<u32> sum, lzm, lzd;
  std::vector<u32> ans;

  static u32 addm(u32 x, u32 y) {
    const u32 s = x + y;
    return s >= MOD ? s - MOD : s;
  }
  static u32 mulm(u32 x, u32 y) { return (u32)((u64)x * y % MOD); }
  static u32 powm(u32 b, u64 e) {
    u32 r = 1;
    while (e) {
      if (e & 1) r = mulm(r, b);
      b = mulm(b, b);
      e >>= 1;
    }
    return r;
  }

  Solver(int n_, const std::vector<u32> &a_,
         const std::vector<std::array<int, 2>> &edges,
         const std::vector<u32> &b_, const std::vector<u32> &c_,
         const std::vector<std::array<u32, 4>> &queries)
      : n(n_), a(a_), B(b_), C(c_), qs(queries) {
    par.assign(n, -1);
    par_edge.assign(n, -1);
    child_of_edge.assign(n - 1, -1);
    std::vector<std::vector<std::pair<int, int>>> g(n);
    for (int i = 0; i + 1 < n; ++i) {
      g[edges[i][0]].push_back({edges[i][1], i});
      g[edges[i][1]].push_back({edges[i][0], i});
    }

    // 根 0 から preorder で巡る。部分木はこの並びで連続になる。
    vert.reserve(n);
    std::vector<char> vis(n, 0);
    std::vector<int> st;
    st.push_back(0);
    vis[0] = 1;
    while (!st.empty()) {
      const int v = st.back();
      st.pop_back();
      vert.push_back(v);
      for (const auto &e : g[v]) {
        if (vis[e.first]) continue;
        vis[e.first] = 1;
        par[e.first] = v;
        par_edge[e.first] = e.second;
        child_of_edge[e.second] = e.first;
        st.push_back(e.first);
      }
    }

    // 部分木の大きさ (preorder の後ろから親へ足す)。
    sub.assign(n, 1);
    for (int i = n - 1; i >= 1; --i) sub[par[vert[i]]] += sub[vert[i]];

    tin.assign(n, 0);
    for (int i = 0; i < n; ++i) tin[vert[i]] = i;

    // 根から頂点への合成。R_v = R_par * M_e。
    Pv.assign(n, 0);
    Qv.assign(n, 0);
    Pv[0] = 1;
    for (int i = 1; i < n; ++i) {
      const int v = vert[i], p = par[v], e = par_edge[v];
      Pv[v] = mulm(Pv[p], B[e]);
      Qv[v] = addm(mulm(Pv[p], C[e]), Qv[p]);
    }

    sum.assign(4 * n, 0);
    lzm.assign(4 * n, 1);
    lzd.assign(4 * n, 0);
    build(1, 0, n);
  }

  void build(int node, int l, int r) {
    if (r - l == 1) {
      const int v = vert[l];
      sum[node] = addm(mulm(Pv[v], a[v]), Qv[v]);
      return;
    }
    const int mid = (l + r) >> 1;
    build(node << 1, l, mid);
    build(node << 1 | 1, mid, r);
    sum[node] = addm(sum[node << 1], sum[node << 1 | 1]);
  }

  // 区間 [l, r) の V を m V + d にし、(P_v, Q_v) も同じ変換にする。
  void apply(int node, int l, int r, u32 m, u32 d) {
    sum[node] = (u32)(((u64)m * sum[node] + (u64)d * (u32)(r - l)) % MOD);
    if (r - l == 1) {
      const int v = vert[l];
      Pv[v] = mulm(m, Pv[v]);
      Qv[v] = addm(mulm(m, Qv[v]), d);
    } else {
      lzm[node] = mulm(m, lzm[node]);
      lzd[node] = addm(mulm(m, lzd[node]), d);
    }
  }

  void push(int node, int l, int r) {
    if (lzm[node] == 1 && lzd[node] == 0) return;
    const int mid = (l + r) >> 1;
    apply(node << 1, l, mid, lzm[node], lzd[node]);
    apply(node << 1 | 1, mid, r, lzm[node], lzd[node]);
    lzm[node] = 1;
    lzd[node] = 0;
  }

  void point_set_a(int node, int l, int r, int pos, u32 x) {
    if (r - l == 1) {
      const int v = vert[l];
      a[v] = x;
      sum[node] = addm(mulm(Pv[v], x), Qv[v]);
      return;
    }
    push(node, l, r);
    const int mid = (l + r) >> 1;
    if (pos < mid) point_set_a(node << 1, l, mid, pos, x);
    else point_set_a(node << 1 | 1, mid, r, pos, x);
    sum[node] = addm(sum[node << 1], sum[node << 1 | 1]);
  }

  void point_get(int node, int l, int r, int pos, u32 &p, u32 &q) {
    if (r - l == 1) {
      const int v = vert[l];
      p = Pv[v];
      q = Qv[v];
      return;
    }
    push(node, l, r);
    const int mid = (l + r) >> 1;
    if (pos < mid) point_get(node << 1, l, mid, pos, p, q);
    else point_get(node << 1 | 1, mid, r, pos, p, q);
  }

  void range_apply(int node, int l, int r, int ql, int qr, u32 m, u32 d) {
    if (qr <= l || r <= ql) return;
    if (ql <= l && r <= qr) {
      apply(node, l, r, m, d);
      return;
    }
    push(node, l, r);
    const int mid = (l + r) >> 1;
    range_apply(node << 1, l, mid, ql, qr, m, d);
    range_apply(node << 1 | 1, mid, r, ql, qr, m, d);
    sum[node] = addm(sum[node << 1], sum[node << 1 | 1]);
  }

  void run() {
    ans.assign(qs.size(), 0);
    for (std::size_t i = 0; i < qs.size(); ++i) {
      const std::array<u32, 4> &qq = qs[i];
      if (qq[0] == 0) {
        const int w = (int)qq[1];
        point_set_a(1, 0, n, tin[w], qq[2]);
      } else {
        const int e = (int)qq[1];
        const u32 y = qq[2], z = qq[3];
        const int c = child_of_edge[e];
        u32 pp, qp;
        point_get(1, 0, n, tin[par[c]], pp, qp);
        const u32 m = mulm(y, powm(B[e], MOD - 2));
        const u32 mc = mulm(m, C[e]);
        const u32 t = z >= mc ? z - mc : z + MOD - mc;
        const u32 om = (1 + MOD - m) % MOD;
        const u32 d = addm(mulm(pp, t), mulm(om, qp));
        range_apply(1, 0, n, tin[c], tin[c] + sub[c], m, d);
        B[e] = y;
        C[e] = z;
      }
      ans[i] = sum[1];
    }
  }

  const std::vector<u32> &answers() const { return ans; }
};
