#pragma once
// 期待出力を作る参照実装 (ライブラリに依存しない)。上の要素から 1 つずつ足す 3^n の DP で、h[∅] = 1、
// h[S ∪ {i}] = sum_{T ⊆ S} f[T ∪ {i}] h[S \ T] (i は S ∪ {i} の最大の要素)。値は u64 の桁あふれのまま (法 2^64)。
#include "pj.hpp"
struct Solver {
  int n;
  vector<u64> f, h;
  Solver(int n_, const vector<u64> &f_) : n(n_), f(f_) {}
  void run() {
    const size_t N = size_t(1) << n;
    h.assign(N, 0);
    h[0] = 1;
    for (int i = 0; i < n; ++i) {
      const size_t L = size_t(1) << i;
      const u64 *fi = f.data() + L;
      for (size_t s = 0; s < L; ++s) {
        u64 t = 0;
        for (size_t a = s;; a = (a - 1) & s) {
          t += fi[a] * h[s ^ a];
          if (!a) break;
        }
        h[L | s] = t;
      }
    }
  }
  vector<u64> answer() const { return h; }
};
