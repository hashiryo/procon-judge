// 愚直解 (提出ではない)。nCr を r 個の掛け算と 1 回のフェルマーの逆元で求める。参照実装と小さい入力
// (seed 1000 以上、N <= 200) で突き合わせる。
#pragma once
#include "pj.hpp"

namespace factorial_brute_impl {
inline unsigned long long pow_mod(unsigned long long a, unsigned long long e, unsigned long long m) {
 unsigned long long r= 1;
 for(a%= m; e; e>>= 1, a= (unsigned long long)((u128)a * a % m))
  if(e & 1) r= (unsigned long long)((u128)r * a % m);
 return r;
}
}  // namespace factorial_brute_impl

struct Solver {
 std::vector<unsigned> run(unsigned N, const std::vector<unsigned>& ns, const std::vector<unsigned>& rs) {
  constexpr unsigned long long M= 998244353;
  (void)N;
  std::vector<unsigned> out;
  out.reserve(ns.size());
  for (std::size_t i= 0; i < ns.size(); ++i) {
   const unsigned n= ns[i], r= std::min(rs[i], ns[i] - rs[i]);
   unsigned long long num= 1, den= 1;
   for (unsigned k= 0; k < r; ++k) {
    num= num * (n - k) % M;
    den= den * (k + 1) % M;
   }
   out.push_back((unsigned)(num * factorial_brute_impl::pow_mod(den, M - 2, M) % M));
  }
  return out;
 }
};
