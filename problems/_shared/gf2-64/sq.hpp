#pragma once
// GF(2^64) の current best sq: PDEP (BMI2) で bit を spread → P(x) で reduce。
// GF(2) では (Σ a_i x^i)^2 = Σ a_i x^{2i} (cross term ゼロ) なので、 PDEP 1 命令で書ける。
// BMI2 が無い環境では bit interleave fallback。
//
// 利用側ルール: gf2-64-sq/algos/* / gf2-64-mul/algos/* は本ファイルを使ってはいけない。
// それ以外の problem (div/pow/sqrt/log) は building block として使用 OK。
#include "_common.hpp"
namespace gf2_64_pclmul {
const __m128i mask_lo= _mm_set1_epi8(0x0F);
const __m128i spread3_tbl= _mm_setr_epi8(0x00, 0x03, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F, (signed char)0xC0, (signed char)0xC3, (signed char)0xCC, (signed char)0xCF, (signed char)0xF0, (signed char)0xF3, (signed char)0xFC, (signed char)0xFF);
// 下位 64 bit は偶数 bit だけ残し (spread に戻す)、上位 64 bit (d) はそのまま通す
constexpr u8 RED_SQ[4]= {0, 27, 90, 65};
inline u64 sq(u64 a) {
 // spread3_tbl[v] = bit i ∈ v を bit 2i と 2i+1 の両方に展開した 8-bit 値 (= spread_tbl[v] * 3)
 //   v=0x0:0x00, 0x1:0x03, 0x2:0x0C, 0x3:0x0F, 0x4:0x30, ... , 0xF:0xFF
 __m128i v= _mm_set_epi64x(0, a);
 __m128i nib= _mm_and_si128(_mm_unpacklo_epi8(v, _mm_srli_epi16(v, 4)), mask_lo);
 __m128i x= _mm_shuffle_epi8(spread3_tbl, nib);
 // x の下位 64 bit が a の下位 32 bit の平方 (lo)、上位 64 bit が d = h ^ (h << 1) (h は a の上位 32 bit の平方)
 u64 d= x[1];
 return (x[0] & 0x5555555555555555) ^ RED_SQ[a >> 62] ^ d ^ (d << 3);
}
}
