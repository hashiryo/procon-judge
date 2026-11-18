#pragma once
#include "pj.hpp"

// 全方位木 DP の書き比べ。どれも今の Library の Rerooting と同じ関数 4 つを受ける。
//   put_edge(v, e, x) -> U: 辺 e の向こうの部分木の値 x を、v から見た寄与にする
//   op(l, r) -> U:          寄与をまとめる (可換で結合的、単位元は ui)
//   put_vertex(v, s) -> T:  v に集まった寄与 s から v の値を作る
// 作るのは、頂点 v を根にしたときの値 dp[v] (頂点の順) と、内部の根で見た部分木の値 dp1 と、親の側の値 dp2 (Rerooting の
// operator()(root, v) に要るので、この問題では使わなくても作る)。森でもよい。
//   RerootPeel: 隣接を組まず、頂点ごとの次数と隣の番号の xor と辺の番号の xor で葉から剥がし、剥がした順 (子が親より前) に
//               番号 k を振り直す。下からの段は k の順に回って親へ寄与を足し、上からの段は逆順に回る。兄弟を除いた積は、
//               下からの段で親にそれまで集まった積 (pre) を、上からの段で親の残りの積 (sacc) を持って作る。配列は k の順で、
//               親の acc と sacc の読み書きだけがばらばらな番地になる。
//   RerootBfs<CSR>: 兄弟が並ぶ BFS の順に並べ直してから、2 つの段を位置の順に回す。配列はどれも位置の順で、段の中で親を
//               読まない。CSR が偽なら RerootPeel と同じく剥がして親を決め、子の CSR を組んで BFS する。真なら隣接の CSR を
//               組んで根から BFS する。
namespace reroot_exp {
// 葉から剥がして、頂点ごとの親と親への辺を決める。剥がした順 (子が親より前、根は最後) を ord に入れ、根の数を返す。
inline int peel(int n, const vector<array<int, 2>> &edges, int *ord, int *par, int *pe) {
  vector<int> dx(3 * size_t(n), 0);  // dx[3v] = 次数、dx[3v + 1] = 隣の番号の xor、dx[3v + 2] = 辺の番号の xor
  for (int i = 0; i < (int)edges.size(); ++i) {
    const int a = edges[i][0], b = edges[i][1];
    ++dx[3 * a], dx[3 * a + 1] ^= b, dx[3 * a + 2] ^= i;
    ++dx[3 * b], dx[3 * b + 1] ^= a, dx[3 * b + 2] ^= i;
  }
  int k = 0;
  for (int v = 0; v < n; ++v)
    for (int u = v; dx[3 * u] == 1;) {
      const int p = dx[3 * u + 1], e = dx[3 * u + 2];
      ord[k++] = u, par[u] = p, pe[u] = e, dx[3 * u] = 0;
      dx[3 * p + 1] ^= u, dx[3 * p + 2] ^= e, --dx[3 * p];
      u = p;
    }
  // 剥がれずに残った頂点が根 (呼ぶ側が親の有無で見分ける)。
  return n - k;
}

template <class T> struct RerootPeel {
  int n;
  vector<T> dp, dp1, dp2;  // dp は頂点の順、dp1 と dp2 は剥がした順の番号 k の順
  template <class U, class PE, class OP, class PV>
  RerootPeel(int n_, const vector<array<int, 2>> &edges, const PE &put_edge, const OP &op, const U &ui, const PV &put_vertex)
      : n(n_), dp(n_), dp1(n_), dp2(n_) {
    if (n <= 0) return;
    vector<int> buf(5 * size_t(n));
    int *vert = buf.data(), *pv = vert + n, *pk = pv + n, *pe = pk + n, *id = pe + n;  // 番号 k ごとの頂点、親の頂点、親の番号、親への辺
    {
      vector<int> par(n, -1), pev(n, -1);
      const int nr = peel(n, edges, vert, par.data(), pev.data());
      int k = n - nr;
      for (int v = 0; v < n; ++v)
        if (par[v] < 0) vert[k++] = v;
      for (int i = 0; i < n; ++i) id[vert[i]] = i;
      for (int i = 0; i < n; ++i) {
        const int v = vert[i], p = par[v];
        pv[i] = p, pk[i] = p < 0 ? -1 : id[p], pe[i] = pev[v];
      }
    }
    vector<U> acc(n, ui), pre(n), g(n), sacc(n);
    for (int i = 0; i < n; ++i) {
      const int v = vert[i], p = pk[i];
      dp1[i] = put_vertex(v, acc[i]);
      if (p >= 0) {
        const U x = put_edge(pv[i], pe[i], dp1[i]);
        g[i] = x, pre[i] = acc[p], acc[p] = op(acc[p], x);
      }
    }
    for (int i = n; i--;) {
      const int v = vert[i], p = pk[i];
      U up = ui;
      if (p >= 0) {
        dp2[i] = put_vertex(pv[i], op(pre[i], sacc[p]));
        sacc[p] = op(sacc[p], g[i]);
        up = put_edge(v, pe[i], dp2[i]);
      }
      dp[v] = put_vertex(v, op(acc[i], up));
      sacc[i] = up;
    }
  }
};

template <class T, bool CSR> struct RerootBfs {
  int n;
  vector<T> dp, dp1, dp2;  // dp は頂点の順、dp1 と dp2 は BFS の位置の順
  template <class U, class PE, class OP, class PV>
  RerootBfs(int n_, const vector<array<int, 2>> &edges, const PE &put_edge, const OP &op, const U &ui, const PV &put_vertex)
      : n(n_), dp(n_), dp1(n_), dp2(n_) {
    if (n <= 0) return;
    // 位置 i ごとに、頂点 vert[i]、親への辺 pe[i] (根は -1)、子の位置の始まり cst[i] (子は [cst[i], cst[i + 1]))。
    vector<int> buf(3 * size_t(n) + 1);
    int *vert = buf.data(), *pe = vert + n, *cst = pe + n;
    if constexpr (!CSR) {
      vector<int> tmp(4 * size_t(n) + 1);
      int *ord = tmp.data(), *par = ord + n, *pev = par + n, *st = pev + n;  // st は n + 1 個
      std::fill(par, par + n, -1);
      peel(n, edges, ord, par, pev);
      // 子の CSR (cs は ord を使い回す)。並べたあと、v の子は cs[v ? st[v - 1] : 0, st[v])。
      int *cs = ord;
      std::fill(st, st + n + 1, 0);
      for (int v = 0; v < n; ++v)
        if (par[v] >= 0) ++st[par[v] + 1];
      for (int v = 0; v < n; ++v) st[v + 1] += st[v];
      for (int v = 0; v < n; ++v)
        if (par[v] >= 0) cs[st[par[v]]++] = v;
      int k = 0;
      for (int v = 0; v < n; ++v)
        if (par[v] < 0) vert[k] = v, pe[k++] = -1;
      for (int i = 0; i < n; ++i) {
        const int v = vert[i];
        cst[i] = k;
        for (int j = v ? st[v - 1] : 0, e = st[v]; j < e; ++j) {
          const int c = cs[j];
          vert[k] = c, pe[k++] = pev[c];
        }
      }
      cst[n] = n;
    } else {
      vector<int> st(n + 1, 0);
      vector<array<int, 2>> adj(2 * edges.size());  // (隣, 辺)
      for (const auto &e : edges) ++st[e[0] + 1], ++st[e[1] + 1];
      for (int v = 0; v < n; ++v) st[v + 1] += st[v];
      for (int i = 0; i < (int)edges.size(); ++i) {
        const int a = edges[i][0], b = edges[i][1];
        adj[st[a]++] = {b, i}, adj[st[b]++] = {a, i};
      }
      // 兄弟が並ぶよう、根を全部先に置いてから BFS する。木なら根は 0 で、森のときだけ先に連結成分ごとの根を探す。
      int k = 0;
      if ((int)edges.size() + 1 == n) {
        vert[k] = 0, pe[k++] = -1;
      } else {
        vector<char> seen(n, 0);
        vector<int> q;
        for (int s = 0; s < n; ++s) {
          if (seen[s]) continue;
          vert[k] = s, pe[k++] = -1, seen[s] = 1, q.assign(1, s);
          while (!q.empty()) {
            const int v = q.back();
            q.pop_back();
            for (int j = v ? st[v - 1] : 0, e = st[v]; j < e; ++j)
              if (!seen[adj[j][0]]) seen[adj[j][0]] = 1, q.push_back(adj[j][0]);
          }
        }
      }
      for (int i = 0; i < n; ++i) {
        const int v = vert[i], ep = pe[i];
        cst[i] = k;
        for (int j = v ? st[v - 1] : 0, e = st[v]; j < e; ++j)
          if (adj[j][1] != ep) vert[k] = adj[j][0], pe[k++] = adj[j][1];
      }
      cst[n] = n;
    }
    vector<U> g(n), sf(n);
    for (int i = n; i--;) {
      const int v = vert[i];
      U s = ui;
      for (int j = cst[i], e = cst[i + 1]; j < e; ++j) {
        const U x = put_edge(v, pe[j], dp1[j]);
        g[j] = x, s = op(s, x);
      }
      dp1[i] = put_vertex(v, s);
    }
    for (int i = 0; i < n; ++i) {
      const int v = vert[i], l = cst[i], r = cst[i + 1];
      U pre = pe[i] < 0 ? ui : put_edge(v, pe[i], dp2[i]);
      U s = ui;
      for (int j = r; j-- > l;) sf[j] = s, s = op(g[j], s);  // sf[j] は g[j + 1], ..., g[r - 1] の積
      for (int j = l; j < r; ++j) dp2[j] = put_vertex(v, op(pre, sf[j])), pre = op(pre, g[j]);
      dp[v] = put_vertex(v, pre);
    }
  }
};
}  // namespace reroot_exp
