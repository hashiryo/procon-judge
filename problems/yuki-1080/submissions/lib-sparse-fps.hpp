#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/sparse_fps.hpp"
#include "mylib/number_theory/mod_sqrt.hpp"

// 疎な形式的冪級数。g = 4x + 9x^2 + ... を有理関数 f/g で書いて、exp(f/g) の係数を
// 漸化式で O(N * 次数) で出す。FFT を使わない。
struct Solver {
  int n;
  vector<i64> ans;

  explicit Solver(int n) : n(n) {}

  void run() {
    static constexpr int MOD = 1e9 + 9;
    using Mint = ModInt<MOD>;
    Mint im = mod_sqrt(MOD - 1, MOD), cf = Mint(1) / (im + 1);
    vector<Mint> f = {0, 4, -3, 1}, g = {1, -3, 3, -1};  // 4x + 9x^2 + ... = f / g
    for (auto &x : f) x *= im;
    auto exp_pi = sfps::exp_of_div(f, g, n);
    for (auto &x : f) x = -x;
    auto exp_mi = sfps::exp_of_div(f, g, n);
    for (int i = 2; i <= n; i++) cf *= i;
    ans.resize(n);
    for (int i = 1; i <= n; i++) ans[i - 1] = ((exp_pi[i] + im * exp_mi[i]) * cf).val();
  }

  const vector<i64> &answer() const { return ans; }
};
