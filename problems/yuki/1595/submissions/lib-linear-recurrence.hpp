#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/bostan_mori.hpp"

// 線形漸化式の第 K 項を Bostan-Mori で出す。剰余 10 は素数ではないが、割るのは
// 分母の定数項 1 だけなので成り立つ。O(d^2 log K)。
struct Solver {
  i64 p, q, r, k, ans = 0;

  Solver(i64 p, i64 q, i64 r, i64 k) : p(p), q(q), r(r), k(k) {}

  void run() {
    using Mint = ModInt<10>;
    ans = linear_recurrence<Mint>({1, 1, 1}, {Mint(p), Mint(q), Mint(r)}, k - 1).val();
  }

  i64 answer() const { return ans; }
};
