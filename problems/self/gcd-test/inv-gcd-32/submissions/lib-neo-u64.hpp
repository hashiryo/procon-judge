#pragma once
// NeoLibrary の inv_gcd を u64 に広げて呼ぶ。lib-neo.hpp (u32 のまま呼ぶ) との差が、32 bit の道で還元を 1 回にした得。
#include "_shared/modulo-test/_common.hpp"
#include "neo/number_theory/inv_gcd.hpp"
inline std::pair<u32, u32> run(u32 a, u32 b) {
 auto [g, x]= inv_gcd<u64>(a, b);
 return {u32(g), u32(x)};
}
