#pragma once
// 期待出力を作る参照実装 (ライブラリに依存しない)。源になる強連結成分 (入ってくる辺の無い成分) の頂点の集合 T で包除する 3^n の
// DP。f[S] を G[S] の辺の部分集合で強連結なものの数、g[S] を S を強連結な塊に分け、塊どうしに辺を置かないものの (-1)^{塊の数+1}
// を付けた和とすると、
//   g[S] = 2^{e(S)} - sum_{∅≠T⊊S} g[T] 2^{e(S\T) + c(T→S\T)}、 f[S] = g[S] + sum_{min S ∈ T⊊S} f[T] g[S\T]
// (e は内側の辺の数、c(T→U) は T から U へ向かう辺の数)。c(T→S\T) = c(T→S) - e(T) で、c(T→S) は S ごとに T の小さい順に足して作る。
#include "pj.hpp"
struct Solver {
  static constexpr u64 MOD = 1000000007;
  int n;
  vector<pair<int, int>> es;
  u32 ans = 0;
  Solver(int n_, const vector<pair<int, int>> &es_) : n(n_), es(es_) {}
  void run() {
    const int N = 1 << n;
    vector<u32> out(n, 0);
    for (auto [a, b] : es) out[a] |= 1u << b;
    vector<int> e(N, 0);
    for (int s = 1; s < N; ++s) {
      const int v = __builtin_ctz(s), r = s & (s - 1);
      int add = __builtin_popcount(out[v] & r);
      for (int u = 0; u < n; ++u)
        if (r >> u & 1) add += out[u] >> v & 1;
      e[s] = e[r] + add;
    }
    vector<u64> pw(n * n + 1);
    pw[0] = 1;
    for (size_t i = 1; i < pw.size(); ++i) pw[i] = pw[i - 1] * 2 % MOD;
    vector<u64> f(N, 0), g(N, 0);
    vector<int> c(N, 0);  // c[T] = c(T→S) (S ごとに作り直す)
    for (int s = 1; s < N; ++s) {
      // c(T→S) を T の小さい順に
      for (int t = (0 - s) & s; t; t = (t - s) & s) {
        const int v = __builtin_ctz(t);
        c[t] = c[t & (t - 1)] + __builtin_popcount(out[v] & s);
      }
      c[0] = 0;
      u64 gs = pw[e[s]];
      for (int t = (s - 1) & s; t; t = (t - 1) & s) {
        const int u = s ^ t;
        gs = (gs + (MOD - g[t]) * pw[e[u] + c[t] - e[t]]) % MOD;
      }
      g[s] = gs;
      const int lo = s & -s, rest = s ^ lo;
      u64 fs = gs;
      for (int t = (rest - 1) & rest;; t = (t - 1) & rest) {  // T = lo | t ⊊ S
        if ((lo | t) != s) fs = (fs + f[lo | t] * g[s ^ (lo | t)]) % MOD;
        if (!t) break;
      }
      f[s] = fs;
    }
    ans = u32(f[N - 1]);
  }
  u32 answer() const { return ans; }
};
