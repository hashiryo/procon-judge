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
//   4. x = PSHUFB(spread3_tbl, nib) & lo_even で 16 nibble を 1 回で引く
//      spread3_tbl は bit i を 2i と 2i+1 の両方に置く表 (= spread * 3) で、上位 64 bit はそのまま d = h ^ (h << 1) になる。
//      下位 64 bit は偶数 bit だけ残して spread (= lo) に戻す
//   5. d から PDEP 版と同じ shift+XOR で還元
//
// pshufb_2_rednamed.hpp との違い: 還元の d = h ^ (h << 1) を表に入れた。
//   h は偶数 bit しか立たないので h ^ (h << 1) = h * 3 は nibble ごとに閉じていて、byte からはみ出さない。
//   PSHUFB は 16 byte 全部に同じ表を使うので lo 側は AND で戻す必要があり、命令数は変わらない
//   (d を作る 1 命令が lo の AND 1 命令に置き換わる)。
//
// PDEP との比較 (実機):
//   PDEP は Intel ~3 cyc, AMD Zen3 まで ~18 cyc (Zen4 で改善)
//   PSHUFB は arch 共通で 1-1.5 cyc
//   → AMD で有利、Intel/ARM でも互角
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_sq_pshufb {
const __m128i mask_lo= _mm_set1_epi8(0x0F);
const __m128i spread3_tbl= _mm_setr_epi8(0x00, 0x03, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F, (signed char)0xC0, (signed char)0xC3, (signed char)0xCC, (signed char)0xCF, (signed char)0xF0, (signed char)0xF3, (signed char)0xFC, (signed char)0xFF);
// 下位 64 bit は偶数 bit だけ残し (spread に戻す)、上位 64 bit (d) はそのまま通す
const __m128i lo_even= _mm_set_epi64x(-1, 0x5555555555555555);
constexpr u8 RED_SQ[4]= {0, 27, 90, 65};
inline u64 sq(u64 a) {
 // spread3_tbl[v] = bit i ∈ v を bit 2i と 2i+1 の両方に展開した 8-bit 値 (= spread_tbl[v] * 3)
 //   v=0x0:0x00, 0x1:0x03, 0x2:0x0C, 0x3:0x0F, 0x4:0x30, ... , 0xF:0xFF
 __m128i v= _mm_set_epi64x(0, a);
 __m128i nib= _mm_and_si128(_mm_unpacklo_epi8(v, _mm_srli_epi16(v, 4)), mask_lo);
 __m128i x= _mm_and_si128(_mm_shuffle_epi8(spread3_tbl, nib), lo_even);
 // x の下位 64 bit が a の下位 32 bit の平方 (lo)、上位 64 bit が d = h ^ (h << 1) (h は a の上位 32 bit の平方)
 u64 d= x[1];
 return x[0] ^ RED_SQ[a >> 62] ^ d ^ (d << 3);
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
