#pragma once
// self/ntt の 4 問で共通の部分 (配列の値の作り方、ハッシュ、回ごとの調整)。
// pj.hpp を読んだあとの base.cpp から読む。配列は 1 ケース 1 本で、ハーネスが splitmix64 で作る。
#include <vector>

namespace ntt_common {

using u32= unsigned int;
using u64= unsigned long long;
using u128= __uint128_t;

// std::uniform_int_distribution は標準ライブラリによって結果が違うので使わない。
struct SplitMix64 {
 u64 x;
 explicit SplitMix64(u64 seed): x(seed) {}
 u64 operator()() {
  u64 z= (x+= 0x9E3779B97F4A7C15ull);
  z= (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
  z= (z ^ (z >> 27)) * 0x94D049BB133111EBull;
  return z ^ (z >> 31);
 }
 u64 below(u64 n) { return u64((u128((*this)()) * n) >> 64); }
};

// fill = 0: [0, mod) の一様乱数、1: すべて mod - 1、2: すべて 0。
inline std::vector<u32> make_values(int n, u64 seed, int fill, u32 mod) {
 std::vector<u32> v(n);
 SplitMix64 rng(seed);
 for (auto &x: v) x= fill == 1 ? mod - 1 : fill == 2 ? 0 : u32(rng.below(mod));
 return v;
}

// 長さを含めた、順序に依存するハッシュ。
inline u64 hash_values(const std::vector<u32> &v) {
 u64 h= 0x100000001b3ull;
 h= h * 1000003 + v.size();
 for (u32 x: v) h= h * 0x100000001b3ull + x;
 return h;
}

// 回ごとに入力の 1 個を変える。同じ計算をコンパイラがループの外へ出すのを防ぐ。
inline void tweak(std::vector<u32> &v, u64 q, u32 mod) {
 if (!v.empty()) v[0]= u32((u64(v[0]) + q) % mod);
}

}  // namespace ntt_common
