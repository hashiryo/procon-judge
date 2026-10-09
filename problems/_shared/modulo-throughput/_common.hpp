#pragma once
// self/modulo-throughput の 3 問で共通の部分 (配列の値の作り方、計算、ハッシュ)。
// 設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
// 提出は modulo-test と同じ struct MP なので、modulo-test の型別名 (u32 など) をここでも読む。
#include "_shared/modulo-test/_common.hpp"

namespace modulo_throughput {

// 乱数は splitmix64。std::uniform_int_distribution は標準ライブラリによって結果が違うので使わない。
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

// 配列の値を作る。fill = 0 は [0, mod) の一様乱数、fill = 1 はすべて mod - 1。
inline vector<u32> make_values(size_t n, u64 seed, int fill, u32 mod) {
  vector<u32> v(n);
  SplitMix64 rng(seed);
  for (auto &x : v) x= fill == 1 ? mod - 1 : u32(rng.below(mod));
  return v;
}

// 計算の本体。c[i] = c[i] * a[i] + b[i] を 1 回ぶん。
// GCC の -O2 は別名の検査をループの前に足す形のベクトル化をしないので、引数に __restrict を付ける。
// always_inline で main に展開させて、静的な法の問題では法の定数をコンパイラに見せたままにする。
template <class M, class V>
[[gnu::always_inline]] inline void step(const M &mp, V *__restrict c, const V *__restrict a, const V *__restrict b, size_t n) {
  for (size_t i= 0; i < n; ++i) c[i]= mp.plus(mp.mul(c[i], a[i]), b[i]);
}

// get で戻した値の列の、順序に依存するハッシュ。
template <class M, class V>
inline u64 hash_values(const M &mp, const vector<V> &c) {
  u64 h= 0;
  for (const auto &v : c) h= h * 1000003 + u64(mp.get(v));
  return h;
}

}  // namespace modulo_throughput
