#pragma once
#pragma GCC optimize("O3,unroll-loops")
#include "../../_shared/gf2-64/_common.hpp"
namespace gf2_64_mul_pclmul_baseline {
PCLMUL inline u64 mul(u64 a, u64 b) {
 __m128i v= _mm_clmulepi64_si128(_mm_cvtsi64_si128(a), _mm_cvtsi64_si128(b), 0);
 b = (u64)_mm_clmulepi64_si128(v, _mm_cvtsi64_si128(0b11011), 1)[0];
 a = (u64)v[0] ^ ((u8[]){0, 27, 45, 54, 90, 65, 119, 108})[((u64)v[1]) >> 60];
 return a^b;
}
}
struct GF2_64Op {
 PCLMUL static vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
  using gf2_64_mul_pclmul_baseline::mul;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= mul(as[i], bs[i]);
  return ans;
 }
};
