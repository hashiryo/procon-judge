#pragma once
// 2 bit 刻みの表 32 本 (1 KB)。引いた値を 4 つずつ __m256i に載せて 256 bit の XOR で畳む版。
// XOR の本数は 4 分の 1 になるが、64 bit を 4 つ vector に詰める手間が増える。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_frob2_2bit_avx {
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
 array<array<u64, 4>, 32> t{};
 for(int p= 0; p < 32; ++p)
  for(int j= 0; j < 2; ++j) {
   const u64 v= BASIS[2 * p + j];
   for(int b= 0; b < (1 << j); ++b) t[p][(1 << j) | b]= t[p][b] ^ v;
  }
 return t;
}();
inline u64 frob2(u64 a) {
 __m256i v0= _mm256_set_epi64x(T[3][(a >> 6) & 3], T[2][(a >> 4) & 3], T[1][(a >> 2) & 3], T[0][a & 3]);
 __m256i v1= _mm256_set_epi64x(T[7][(a >> 14) & 3], T[6][(a >> 12) & 3], T[5][(a >> 10) & 3], T[4][(a >> 8) & 3]);
 __m256i v2= _mm256_set_epi64x(T[11][(a >> 22) & 3], T[10][(a >> 20) & 3], T[9][(a >> 18) & 3], T[8][(a >> 16) & 3]);
 __m256i v3= _mm256_set_epi64x(T[15][(a >> 30) & 3], T[14][(a >> 28) & 3], T[13][(a >> 26) & 3], T[12][(a >> 24) & 3]);
 __m256i v4= _mm256_set_epi64x(T[19][(a >> 38) & 3], T[18][(a >> 36) & 3], T[17][(a >> 34) & 3], T[16][(a >> 32) & 3]);
 __m256i v5= _mm256_set_epi64x(T[23][(a >> 46) & 3], T[22][(a >> 44) & 3], T[21][(a >> 42) & 3], T[20][(a >> 40) & 3]);
 __m256i v6= _mm256_set_epi64x(T[27][(a >> 54) & 3], T[26][(a >> 52) & 3], T[25][(a >> 50) & 3], T[24][(a >> 48) & 3]);
 __m256i v7= _mm256_set_epi64x(T[31][(a >> 62) & 3], T[30][(a >> 60) & 3], T[29][(a >> 58) & 3], T[28][(a >> 56) & 3]);
 __m256i w0= _mm256_xor_si256(v0, v1);
 __m256i w1= _mm256_xor_si256(v2, v3);
 __m256i w2= _mm256_xor_si256(v4, v5);
 __m256i w3= _mm256_xor_si256(v6, v7);
 __m256i w4= _mm256_xor_si256(w0, w1);
 __m256i w5= _mm256_xor_si256(w2, w3);
 __m256i w6= _mm256_xor_si256(w4, w5);
 __m128i y= _mm_xor_si128(_mm256_castsi256_si128(w6), _mm256_extracti128_si256(w6, 1));
 return u64(_mm_cvtsi128_si64(y)) ^ u64(_mm_extract_epi64(y, 1));
}
}  // namespace gf2_64_frob2_2bit_avx
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using gf2_64_frob2_2bit_avx::frob2;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= frob2(as[i]);
  return ans;
 }
};
