#pragma once
/** @see https://min-25.hatenablog.com/entry/2015/04/24/031413
 */
#include "mylib/counting/FactorialPrecalculation.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/number_theory/enumerate_primes.hpp"
#include "mylib/algebra/sample_points_shift.hpp"
inline u32 run(u32 r_, u32 d_, u64 n_) {
 using Mint= ModInt<998244353>;
 using F= FactorialPrecalculation<Mint>;
 long long r= r_, d= d_, n= n_;
 if(--n < 0) return 0;
 if(r == 0) return !d;
 if(r == 1) {
  vector<Mint> y= pow_table<Mint>(d + 1, d);
  for(int i= 1; i <= d + 1; ++i) y[i]+= y[i - 1];
  return sample_points_shift<Mint>(y, n).val();
 }
 vector<Mint> pw(d + 1), y= pow_table<Mint>(d, d);
 pw[0]= 1;
 for(int i= 1; i <= d; i++) pw[i]= pw[i - 1] * r;
 for(int i= 1; i <= d; i++) y[i]= y[i - 1] + pw[i] * y[i];
 Mint ans= 0;
 for(int i= 0; i <= d; ++i) {
  Mint tmp= F::nCr(d + 1, i + 1) * pw[d - i] * y[i];
  ans+= (d - i) & 1 ? -tmp : tmp;
 }
 ans/= Mint(1 - r).pow(d + 1);
 Mint iv= Mint(1) / r, ipw= 1;
 for(int i= 0; i <= d; ++i) y[i]= (y[i] - ans) * ipw, ipw*= iv;
 ans+= Mint(r).pow(n) * sample_points_shift<Mint>(y, n);
 return ans.val();
}
