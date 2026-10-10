#pragma once
#include "../common.hpp"
// zmod_naive (ZMod の演算子で i-k-j) が今の Library の Matrix より遅い理由を切り分ける診断用。ZMod を通さずに u32 の配列で、
// 同じ i-k-j のループに、ZMod と同じ式の Barrett と、今の Library の MP_Mo32 と同じ式の Montgomery を入れる。
// どちらも値は 2M 未満で持ち、足し算は 2M 以上なら 2M を引く。
struct DiagBarrett {
 static constexpr u64 X= u64(-1) / MOD;
 static constexpr u32 set(u32 x) { return x; }
 static constexpr u32 get(u32 x) { return x >= MOD ? x - MOD : x; }
 static constexpr u32 mul(u32 a, u32 b) {
  const u64 t= u64(a) * b;
  return u32(t - u64((u128(t) * X) >> 64) * MOD);
 }
};
struct DiagMont {
 static constexpr u32 IV= [] {
  u32 x= 1;
  for(int i= 0; i < 6; ++i) x*= 2 - x * MOD;
  return x;
 }();
 static constexpr u32 R2= u32(-u64(MOD) % MOD);
 static constexpr u32 reduce(u64 w) { return u32(w >> 32) + MOD - u32((u64(u32(w) * IV) * MOD) >> 32); }
 static constexpr u32 mul(u32 a, u32 b) { return reduce(u64(a) * b); }
 static constexpr u32 set(u32 x) { return mul(x, R2); }
 static constexpr u32 get(u32 x) {
  x= reduce(x);
  return x >= MOD ? x - MOD : x;
 }
};
template <class R> inline vector<u32> run_diag(int n, int m, int p, const vector<u32>& a, const vector<u32>& b) {
 vector<u32> A(a.size()), B(b.size()), C((size_t)n * p);
 for(size_t i= 0; i < a.size(); ++i) A[i]= R::set(a[i]);
 for(size_t i= 0; i < b.size(); ++i) B[i]= R::set(b[i]);
 for(int i= 0; i < n; ++i) {
  u32* c= &C[(size_t)i * p];
  for(int k= 0; k < m; ++k) {
   const u32 x= A[(size_t)i * m + k];
   const u32* y= &B[(size_t)k * p];
   for(int j= 0; j < p; ++j) {
    u32 s= c[j] + R::mul(x, y[j]);
    c[j]= s >= 2 * MOD ? s - 2 * MOD : s;
   }
  }
 }
 for(auto& x : C) x= R::get(x);
 return C;
}
