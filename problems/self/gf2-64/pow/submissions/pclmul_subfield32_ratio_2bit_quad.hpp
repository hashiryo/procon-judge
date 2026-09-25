#pragma once
// a^e を e = q (2^32+1) + r と割り、b = a^(2^32+1) ∈ F_2^32 の側を log / pow 表で、
// r の側を窓で回す版。既存の pclmul_subfield_split 系は (2^64-1)/(2^16-1) で割って
// q < 2^16 だけを F_2^16 の表引きにし、残り 48 bit を窓で回していたが、F_2^32 まで
// 落とせば表引きが q < 2^32 を引き受けるので窓は 32 bit で済む。
//
// log の引き方 (_subfield32_ratio.hpp): b の F_2^16 座標 (b_0, b_1) の比で類を決めて引く (s を作らない)
// pow 表 (_subfield32.hpp): 2 段の pow 表 (257 + 256 要素, 4 KB) + mul2 1 本
// 窓 (_win32.hpp): 2 bit × 16 桁。葉の frob8 / frob16 / frob24 を 4 本の基底の冪表に置き換える
//
// r は 2^32 も取り得るが、そのときだけ a^(2^32) = frob32(a) を後から掛ける。
//
// 必要な拡張: VPCLMULQDQ + AVX2 (Intel Ice Lake / AMD Zen3 以降)。
#pragma GCC optimize("O3,unroll-loops")
#include "_subfield32_ratio.hpp"
#include "_win32.hpp"
namespace gf2_64_pow_subfield32_ratio_2bit_quad {
using namespace gf2_64_pow_subfield32;
using gf2_64_pclmul::frob32;
u64 pow(u64 a, u64 e) {
 if(!e) return 1;
 if(!a) return 0;
 const u32 q= u32(e / SPLIT);
 const u64 r= e - u64(q) * SPLIT;
 const u64 fa= frob32(a);
 auto [mp, mn]= subfield_exp2(mul(a, fa), q);  // (a^(2^32+1))^q = h_P^mp · h_N^mn
 auto [zp, zn]= pow_pair(mp, mn);
 auto [w0, w1]= win32_2bit_quad(a, u32(r));          // w0 w1 = a^(r mod 2^32)
 auto [x, y]= unpack(mul2(_mm256_set_epi64x(0, w0, 0, zp), _mm256_set_epi64x(0, w1, 0, zn)));
 const u64 res= mul(x, y);
 return r >> 32 ? mul(res, fa) : res;  // r = 2^32 のときだけ a^(2^32) が余る
}
}  // namespace gf2_64_pow_subfield32_ratio_2bit_quad
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as, const vector<u64>& es) {
  using gf2_64_pow_subfield32_ratio_2bit_quad::pow;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= pow(as[i], es[i]);
  return ans;
 }
};
