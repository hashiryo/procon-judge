#pragma once
// 既定の核 (ntt.hpp が選ぶ AVX2。x86 は intrinsics、arm は SIMDe) で回す。
// 実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/linear_recurrence.hpp"
#include "neo/base/core/convolve/ntt.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 std::vector<Z> a, c;
 u64 k;
 i64 out= 0;

 Solver(const std::vector<i64>& aa, const std::vector<i64>& cc, u64 kk): a(aa.size()), c(cc.size()), k(kk) {
  for (size_t i= 0; i < aa.size(); ++i) a[i]= Z::raw((unsigned)aa[i]);
  for (size_t i= 0; i < cc.size(); ++i) c[i]= Z::raw((unsigned)cc[i]);
 }

 void run() { out= linear_recurrence<Z>(c, a, k).val(); }

 i64 value() const { return out; }
};
