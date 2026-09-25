#pragma once
// PSHUFB (SSSE3) ベースの squaring:
//   GF(2) で a^2 = bit i → bit 2i (cross term ゼロ)。
//   4-bit nibble 単位の spread (nibble 値 v ∈ [0,15] → 8-bit spread, bits at 0,2,4,6) を
//   16 entry テーブルで保持し、PSHUFB で 16 nibble parallel lookup する。
//
// 流れ:
//   1. v = [0:a] を XMM に置く (8 byte が下位)
//   2. s = v >> 4 (SRLI epi16。各 byte の低 nibble 側に元の高 nibble が来る)
//   3. nib = unpacklo(v, s) & 0x0F で byte 2k に byte k の低 nibble、byte 2k+1 に高 nibble を交互配置
//      (s で隣の byte から漏れる bit は高 nibble 側に入るので、AND は unpack 後の 1 回で消せる)
//   4. squared = PSHUFB(spread_tbl, nib) で 16 nibble を 1 回で引く → 128-bit 平方
//   5. squared.hi64 を反 reducible 部分とみなし PDEP 版と同じ shift+XOR で還元
//
// pshufb.hpp との違い: 表引きしてから交互配置するのをやめ、添字を先に交互配置する。
//   PSHUFB は byte ごとの表引きなので並べ替えと順序を入れ替えても結果は同じで、PSHUFB 2 → 1、AND 2 → 1 になる。
//
// PDEP との比較 (実機):
//   PDEP は Intel ~3 cyc, AMD Zen3 まで ~18 cyc (Zen4 で改善)
//   PSHUFB は arch 共通で 1-1.5 cyc
//   → AMD で有利、Intel/ARM でも互角
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_sq_pshufb {
const __m128i mask_lo= _mm_set1_epi8(0x0F);
const __m128i spread_tbl= _mm_setr_epi8(0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15, 0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55);
inline u64 sq(u64 a) {
 // spread_tbl[v] = bit i ∈ v を bit 2i に展開した 8-bit 値
 //   v=0x0:0x00, 0x1:0x01, 0x2:0x04, 0x3:0x05, 0x4:0x10, ... , 0xF:0x55
 __m128i v= _mm_set_epi64x(0, a);
 __m128i nib= _mm_and_si128(_mm_unpacklo_epi8(v, _mm_srli_epi16(v, 4)), mask_lo);
 __m128i squared= _mm_shuffle_epi8(spread_tbl, nib);
 // squared の下位 64 bit が a の下位 32 bit の平方、上位 64 bit が a の上位 32 bit の平方
 u64 h= squared[1], d= h ^ (h << 1);
 return squared[0] ^ ((u8[]){0, 27, 90, 65})[a >> 62] ^ d ^ (d << 3);
}
}  // namespace gf2_64_sq_pshufb
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using gf2_64_sq_pshufb::sq;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= sq(as[i]);
  return ans;
 }
};
