#pragma once
#include "../common.hpp"
// NeoLibrary の ZMod<998244353> の inv() をそのまま使う (型の中に持たせた小さい拡張 Euclid の互除法)。
// neo/algebra/ZMod.hpp が変わると測り直される。
#include "neo/algebra/ZMod.hpp"
inline vector<u32> run(u32, const vector<u32>& qs) {
 using Z= ZMod<998244353>;
 vector<u32> ans;
 ans.reserve(qs.size());
 for(u32 a : qs) ans.push_back(a % Z::mod() == 0 ? u32(-1) : Z(a).inv().val());
 return ans;
}
