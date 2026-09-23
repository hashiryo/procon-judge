#pragma once
// PDEP-based squaring:
//   GF(2) では (sum b_i x^i)^2 = sum b_i x^{2i} (cross term ゼロ)。
//   bit i → bit 2i の spread は PDEP で 1 命令:
//     lo_poly = pdep(a & 0xFFFFFFFF, 0x5555...5555)  ← bit 0..31 → 0,2,..,62
//     hi_poly = pdep(a >> 32, 0x5555...5555)         ← bit 32..63 → 0,2,..,62 (= 64,66,..,126)
//   その後 P(x) で reduce (= 標準 reduction)。
//
// BMI2 が無い環境 (古い x86, ARM) ではビット並びを spread する loop で fallback。
//
// GNU_TARGET("pclmul") 比較:
//   - GNU_TARGET("pclmul") sq: 1 GNU_TARGET("pclmul") (~5 cyc) + reduce
//   - PDEP sq:   2 PDEP    (~3 cyc each, 並列) + reduce
//   GNU_TARGET("pclmul") はレイテンシ長め、 PDEP は並列性高い。 throughput は PDEP がやや勝つ可能性
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_sq_pdep {
inline u64 spread_bits(u32 a) {
#ifdef __BMI2__
 return _pdep_u64(u64(a), 0x5555555555555555ull);
#else
 // fallback: bit interleave for 32-bit input
 u64 x= a;
 x= (x | (x << 16)) & 0x0000FFFF0000FFFFull;
 x= (x | (x << 8)) & 0x00FF00FF00FF00FFull;
 x= (x | (x << 4)) & 0x0F0F0F0F0F0F0F0Full;
 x= (x | (x << 2)) & 0x3333333333333333ull;
 x= (x | (x << 1)) & 0x5555555555555555ull;
 return x;
#endif
}
GNU_TARGET("pclmul") inline u64 sq(u64 a) {
 u64 h= spread_bits(u32(a >> 32));
 u64 b= (u64)_mm_clmulepi64_si128(_mm_cvtsi64_si128(h), _mm_cvtsi64_si128(0b11011), 0)[0];
 a= spread_bits(u32(a)) ^ ((u8[]){0, 27, 45, 54, 90, 65, 119, 108})[h >> 60];
 return a ^ b;
}
}
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using gf2_64_sq_pdep::sq;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= sq(as[i]);
  return ans;
 }
};
