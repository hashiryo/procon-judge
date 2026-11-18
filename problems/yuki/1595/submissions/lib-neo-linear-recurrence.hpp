#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/polynomial/linear_recurrence.hpp"
// NeoLibrary の linear_recurrence (Bostan-Mori) を ZMod<10> で使う。法 10 は素数ではないが、割るのは分母の定数項 1
// だけなので成り立つ。O(d^2 log K)。
struct Solver {
 i64 p, q, r, k, ans= 0;
 Solver(i64 p, i64 q, i64 r, i64 k): p(p), q(q), r(r), k(k) {}
 void run() {
  using Z= ZMod<10>;
  ans= linear_recurrence<Z>({1, 1, 1}, {Z(p), Z(q), Z(r)}, k - 1).val();
 }
 i64 answer() const { return ans; }
};
