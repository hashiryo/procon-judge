#pragma once
// Frobenius byte-table linear map:
//   sq: F_{2^64} → F_{2^64} は F_2-線型写像。 入力を 8 byte に分けて、 各 byte
//   位置 (0..7) ごとに 256-entry の lookup table を precompute (= 2 KB / table)。
//   sq(a) = XOR_{p=0..7} FROB1_BYTE[p][(a >> 8p) & 0xFF]
//
//   テーブル構築には素朴 sq (reference 同様) を一度実行。 init は O(2K mul) で軽い。
//
// 期待: 8 lookup + 8 XOR ≈ 8-12 cyc。 PDEP 並みかやや速い可能性。 pclmul に依存しない
//   ので非対応環境でも高速。 cache footprint は 16 KB で L1 fit。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_sq_frobenius_byte {
constexpr u64 sq_naive(u64 a) {
 u64 lo= 0, hi= 0;
 for(int i= 0; i < 64; ++i) {
  if((a >> i) & 1) {
   int j= 2 * i;
   if(j < 64) lo^= u64(1) << j;
   else hi^= u64(1) << (j - 64);
  }
 }
 for(int i= 63; i >= 0; --i) {
  if((hi >> i) & 1) {
   hi^= u64(1) << i;
   lo^= IRRED_LOW << i;
   if(i > 0) hi^= IRRED_LOW >> (64 - i);
  }
 }
 return lo;
}
constexpr array<array<u64, 256>, 8> FROB1_BYTE= [] {
 array<array<u64, 256>, 8> t{};
 for(int p= 0; p < 8; ++p)
  for(int b= 0; b < 256; ++b) t[p][b]= sq_naive(u64(b) << (8 * p));
 return t;
}();
inline u64 sq(u64 a) {
 __m128i v0= _mm_set_epi64x(FROB1_BYTE[1][u8(a >> 8)], FROB1_BYTE[0][u8(a)]);
 __m128i v1= _mm_set_epi64x(FROB1_BYTE[3][u8(a >> 24)], FROB1_BYTE[2][u8(a >> 16)]);
 __m128i v2= _mm_set_epi64x(FROB1_BYTE[5][u8(a >> 40)], FROB1_BYTE[4][u8(a >> 32)]);
 __m128i v3= _mm_set_epi64x(FROB1_BYTE[7][u8(a >> 56)], FROB1_BYTE[6][u8(a >> 48)]);
 __m128i y= _mm_xor_si128(_mm_xor_si128(v0, v1), _mm_xor_si128(v2, v3));
 return u64(_mm_cvtsi128_si64(y)) ^ u64(_mm_extract_epi64(y, 1));
}
}
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using namespace gf2_64_sq_frobenius_byte;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= sq(as[i]);
  return ans;
 }
};
