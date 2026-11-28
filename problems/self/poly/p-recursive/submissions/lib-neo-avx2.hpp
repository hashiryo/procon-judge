#pragma once
// NeoLibrary の p_recursive::nth を、AVX2 の核 (NttAvx2) で回す。x64 専用 (arm は SIMDe)。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/ntt_avx2.hpp"
#include "neo/algo/compute/poly/p_recursive.hpp"
inline u32 run(const std::vector<std::vector<u32>>& p, const std::vector<u32>& a0, u64 N) {
 using Z= ZMod<998244353>;
 std::vector<std::vector<Z>> pp(p.size());
 for (size_t i= 0; i < p.size(); ++i) {
  pp[i].resize(p[i].size());
  for (size_t j= 0; j < p[i].size(); ++j) pp[i][j]= Z::raw(p[i][j]);
 }
 std::vector<Z> aa(a0.size());
 for (size_t i= 0; i < a0.size(); ++i) aa[i]= Z::raw(a0[i]);
 return p_recursive::nth<NttAvx2>(pp, aa, N).val();
}
