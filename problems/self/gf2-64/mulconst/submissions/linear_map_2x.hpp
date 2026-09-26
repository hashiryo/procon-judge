#pragma once
// 定数倍用の [8][256] の表を使い、__m128i の 2 つの lane に別々の入力を割り当てて
// 2 つ同時に計算する版。linear_map_sse と違って XOR の本数は 1 入力あたり 7 本のままだが、
// 最後に 128 bit を 64 bit へ畳む手間が要らず、取り出しがそのまま答えになる。
#pragma GCC optimize("O3,unroll-loops")
#include "common.hpp"
// 定数倍は F_2-線型写像なので、frob と同じ 8 byte tables (16 KB) に焼ける。
// 基底の像 BASIS[i] = MUL_CONST * x^i は、x 倍 (shift + reduce) を 64 回繰り返すだけで出る。
namespace gf2_64_mulconst_linear_map_2x {
constexpr auto MUL_BYTE= []() {
 u64 basis[64]{};
 basis[0]= MUL_CONST;
 for(int i= 1; i < 64; ++i) basis[i]= (basis[i - 1] << 1) ^ (IRRED_LOW & -(basis[i - 1] >> 63));
 array<array<u64, 256>, 8> t{};
 for(int p= 0; p < 8; ++p)
  for(int j= 0; j < 8; ++j) {
   const u64 v= basis[8 * p + j];
   for(int b= 0; b < (1 << j); ++b) t[p][(1 << j) | b]= t[p][b] ^ v;
  }
 return t;
}();
inline __m128i mulc2(u64 a0, u64 a1) {
 __m128i x= _mm_set_epi64x(MUL_BYTE[0][u8(a1)], MUL_BYTE[0][u8(a0)]);
 for(int p= 1; p < 8; ++p) x= _mm_xor_si128(x, _mm_set_epi64x(MUL_BYTE[p][u8(a1 >> (8 * p))], MUL_BYTE[p][u8(a0 >> (8 * p))]));
 return x;
}
}  // namespace gf2_64_mulconst_linear_map_2x
inline vector<u64> run(const vector<u64>& as) {
 using gf2_64_mulconst_linear_map_2x::MUL_BYTE;
 using gf2_64_mulconst_linear_map_2x::mulc2;
 const size_t n= as.size();
 vector<u64> ans(n);
 size_t i= 0;
 for(; i + 1 < n; i+= 2) {
  const __m128i y= mulc2(as[i], as[i + 1]);
  ans[i]= u64(_mm_cvtsi128_si64(y));
  ans[i + 1]= u64(_mm_extract_epi64(y, 1));
 }
 if(i < n) {
  const u64 a= as[i];
  ans[i]= MUL_BYTE[0][u8(a)] ^ MUL_BYTE[1][u8(a >> 8)] ^ MUL_BYTE[2][u8(a >> 16)] ^ MUL_BYTE[3][u8(a >> 24)] ^ MUL_BYTE[4][u8(a >> 32)] ^ MUL_BYTE[5][u8(a >> 40)] ^ MUL_BYTE[6][u8(a >> 48)] ^ MUL_BYTE[7][u8(a >> 56)];
 }
 return ans;
}
