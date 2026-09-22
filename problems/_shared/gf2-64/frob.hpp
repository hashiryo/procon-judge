#pragma once
// GF(2^64) の Frobenius^k (= a^{2^k}) を byte table で計算する current best 実装。
//
// 命名規約: frobK の K は **sq の適用回数** (= 結果は a^{2^K} に等しい)。
//   frob1 (= sq, _shared/sq.hpp で提供)
//   frob2 (= a^4)
//   frob4 (= a^16)
//   frob8 (= a^256)
//   frob16 (= a^65536)
//
// 各 frobK は F_2-線型写像なので 8 byte tables (= 16 KB / table) で 8 lookup + 7 XOR。
// テーブルは constexpr で compile-time に作られる (PDEP は使わず bit-interleave で
// 構成して constexpr 評価可)。
//
// 表の作り方: frobK は線型なので、基底 e_0..e_63 の像 64 個が決まれば 2048 通りの entry は
// その XOR で出る。そして frob(K+L) = frobK ∘ frobL なので、欲しい K は既にある 2 つの段の
// 足し算で作れる (48 = 32 + 16 など)。64 個の基底を上の段の表で 1 回引き直すだけで次の段が
// 出るので、1 つの段あたり 64 回の lookup と 2048 回の XOR しか要らない。
// 2048 通りを K 回ずつ sq していた頃と比べて sq の呼び出しは 64 回だけになり、
// constexpr 変数 1 つあたりの step も 1 桁減って既定の上限に十分な余裕ができる。
//
// 利用側ルール:
//   gf2-64-frob{2,4,8,16}/algos/* は本ファイルを使ってはいけない (frobK の
//     比較対象なので)。
//   それ以外の problem (div / pow / sqrt / log) は building block として OK。
//   K=1 (= sq) は _shared/sq.hpp が PDEP で提供。 ここには無い。
//
// Itoh-Tsujii で必要な K = 1, 2, 4, 8, 16 を全カバー。
#include "_common.hpp"
namespace gf2_64_pclmul {
namespace _frob_detail {
constexpr u8 RED_TABLE[]= {0, 27, 45, 54, 90, 65, 119, 108};
constexpr u64 spread_constexpr(u32 a) {
 u64 x= a;
 x= (x | (x << 16)) & 0x0000FFFF0000FFFFull;
 x= (x | (x << 8)) & 0x00FF00FF00FF00FFull;
 x= (x | (x << 4)) & 0x0F0F0F0F0F0F0F0Full;
 x= (x | (x << 2)) & 0x3333333333333333ull;
 x= (x | (x << 1)) & 0x5555555555555555ull;
 return x;
}
constexpr u64 sq_constexpr(u64 a) {
 u64 h= spread_constexpr(u32(a >> 32));
 u64 d= h ^ (h << 1);
 return spread_constexpr(u32(a)) ^ RED_TABLE[h >> 60] ^ d ^ (d << 3);
}
using Basis= array<u64, 64>;             // basis[i] = frobK(1 << i)
using Table= array<array<u64, 256>, 8>;  // table[p][b] = frobK(b << 8p)
// 基底の像から 2048 通りを埋める。bit を下から 1 本ずつ足して、出来ている半分から倍々に伸ばす。
constexpr Table expand(const Basis& g) {
 Table t{};
 for(int p= 0; p < 8; ++p)
  for(int j= 0; j < 8; ++j) {
   const u64 v= g[8 * p + j];
   const int half= 1 << j;
   for(int b= 0; b < half; ++b) t[p][half + b]= t[p][b] ^ v;
  }
 return t;
}
constexpr u64 apply(const Table& t, u64 a) { return t[0][u8(a)] ^ t[1][u8(a >> 8)] ^ t[2][u8(a >> 16)] ^ t[3][u8(a >> 24)] ^ t[4][u8(a >> 32)] ^ t[5][u8(a >> 40)] ^ t[6][u8(a >> 48)] ^ t[7][u8(a >> 56)]; }
struct Frob {
 Basis g;
 Table t;
};
// frob(K+L) の基底 = frobK を frobL の基底 64 個に掛けたもの。上の段の sq をやり直さない。
constexpr Frob compose(const Frob& hi, const Frob& lo) {
 Basis g{};
 for(int i= 0; i < 64; ++i) g[i]= apply(hi.t, lo.g[i]);
 return {g, expand(g)};
}
// 唯一 sq から作る段。ここだけ 64 回 sq を呼ぶ。
constexpr Frob make_frob1() {
 Basis g{};
 for(int i= 0; i < 64; ++i) g[i]= sq_constexpr(u64(1) << i);
 return {g, expand(g)};
}
// 右辺の足し算がそのまま K (= sq の適用回数) の足し算。欲しい段だけを並べてある。
constexpr Frob F1= make_frob1();
constexpr Frob F2= compose(F1, F1);
constexpr Frob F3= compose(F2, F1);
constexpr Frob F4= compose(F2, F2);
constexpr Frob F5= compose(F4, F1);
constexpr Frob F6= compose(F4, F2);
constexpr Frob F7= compose(F4, F3);
constexpr Frob F8= compose(F4, F4);
constexpr Frob F9= compose(F8, F1);
constexpr Frob F10= compose(F8, F2);
constexpr Frob F12= compose(F8, F4);
constexpr Frob F16= compose(F8, F8);
constexpr Frob F24= compose(F16, F8);
constexpr Frob F32= compose(F16, F16);
constexpr Frob F36= compose(F32, F4);
constexpr Frob F48= compose(F32, F16);
}  // namespace _frob_detail
// FROBK_BYTE[p][b] = frobK(b << 8p)。 K = sq 適用回数。
inline constexpr auto FROB2_BYTE= _frob_detail::F2.t;
inline constexpr auto FROB3_BYTE= _frob_detail::F3.t;
inline constexpr auto FROB4_BYTE= _frob_detail::F4.t;
inline constexpr auto FROB5_BYTE= _frob_detail::F5.t;
inline constexpr auto FROB6_BYTE= _frob_detail::F6.t;
inline constexpr auto FROB7_BYTE= _frob_detail::F7.t;
inline constexpr auto FROB8_BYTE= _frob_detail::F8.t;
inline constexpr auto FROB9_BYTE= _frob_detail::F9.t;
inline constexpr auto FROB10_BYTE= _frob_detail::F10.t;
inline constexpr auto FROB12_BYTE= _frob_detail::F12.t;  // pow 4-lane 分割 (12 bit lane) の結合用
inline constexpr auto FROB16_BYTE= _frob_detail::F16.t;
inline constexpr auto FROB24_BYTE= _frob_detail::F24.t;  // pow 2-lane 分割 (a^r = frob24(a^{r_H}) · a^{r_L}) の結合用
inline constexpr auto FROB32_BYTE= _frob_detail::F32.t;  // norm decomposition (α + α^{2^16} + α^{2^32} + α^{2^48}) 用
inline constexpr auto FROB36_BYTE= _frob_detail::F36.t;  // pow 4-lane 分割の結合用
inline constexpr auto FROB48_BYTE= _frob_detail::F48.t;  // norm decomposition 用
// 各 frobK は対応する byte table を直接展開 (関数引数経由の indirection を避ける)。
inline u64 frob2(u64 a) { return FROB2_BYTE[0][u8(a)] ^ FROB2_BYTE[1][u8(a >> 8)] ^ FROB2_BYTE[2][u8(a >> 16)] ^ FROB2_BYTE[3][u8(a >> 24)] ^ FROB2_BYTE[4][u8(a >> 32)] ^ FROB2_BYTE[5][u8(a >> 40)] ^ FROB2_BYTE[6][u8(a >> 48)] ^ FROB2_BYTE[7][u8(a >> 56)]; }
inline u64 frob3(u64 a) { return FROB3_BYTE[0][u8(a)] ^ FROB3_BYTE[1][u8(a >> 8)] ^ FROB3_BYTE[2][u8(a >> 16)] ^ FROB3_BYTE[3][u8(a >> 24)] ^ FROB3_BYTE[4][u8(a >> 32)] ^ FROB3_BYTE[5][u8(a >> 40)] ^ FROB3_BYTE[6][u8(a >> 48)] ^ FROB3_BYTE[7][u8(a >> 56)]; }
inline u64 frob4(u64 a) { return FROB4_BYTE[0][u8(a)] ^ FROB4_BYTE[1][u8(a >> 8)] ^ FROB4_BYTE[2][u8(a >> 16)] ^ FROB4_BYTE[3][u8(a >> 24)] ^ FROB4_BYTE[4][u8(a >> 32)] ^ FROB4_BYTE[5][u8(a >> 40)] ^ FROB4_BYTE[6][u8(a >> 48)] ^ FROB4_BYTE[7][u8(a >> 56)]; }
inline u64 frob5(u64 a) { return FROB5_BYTE[0][u8(a)] ^ FROB5_BYTE[1][u8(a >> 8)] ^ FROB5_BYTE[2][u8(a >> 16)] ^ FROB5_BYTE[3][u8(a >> 24)] ^ FROB5_BYTE[4][u8(a >> 32)] ^ FROB5_BYTE[5][u8(a >> 40)] ^ FROB5_BYTE[6][u8(a >> 48)] ^ FROB5_BYTE[7][u8(a >> 56)]; }
inline u64 frob6(u64 a) { return FROB6_BYTE[0][u8(a)] ^ FROB6_BYTE[1][u8(a >> 8)] ^ FROB6_BYTE[2][u8(a >> 16)] ^ FROB6_BYTE[3][u8(a >> 24)] ^ FROB6_BYTE[4][u8(a >> 32)] ^ FROB6_BYTE[5][u8(a >> 40)] ^ FROB6_BYTE[6][u8(a >> 48)] ^ FROB6_BYTE[7][u8(a >> 56)]; }
inline u64 frob7(u64 a) { return FROB7_BYTE[0][u8(a)] ^ FROB7_BYTE[1][u8(a >> 8)] ^ FROB7_BYTE[2][u8(a >> 16)] ^ FROB7_BYTE[3][u8(a >> 24)] ^ FROB7_BYTE[4][u8(a >> 32)] ^ FROB7_BYTE[5][u8(a >> 40)] ^ FROB7_BYTE[6][u8(a >> 48)] ^ FROB7_BYTE[7][u8(a >> 56)]; }
inline u64 frob8(u64 a) { return FROB8_BYTE[0][u8(a)] ^ FROB8_BYTE[1][u8(a >> 8)] ^ FROB8_BYTE[2][u8(a >> 16)] ^ FROB8_BYTE[3][u8(a >> 24)] ^ FROB8_BYTE[4][u8(a >> 32)] ^ FROB8_BYTE[5][u8(a >> 40)] ^ FROB8_BYTE[6][u8(a >> 48)] ^ FROB8_BYTE[7][u8(a >> 56)]; }
inline u64 frob9(u64 a) { return FROB9_BYTE[0][u8(a)] ^ FROB9_BYTE[1][u8(a >> 8)] ^ FROB9_BYTE[2][u8(a >> 16)] ^ FROB9_BYTE[3][u8(a >> 24)] ^ FROB9_BYTE[4][u8(a >> 32)] ^ FROB9_BYTE[5][u8(a >> 40)] ^ FROB9_BYTE[6][u8(a >> 48)] ^ FROB9_BYTE[7][u8(a >> 56)]; }
inline u64 frob10(u64 a) { return FROB10_BYTE[0][u8(a)] ^ FROB10_BYTE[1][u8(a >> 8)] ^ FROB10_BYTE[2][u8(a >> 16)] ^ FROB10_BYTE[3][u8(a >> 24)] ^ FROB10_BYTE[4][u8(a >> 32)] ^ FROB10_BYTE[5][u8(a >> 40)] ^ FROB10_BYTE[6][u8(a >> 48)] ^ FROB10_BYTE[7][u8(a >> 56)]; }
inline u64 frob12(u64 a) { return FROB12_BYTE[0][u8(a)] ^ FROB12_BYTE[1][u8(a >> 8)] ^ FROB12_BYTE[2][u8(a >> 16)] ^ FROB12_BYTE[3][u8(a >> 24)] ^ FROB12_BYTE[4][u8(a >> 32)] ^ FROB12_BYTE[5][u8(a >> 40)] ^ FROB12_BYTE[6][u8(a >> 48)] ^ FROB12_BYTE[7][u8(a >> 56)]; }
inline u64 frob16(u64 a) { return FROB16_BYTE[0][u8(a)] ^ FROB16_BYTE[1][u8(a >> 8)] ^ FROB16_BYTE[2][u8(a >> 16)] ^ FROB16_BYTE[3][u8(a >> 24)] ^ FROB16_BYTE[4][u8(a >> 32)] ^ FROB16_BYTE[5][u8(a >> 40)] ^ FROB16_BYTE[6][u8(a >> 48)] ^ FROB16_BYTE[7][u8(a >> 56)]; }
inline u64 frob24(u64 a) { return FROB24_BYTE[0][u8(a)] ^ FROB24_BYTE[1][u8(a >> 8)] ^ FROB24_BYTE[2][u8(a >> 16)] ^ FROB24_BYTE[3][u8(a >> 24)] ^ FROB24_BYTE[4][u8(a >> 32)] ^ FROB24_BYTE[5][u8(a >> 40)] ^ FROB24_BYTE[6][u8(a >> 48)] ^ FROB24_BYTE[7][u8(a >> 56)]; }
inline u64 frob32(u64 a) { return FROB32_BYTE[0][u8(a)] ^ FROB32_BYTE[1][u8(a >> 8)] ^ FROB32_BYTE[2][u8(a >> 16)] ^ FROB32_BYTE[3][u8(a >> 24)] ^ FROB32_BYTE[4][u8(a >> 32)] ^ FROB32_BYTE[5][u8(a >> 40)] ^ FROB32_BYTE[6][u8(a >> 48)] ^ FROB32_BYTE[7][u8(a >> 56)]; }
inline u64 frob36(u64 a) { return FROB36_BYTE[0][u8(a)] ^ FROB36_BYTE[1][u8(a >> 8)] ^ FROB36_BYTE[2][u8(a >> 16)] ^ FROB36_BYTE[3][u8(a >> 24)] ^ FROB36_BYTE[4][u8(a >> 32)] ^ FROB36_BYTE[5][u8(a >> 40)] ^ FROB36_BYTE[6][u8(a >> 48)] ^ FROB36_BYTE[7][u8(a >> 56)]; }
inline u64 frob48(u64 a) { return FROB48_BYTE[0][u8(a)] ^ FROB48_BYTE[1][u8(a >> 8)] ^ FROB48_BYTE[2][u8(a >> 16)] ^ FROB48_BYTE[3][u8(a >> 24)] ^ FROB48_BYTE[4][u8(a >> 32)] ^ FROB48_BYTE[5][u8(a >> 40)] ^ FROB48_BYTE[6][u8(a >> 48)] ^ FROB48_BYTE[7][u8(a >> 56)]; }
}
