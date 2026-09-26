#pragma once
// 定数倍用の [8][256] の表を使い、__m256i の 4 つの lane に別々の入力を割り当てて
// 4 つ同時に計算する版 (linear_map_2x の lane を 4 つにしたもの)。XOR は 256 bit の 7 本で
// 4 入力ぶんが済む。4 つの答えは入力と同じ順に lane に並ぶので、取り出さずに ans へ
// 1 回で書き込む。
#pragma GCC optimize("O3,unroll-loops")
#include "common.hpp"
// 定数倍は F_2-線型写像なので、frob と同じ 8 byte tables (16 KB) に焼ける。
// 基底の像 BASIS[i] = MUL_CONST * x^i は、x 倍 (shift + reduce) を 64 回繰り返すだけで出る。
namespace gf2_64_mulconst_linear_map_4x {
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
inline __m256i mulc4(u64 a0, u64 a1, u64 a2, u64 a3) {
 __m256i x= _mm256_set_epi64x(MUL_BYTE[0][u8(a3)], MUL_BYTE[0][u8(a2)], MUL_BYTE[0][u8(a1)], MUL_BYTE[0][u8(a0)]);
 for(int p= 1; p < 8; ++p) x= _mm256_xor_si256(x, _mm256_set_epi64x(MUL_BYTE[p][u8(a3 >> (8 * p))], MUL_BYTE[p][u8(a2 >> (8 * p))], MUL_BYTE[p][u8(a1 >> (8 * p))], MUL_BYTE[p][u8(a0 >> (8 * p))]));
 return x;
}
}  // namespace gf2_64_mulconst_linear_map_4x
inline vector<u64> run(const vector<u64>& as) {
 using gf2_64_mulconst_linear_map_4x::MUL_BYTE;
 using gf2_64_mulconst_linear_map_4x::mulc4;
 const size_t n= as.size();
 vector<u64> ans(n);
 size_t i= 0;
 for(; i + 3 < n; i+= 4) _mm256_storeu_si256((__m256i*)&ans[i], mulc4(as[i], as[i + 1], as[i + 2], as[i + 3]));
 for(; i < n; ++i) {
  const u64 a= as[i];
  ans[i]= MUL_BYTE[0][u8(a)] ^ MUL_BYTE[1][u8(a >> 8)] ^ MUL_BYTE[2][u8(a >> 16)] ^ MUL_BYTE[3][u8(a >> 24)] ^ MUL_BYTE[4][u8(a >> 32)] ^ MUL_BYTE[5][u8(a >> 40)] ^ MUL_BYTE[6][u8(a >> 48)] ^ MUL_BYTE[7][u8(a >> 56)];
 }
 return ans;
}
