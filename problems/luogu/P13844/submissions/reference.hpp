#pragma once
// 期待出力を作る参照実装 (ライブラリに依存しない)。f = exp(g) の上の要素 i を含む部分は f_hi = g_hi * f_lo (部分集合畳み込み、
// X_hi[S] = X[S ∪ {i}]) なので、f_lo[∅] = 1 を使って g_hi[S] = f_hi[S] - sum_{∅≠T⊆S} f_lo[T] g_hi[S \ T] と S の小さい順に求める
// 3^n の DP。値は u64 の桁あふれのまま (法 2^64)。
#include "pj.hpp"
struct Solver {
  int n;
  vector<u64> f, g;
  Solver(int n_, const vector<u64> &f_) : n(n_), f(f_) {}
  void run() {
    const size_t N = size_t(1) << n;
    g.assign(N, 0);
    for (int i = 0; i < n; ++i) {
      const size_t L = size_t(1) << i;
      const u64 *fh = f.data() + L;
      u64 *gh = g.data() + L;
      for (size_t s = 0; s < L; ++s) {
        u64 t = fh[s];
        for (size_t a = s; a; a = (a - 1) & s) t -= f[a] * gh[s ^ a];
        gh[s] = t;
      }
    }
  }
  vector<u64> answer() const { return g; }
};
