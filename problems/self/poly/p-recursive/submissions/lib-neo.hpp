#pragma once
// _shared/ntt/p_recursive.hpp の p_recursive::nth_values (値ベースの BGS) を、NeoLibrary の既定の核 (単独の NTT、スカラー基数 4) で回す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "_shared/ntt/p_recursive.hpp"
inline u32 run(const std::vector<std::vector<u32>>& p, const std::vector<u32>& a0, u64 N) {
 using Z= ZMod<998244353>;
 std::vector<std::vector<Z>> pp(p.size());
 for (size_t i= 0; i < p.size(); ++i) {
  pp[i].resize(p[i].size());
  for (size_t j= 0; j < p[i].size(); ++j) pp[i][j]= Z::raw(p[i][j]);
 }
 std::vector<Z> aa(a0.size());
 for (size_t i= 0; i < a0.size(); ++i) aa[i]= Z::raw(a0[i]);
 return p_recursive::nth_values(pp, aa, N).val();
}
