#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/nt/sqrt_mod.hpp"
#include "neo/base/core/poly/sparse_fps.hpp"
// NeoLibrary の sparse_fps::exp_of_div。g = 4x + 9x^2 + ... を有理関数 f/g で書いて、exp(f/g) の係数を漸化式で
// O(N * 次数) で出す。長さ n + 1 を渡す。虚数単位は NeoLibrary の sqrt_mod。
struct Solver {
 int n;
 vector<i64> ans;
 explicit Solver(int n): n(n) {}
 void run() {
  static constexpr int MOD= 1e9 + 9;
  using Z= ZMod<MOD>;
  Z im= Z(sqrt_mod(MOD - 1, MOD)), cf= Z(1) / (im + 1);
  vector<Z> f= {0, 4, -3, 1}, g= {1, -3, 3, -1};  // 4x + 9x^2 + ... = f / g
  for(auto& x: f) x*= im;
  auto exp_pi= sparse_fps::exp_of_div(f, g, n + 1);
  for(auto& x: f) x= -x;
  auto exp_mi= sparse_fps::exp_of_div(f, g, n + 1);
  for(int i= 2; i <= n; i++) cf*= i;
  ans.resize(n);
  for(int i= 1; i <= n; i++) ans[i - 1]= ((exp_pi[i] + im * exp_mi[i]) * cf).val();
 }
 const vector<i64>& answer() const { return ans; }
};
