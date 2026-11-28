#pragma once
// NeoLibrary の畳み込み (neo/base/core/convolve/ntt.hpp、法 998244353 の NTT) で
//   F(i) = Σ_j p_j q_{i+j} (s_{i+j} - t_j)^2
// を 3 本の相関で求める。p_j は T[j] が * でない印、q_k は S[k] が * でない印で、
// 文字は 1..26 の値。展開すると
//   F(i) = Σ q s^2 p - 2 Σ q s p t + Σ q p t^2
// なので、S 側の 3 列 (q s^2、q s、q) と T 側の 3 列 (p、2 p t、p t^2) の
// middle product を足し引きする。マスクを外すと * の位置の差が消えて偽の一致が出る。
// F(i) は 25^2 * |T| 以下で 998244353 より小さいので、mod で 0 かどうかがそのまま一致の判定になる。
#include <string>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/convolve.hpp"
#include "neo/base/core/convolve/ntt.hpp"

struct Solver {
  using Z = ZMod<998244353>;
  // S 側 (位置 k): q s^2、q s、q。T 側 (位置 j): p、2 p t、p t^2。
  std::vector<Z> s_sq, s_s, s_mask;
  std::vector<Z> t_mask, t_twice, t_sq;
  int n, m;
  std::string ans;

  Solver(const std::string &s, const std::string &t)
      : n(int(s.size())), m(int(t.size())) {
    s_sq.resize(n), s_s.resize(n), s_mask.resize(n);
    for (int k = 0; k < n; ++k) {
      if (s[k] == '*') continue;  // 0 のまま
      const unsigned v = unsigned(s[k] - 'a' + 1);
      s_mask[k] = Z::raw(1);
      s_s[k] = Z::raw(v);
      s_sq[k] = Z::raw(v * v);
    }
    t_mask.resize(m), t_twice.resize(m), t_sq.resize(m);
    for (int j = 0; j < m; ++j) {
      if (t[j] == '*') continue;
      const unsigned v = unsigned(t[j] - 'a' + 1);
      t_mask[j] = Z::raw(1);
      t_twice[j] = Z::raw(2 * v);
      t_sq[j] = Z::raw(v * v);
    }
  }

  void run() {
    const std::vector<Z> c1 = middle_product(s_sq, t_mask);
    const std::vector<Z> c2 = middle_product(s_s, t_twice);
    const std::vector<Z> c3 = middle_product(s_mask, t_sq);
    ans.resize(n - m + 1);
    for (int i = 0; i <= n - m; ++i) {
      const Z f = c1[i] - c2[i] + c3[i];
      ans[i] = (f == Z()) ? '1' : '0';
    }
  }

  const std::string &answer() const { return ans; }
};
