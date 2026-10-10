#pragma once
#include "../common.hpp"
// NeoLibrary の inv_mod (neo/number_theory/inv_gcd.hpp、32 bit の道) を、ZMod<998244353> の val() に使う。
// ZMod の inv() にこの方式を写すかを決めるための比べる相手。neo/number_theory/inv_gcd.hpp か neo/algebra/ZMod.hpp が変わると測り直される。
#include "neo/algebra/ZMod.hpp"
#include "neo/number_theory/inv_gcd.hpp"
inline vector<u32> run(u32, const vector<u32>& qs) {
 using Z= ZMod<998244353>;
 vector<u32> ans;
 ans.reserve(qs.size());
 for(u32 a : qs) {
  const u32 v= Z(a).val();
  ans.push_back(v == 0 ? u32(-1) : inv_mod<u32>(v, Z::mod()));
 }
 return ans;
}
