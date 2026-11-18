#pragma once
// 参照実装 (ライブラリに依存しない)。potato167 さんの解説 (定数倍が軽い方法) の漸化式。mask(a) = #{i : s_i ⊆ a} を部分集合の和で
// 求め、f(l, r) を x = l から始めて x = r にちょうど着く列の数とすると、f(l, l + 1) = mask(l)、2^{d+1} で揃った l と r = l + 2^{d+1}
// では L = f(l, l + 2^d)、R = f(l + 2^d, r) として f(l, r) = L R + R - L。答えは f(0, 2^N)。
#include "pj.hpp"
struct Solver {
  static constexpr u32 P = 998244353;
  int n;
  vector<int> s;
  u32 ans = 0;
  Solver(int n_, const vector<int> &s_) : n(n_), s(s_) {}
  void run() {
    const size_t N = size_t(1) << n;
    vector<u32> f(N, 0);
    for (int x : s) f[x] = 1;
    for (int b = 0; b < n; ++b)
      for (size_t a = 0; a < N; ++a)
        if (a >> b & 1) f[a] += f[a ^ (size_t(1) << b)];  // M <= 2e5 なので桁あふれしない
    for (int d = 0; d < n; ++d) {
      const size_t h = size_t(1) << d;
      for (size_t l = 0; l < N; l += 2 * h) {
        const u64 L = f[l] % P, R = f[l + h] % P;
        f[l] = u32((L * R + R + P - L) % P);
      }
    }
    ans = f[0] % P;
  }
  u32 answer() const { return ans; }
};
