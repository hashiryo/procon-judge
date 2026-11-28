// 愚直解 (提出ではない)。参照実装と小さい入力 (seed 1000 以上) で突き合わせる。
// a ↑↑ b を、指数を u128 で正確に持てる範囲でそのまま累乗する。λ の再帰が要る段 (b >= 3) は、
// 2 の塔 (b <= 5、指数 65536 まで) と 3 の塔 (b <= 4、指数 3^27 まで) で確かめる。
#pragma once
#include "pj.hpp"

namespace brute_impl {
inline unsigned long long pow_mod(unsigned long long a, unsigned long long e, unsigned long long m) {
 unsigned long long r= 1;
 for(a%= m; e; e>>= 1, a= (unsigned long long)((u128)a * a % m))
  if(e & 1) r= (unsigned long long)((u128)r * a % m);
 return r;
}
}  // namespace brute_impl

struct Solver {
 unsigned solve(unsigned a, unsigned b) {
  constexpr unsigned long long M= 998244353;
  if (b == 0) return 1;
  // t = a ↑↑ i を正確に持つ。最後の段だけ mod M で累乗する。
  u128 t= 1;
  for (unsigned i= 0; i < b; ++i) {
   if (i + 1 == b) {
    assert(t <= ~0ull);
    return (unsigned)brute_impl::pow_mod(a, (unsigned long long)t, M);
   }
   // t = a^t を exact に。u128 に収まらない入力は小さい seed では来ない。
   u128 p= 1, base= a;
   for (u128 e= t; e; e>>= 1) {
    if (e & 1) p*= base;
    if (e >>= 1) base*= base;
   }
   t= p;
  }
  return 1;
 }
};
