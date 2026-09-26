#pragma once
// 定数倍用の [8][256] の表 (16 KB) を焼いて、64 bit の XOR で畳む版。
#pragma GCC optimize("O3,unroll-loops")
#include "common.hpp"
// 定数倍は F_2-線型写像なので、frob と同じ 8 byte tables (16 KB) に焼ける。
// 基底の像 BASIS[i] = MUL_CONST * x^i は、x 倍 (shift + reduce) を 64 回繰り返すだけで出る。
namespace gf2_64_mulconst_linear_map {
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
inline u64 mulc(u64 a) {
 return MUL_BYTE[0][u8(a)] ^ MUL_BYTE[1][u8(a >> 8)] ^ MUL_BYTE[2][u8(a >> 16)] ^ MUL_BYTE[3][u8(a >> 24)] ^ MUL_BYTE[4][u8(a >> 32)] ^ MUL_BYTE[5][u8(a >> 40)] ^ MUL_BYTE[6][u8(a >> 48)] ^ MUL_BYTE[7][u8(a >> 56)];
}
}  // namespace gf2_64_mulconst_linear_map
inline vector<u64> run(const vector<u64>& as) {
 using gf2_64_mulconst_linear_map::mulc;
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= mulc(as[i]);
 return ans;
}
