#pragma once
// 1 bit 刻みの表 64 本 (1 KB)。XOR を __m128i で 2 つずつ畳む版。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_frob2_1bit_sse {
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
 array<array<u64, 2>, 64> t{};
 for(int p= 0; p < 64; ++p)
  for(int j= 0; j < 1; ++j) {
   const u64 v= BASIS[1 * p + j];
   for(int b= 0; b < (1 << j); ++b) t[p][(1 << j) | b]= t[p][b] ^ v;
  }
 return t;
}();
inline u64 frob2(u64 a) {
 __m128i v0= _mm_set_epi64x(T[1][(a >> 1) & 1], T[0][a & 1]);
 __m128i v1= _mm_set_epi64x(T[3][(a >> 3) & 1], T[2][(a >> 2) & 1]);
 __m128i v2= _mm_set_epi64x(T[5][(a >> 5) & 1], T[4][(a >> 4) & 1]);
 __m128i v3= _mm_set_epi64x(T[7][(a >> 7) & 1], T[6][(a >> 6) & 1]);
 __m128i v4= _mm_set_epi64x(T[9][(a >> 9) & 1], T[8][(a >> 8) & 1]);
 __m128i v5= _mm_set_epi64x(T[11][(a >> 11) & 1], T[10][(a >> 10) & 1]);
 __m128i v6= _mm_set_epi64x(T[13][(a >> 13) & 1], T[12][(a >> 12) & 1]);
 __m128i v7= _mm_set_epi64x(T[15][(a >> 15) & 1], T[14][(a >> 14) & 1]);
 __m128i v8= _mm_set_epi64x(T[17][(a >> 17) & 1], T[16][(a >> 16) & 1]);
 __m128i v9= _mm_set_epi64x(T[19][(a >> 19) & 1], T[18][(a >> 18) & 1]);
 __m128i v10= _mm_set_epi64x(T[21][(a >> 21) & 1], T[20][(a >> 20) & 1]);
 __m128i v11= _mm_set_epi64x(T[23][(a >> 23) & 1], T[22][(a >> 22) & 1]);
 __m128i v12= _mm_set_epi64x(T[25][(a >> 25) & 1], T[24][(a >> 24) & 1]);
 __m128i v13= _mm_set_epi64x(T[27][(a >> 27) & 1], T[26][(a >> 26) & 1]);
 __m128i v14= _mm_set_epi64x(T[29][(a >> 29) & 1], T[28][(a >> 28) & 1]);
 __m128i v15= _mm_set_epi64x(T[31][(a >> 31) & 1], T[30][(a >> 30) & 1]);
 __m128i v16= _mm_set_epi64x(T[33][(a >> 33) & 1], T[32][(a >> 32) & 1]);
 __m128i v17= _mm_set_epi64x(T[35][(a >> 35) & 1], T[34][(a >> 34) & 1]);
 __m128i v18= _mm_set_epi64x(T[37][(a >> 37) & 1], T[36][(a >> 36) & 1]);
 __m128i v19= _mm_set_epi64x(T[39][(a >> 39) & 1], T[38][(a >> 38) & 1]);
 __m128i v20= _mm_set_epi64x(T[41][(a >> 41) & 1], T[40][(a >> 40) & 1]);
 __m128i v21= _mm_set_epi64x(T[43][(a >> 43) & 1], T[42][(a >> 42) & 1]);
 __m128i v22= _mm_set_epi64x(T[45][(a >> 45) & 1], T[44][(a >> 44) & 1]);
 __m128i v23= _mm_set_epi64x(T[47][(a >> 47) & 1], T[46][(a >> 46) & 1]);
 __m128i v24= _mm_set_epi64x(T[49][(a >> 49) & 1], T[48][(a >> 48) & 1]);
 __m128i v25= _mm_set_epi64x(T[51][(a >> 51) & 1], T[50][(a >> 50) & 1]);
 __m128i v26= _mm_set_epi64x(T[53][(a >> 53) & 1], T[52][(a >> 52) & 1]);
 __m128i v27= _mm_set_epi64x(T[55][(a >> 55) & 1], T[54][(a >> 54) & 1]);
 __m128i v28= _mm_set_epi64x(T[57][(a >> 57) & 1], T[56][(a >> 56) & 1]);
 __m128i v29= _mm_set_epi64x(T[59][(a >> 59) & 1], T[58][(a >> 58) & 1]);
 __m128i v30= _mm_set_epi64x(T[61][(a >> 61) & 1], T[60][(a >> 60) & 1]);
 __m128i v31= _mm_set_epi64x(T[63][(a >> 63) & 1], T[62][(a >> 62) & 1]);
 __m128i w0= _mm_xor_si128(v0, v1);
 __m128i w1= _mm_xor_si128(v2, v3);
 __m128i w2= _mm_xor_si128(v4, v5);
 __m128i w3= _mm_xor_si128(v6, v7);
 __m128i w4= _mm_xor_si128(v8, v9);
 __m128i w5= _mm_xor_si128(v10, v11);
 __m128i w6= _mm_xor_si128(v12, v13);
 __m128i w7= _mm_xor_si128(v14, v15);
 __m128i w8= _mm_xor_si128(v16, v17);
 __m128i w9= _mm_xor_si128(v18, v19);
 __m128i w10= _mm_xor_si128(v20, v21);
 __m128i w11= _mm_xor_si128(v22, v23);
 __m128i w12= _mm_xor_si128(v24, v25);
 __m128i w13= _mm_xor_si128(v26, v27);
 __m128i w14= _mm_xor_si128(v28, v29);
 __m128i w15= _mm_xor_si128(v30, v31);
 __m128i w16= _mm_xor_si128(w0, w1);
 __m128i w17= _mm_xor_si128(w2, w3);
 __m128i w18= _mm_xor_si128(w4, w5);
 __m128i w19= _mm_xor_si128(w6, w7);
 __m128i w20= _mm_xor_si128(w8, w9);
 __m128i w21= _mm_xor_si128(w10, w11);
 __m128i w22= _mm_xor_si128(w12, w13);
 __m128i w23= _mm_xor_si128(w14, w15);
 __m128i w24= _mm_xor_si128(w16, w17);
 __m128i w25= _mm_xor_si128(w18, w19);
 __m128i w26= _mm_xor_si128(w20, w21);
 __m128i w27= _mm_xor_si128(w22, w23);
 __m128i w28= _mm_xor_si128(w24, w25);
 __m128i w29= _mm_xor_si128(w26, w27);
 __m128i w30= _mm_xor_si128(w28, w29);
 return u64(_mm_cvtsi128_si64(w30)) ^ u64(_mm_extract_epi64(w30, 1));
}
}  // namespace gf2_64_frob2_1bit_sse
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using gf2_64_frob2_1bit_sse::frob2;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= frob2(as[i]);
  return ans;
 }
};
