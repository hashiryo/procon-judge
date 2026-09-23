#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/fps_exp.hpp"
#include "mylib/fft/fps_inv.hpp"
#include "mylib/number_theory/mod_sqrt.hpp"

// 密な形式的冪級数。係数を配列に並べて exp を FFT で O(N log N) で出し、
// exp(-i g) は exp(i g) の逆元で出す。
struct Solver {
  int n;
  vector<i64> ans;

  explicit Solver(int n) : n(n) {}

  void run() {
    static constexpr int MOD = 1e9 + 9;
    using Mint = ModInt<MOD>;
    vector<Mint> f(n + 1);
    for (int i = 1; i <= n; i++) f[i] = Mint(i + 1) * (i + 1);
    Mint im = mod_sqrt(MOD - 1, MOD), cf = Mint(1) / (im + 1);
    for (auto &x : f) x *= im;
    auto exp_pi = exp<Mint, 1 << 20>(f), exp_mi = inv<Mint, 1 << 20>(exp_pi);
    for (int i = 2; i <= n; i++) cf *= i;
    ans.resize(n);
    for (int i = 1; i <= n; i++) ans[i - 1] = ((exp_pi[i] + im * exp_mi[i]) * cf).val();
  }

  const vector<i64> &answer() const { return ans; }
};
