#pragma once
// failtree4 の R の足し上げで、suffix link の先が R の 1 つ前の頂点なら、足す値をメモリでなくレジスタで次へ渡すもの。a だけのパターンの
// ように suffix link が 1 本の鎖になると、メモリを通すと足し込みが前の書き込みを 1 つずつ待つ。以下は failtree4 の説明。
//
// failtree3 の (2) と (3) を、引く値が 0 でない頂点だけに絞ったもの。引く値が 0 でないのは、死んだ状態の suffix link の先から根までの
// 鎖に乗る頂点 (集合 R) だけなので、(2) は R を幅優先の逆順にたどり、(3) は全部の頂点に F(親) を写してから R の頂点だけを直す。写す
// 走査は割り算も引き算も無い読み出しと書き込みだけになる。R は生きた状態の 0.1 % から 30 % ほど (a だけのパターンでは全部)。以下は
// failtree3 の説明。
//
// 値そのものではなく、suffix link の木の部分木の和 F(x) (今の文字列が x の文字列で終わる場合の数) を持って 1 歩ずつ進める DP。
//
// 死んだ状態が無ければ、1 文字足したあとに x の文字列で終わる文字列は、足す前に x の親の文字列で終わっていたものに x の文字を足した
// ものなので F'(x) = F(x の親) になる。死んだ状態があるときは、x の部分木にある死んだ状態 y (suffix link の先が生きているもの) の部分木は
// 全部死んでいて、そこへ入る分 F(y の親) を引けばよい。
//   F'(x) = F(x の親) - Σ_{x の部分木の y} F(y の親)、F'(根) = σ F(根) - Σ_{すべての y} F(y の親)
// そこで 1 歩を、(1) 死んだ状態 y ごとに F(y の親) を y の suffix link の先に足す、(2) 幅優先の逆順に suffix link の先へ足し上げる、
// (3) 幅優先の順に F'(x) = F(x の親) - (足し上げた値) を求める、で行う。配列は生きた状態だけを幅優先の順に詰めたもので、死んだ状態は
// (親、suffix link の先) の組の列にだけ残る。答えは n 歩後の F(根)。
//
// トライと suffix link は NeoLibrary の aho_corasick (neo/string/aho_corasick.hpp) で求め、親ごとの子の列で幅優先の順に並べ直す。
#include "pj.hpp"
#include "neo/string/aho_corasick.hpp"

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    constexpr u64 MOD = 998244353;
    const AhoCorasick ac = aho_corasick(ps);
    const int K = (int)ac.par.size();
    // 親ごとの子の列 (CSR) を作り、根から幅優先にたどって番号 idx を振る。
    vector<int> off(K + 1, 0), kids(max(K - 1, 1)), idx(K), bfs(K), dep(K, 0);
    for (int v = 1; v < K; ++v) ++off[ac.par[v] + 1];
    for (int v = 0; v < K; ++v) off[v + 1] += off[v];
    {
      vector<int> pos(off.begin(), off.end() - 1);
      for (int v = 1; v < K; ++v) kids[pos[ac.par[v]]++] = v;
    }
    bfs[0] = 0, idx[0] = 0;
    for (int h = 0, t = 1; h < K; ++h) {
      const int v = bfs[h];
      for (int j = off[v]; j < off[v + 1]; ++j) idx[kids[j]] = t, dep[t] = dep[h] + 1, bfs[t++] = kids[j];
    }
    vector<int>().swap(off), vector<int>().swap(kids), vector<int>().swap(bfs);
    vector<int> par(K), link(K);
    vector<unsigned char> dead(K, 0);
    for (int v = 0; v < K; ++v) par[idx[v]] = idx[ac.par[v]], link[idx[v]] = idx[ac.link[v]];
    for (const int w : ac.word) dead[idx[w]] = 1;
    for (int i = 1; i < K; ++i) dead[i] |= dead[link[i]];
    // 生きた状態 (死んでおらず、親も生きている頂点) に幅優先の順の番号 aid を振る。
    vector<int> aid(K, -1);
    int A = 1;
    aid[0] = 0;
    for (int i = 1; i < K; ++i)
      if (!dead[i] && aid[par[i]] >= 0) aid[i] = A++;
    const int D = dep[K - 1];
    vector<int> P(A), L(A), ua(D + 1, 0), ue(D + 1, 0);  // ua[d]、ue[d] は深さ d 以下の生きた状態の数と死んだ組の数
    vector<pair<int, int>> E;                              // 死んだ状態の (親、suffix link の先)。どちらも生きた状態の番号
    for (int i = 1; i < K; ++i) {
      if (aid[i] >= 0) {
        P[aid[i]] = aid[par[i]], L[aid[i]] = aid[link[i]];
        ua[dep[i]] = aid[i] + 1;
      } else if (dead[i] && aid[par[i]] >= 0 && !dead[link[i]]) {
        E.emplace_back(aid[par[i]], aid[link[i]]);
        ue[dep[i]] = (int)E.size();
      }
    }
    ua[0] = 1;
    for (int d = 1; d <= D; ++d) ua[d] = max(ua[d], ua[d - 1]), ue[d] = max(ue[d], ue[d - 1]);
    // R は死んだ状態の suffix link の先から根までの鎖に乗る頂点を、幅優先の順に並べたもの (根は除く)。ur[d] は深さ d 以下の数。
    vector<int> R, ur(D + 1, 0);
    {
      vector<char> in(A, 0);
      in[0] = 1;
      for (const auto &e : E)
        for (int x = e.second; !in[x]; x = L[x]) in[x] = 1;
      vector<int> adep(A, 0);
      for (int x = 1; x < A; ++x) adep[x] = adep[P[x]] + 1;
      for (int x = 1; x < A; ++x)
        if (in[x]) R.push_back(x), ur[adep[x]] = (int)R.size();
      for (int d = 1; d <= D; ++d) ur[d] = max(ur[d], ur[d - 1]);
    }
    vector<u64> acc(A, 0);
    vector<u32> F(A, 0), G(A, 0);
    F[0] = 1;
    for (int i = 1; i <= n; ++i) {
      const int ra = ua[min(i, D)], re = ue[min(i, D)], rr = ur[min(i, D)];
      for (int e = 0; e < re; ++e) acc[E[e].second] += F[E[e].first];
      u64 carry = 0;  // R[j + 1] から R[j] へ渡す値 (R[j + 1] の suffix link の先が R[j] のとき)
      for (int j = rr - 1; j >= 0; --j) {
        const int x = R[j], l = L[x];
        const u64 v = acc[x] + carry;
        acc[x] = v;
        if (j > 0 && l == R[j - 1]) carry = v;
        else acc[l] += v, carry = 0;
      }
      G[0] = u32((u64(sigma) * F[0] + MOD - acc[0] % MOD) % MOD);
      acc[0] = 0;
      for (int x = 1; x < ra; ++x) G[x] = F[P[x]];
      for (int j = 0; j < rr; ++j) {
        const int x = R[j];
        const u64 g = G[x] + MOD - acc[x] % MOD;
        G[x] = u32(g >= MOD ? g - MOD : g);
        acc[x] = 0;
      }
      F.swap(G);
    }
    return F[0];
  }
};
