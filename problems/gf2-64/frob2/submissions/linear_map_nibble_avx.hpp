#pragma once
// 4 bit 刻みの表 16 本 (2 KB)。引いた値を 4 つずつ __m256i に載せて 256 bit の XOR で畳む版。
// XOR の本数は 4 分の 1 になるが、64 bit を 4 つ vector に詰める手間が増える。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_frob2_nibble_avx {
// 基底の像 BASIS[i] = frob2(1 << i)。sq を呼ぶのはここだけで、表はこの XOR の
// 足し合わせ (倍々) で広げる。表の形が変わっても作り方は同じ。
constexpr auto BASIS= []() {
 auto spread= [](u32 a) constexpr -> u64 {
  u64 x= a;
  x= (x | (x << 16)) & 0x0000FFFF0000FFFFull;
  x= (x | (x << 8)) & 0x00FF00FF00FF00FFull;
  x= (x | (x << 4)) & 0x0F0F0F0F0F0F0F0Full;
  x= (x | (x << 2)) & 0x3333333333333333ull;
  x= (x | (x << 1)) & 0x5555555555555555ull;
  return x;
 };
 constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 auto sq= [&](u64 a) constexpr -> u64 {
  u64 h= spread(u32(a >> 32));
  u64 d= h ^ (h << 1);
  return spread(u32(a)) ^ RED[h >> 60] ^ d ^ (d << 3);
 };
 array<u64, 64> b{};
 for(int i= 0; i < 64; ++i) b[i]= sq(sq(u64(1) << i));
 return b;
}();
constexpr auto T= []() {
 array<array<u64, 16>, 16> t{};
 for(int p= 0; p < 16; ++p)
  for(int j= 0; j < 4; ++j) {
   const u64 v= BASIS[4 * p + j];
   for(int b= 0; b < (1 << j); ++b) t[p][(1 << j) | b]= t[p][b] ^ v;
  }
 return t;
}();
inline u64 frob2(u64 a) {
 __m256i v0= _mm256_set_epi64x(T[3][(a >> 12) & 15], T[2][(a >> 8) & 15], T[1][(a >> 4) & 15], T[0][a & 15]);
 __m256i v1= _mm256_set_epi64x(T[7][(a >> 28) & 15], T[6][(a >> 24) & 15], T[5][(a >> 20) & 15], T[4][(a >> 16) & 15]);
 __m256i v2= _mm256_set_epi64x(T[11][(a >> 44) & 15], T[10][(a >> 40) & 15], T[9][(a >> 36) & 15], T[8][(a >> 32) & 15]);
 __m256i v3= _mm256_set_epi64x(T[15][(a >> 60) & 15], T[14][(a >> 56) & 15], T[13][(a >> 52) & 15], T[12][(a >> 48) & 15]);
 __m256i w0= _mm256_xor_si256(v0, v1);
 __m256i w1= _mm256_xor_si256(v2, v3);
 __m256i w2= _mm256_xor_si256(w0, w1);
 __m128i y= _mm_xor_si128(_mm256_castsi256_si128(w2), _mm256_extracti128_si256(w2, 1));
 return u64(_mm_cvtsi128_si64(y)) ^ u64(_mm_extract_epi64(y, 1));
}
}  // namespace gf2_64_frob2_nibble_avx
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using gf2_64_frob2_nibble_avx::frob2;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= frob2(as[i]);
  return ans;
 }
};
