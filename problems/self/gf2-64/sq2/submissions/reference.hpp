#pragma once
// 素朴 reference (golden): 各 lane を取り出し、bit ごとの clmul と長除法の reduce で a·a を作る。pclmul も
// pshufb も使わない。gf2-64-mul2 の reference と同じ計算。
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_ref {
inline std::pair<u64, u64> clmul_loop(u64 a, u64 b) {
 u64 lo= 0, hi= 0;
 for(int i= 0; i < 64; ++i) {
  if((b >> i) & 1) {
   lo^= a << i;
   if(i) hi^= a >> (64 - i);
  }
 }
 return {lo, hi};
}
inline u64 reduce_naive(u64 lo, u64 hi) {
 for(int i= 63; i >= 0; --i) {
  if((hi >> i) & 1) {
   hi^= u64(1) << i;
   lo^= IRRED_LOW << i;
   if(i > 0) hi^= IRRED_LOW >> (64 - i);
  }
 }
 return lo;
}
inline u64 sq(u64 a) {
 auto [lo, hi]= clmul_loop(a, a);
 return reduce_naive(lo, hi);
}
}  // namespace gf2_64_ref
inline __m256i sq2(__m256i v) {
 const u64 a= u64(_mm256_extract_epi64(v, 0)), b= u64(_mm256_extract_epi64(v, 2));
 return _mm256_set_epi64x(0, (long long)gf2_64_ref::sq(b), 0, (long long)gf2_64_ref::sq(a));
}
