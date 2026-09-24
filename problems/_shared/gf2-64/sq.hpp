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
const __m128i spread_tbl= _mm_setr_epi8(0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15, 0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55);
constexpr u8 RED[4]= {0, 27, 90, 65};
inline u64 sq(u64 a) {
 // spread_tbl[v] = bit i ∈ v を bit 2i に展開した 8-bit 値
 //   v=0x0:0x00, 0x1:0x01, 0x2:0x04, 0x3:0x05, 0x4:0x10, ... , 0xF:0x55
 __m128i v= _mm_set_epi64x(0, a);
 __m128i nib= _mm_and_si128(_mm_unpacklo_epi8(v, _mm_srli_epi16(v, 4)), mask_lo);
 __m128i squared= _mm_shuffle_epi8(spread_tbl, nib);
 // squared の下位 64 bit が a の下位 32 bit の平方、上位 64 bit が a の上位 32 bit の平方
 u64 h= squared[1], d= h ^ (h << 1);
 return squared[0] ^ RED[a >> 62] ^ d ^ (d << 3);
}
}
