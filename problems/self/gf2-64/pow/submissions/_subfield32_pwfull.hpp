#pragma once
// _subfield32.hpp の pow 表を、2 段 (257 + 256 要素) ではなく 65537 要素そのままで
// 持つ版 (提出ではない)。
//
// 2 段の方は h^m = h^(256 hi) · h^lo と分けるので表は 4 KB で L1 に載るが、μ_P 側と
// μ_N 側の掛け算を相乗りさせた mul2 が 1 本要る。こちらは表を引くだけで済む代わりに、
// 64 bit をそのまま持つので 512 KB + 512 KB になる。ランダムに叩く表は他にも
// SIG.LN (128 KB) と μ_P の log 表 (256 KB) があるので、合わせて L2 に収まるかどうかが
// 分かれ目になる。どちらが得かは環境しだいなので、差し替えられるようにしてある。
#include "_subfield32.hpp"
namespace gf2_64_pow_subfield32 {
struct PowFull {
 u64 P[65537];  // h_P^k
 u64 N[65535];  // BETA^k
};
constexpr PowFull PW_FULL= []() {
 PowFull r{};
 u64 cur= 1;
 for(u32 k= 0; k < 65537; ++k) {
  r.P[k]= cur;
  cur= apply_bt(MUL_HP, cur);
 }
 // F_2^16 側は SIG と同じ BETA の LFSR を歩いて、識別子を埋め込む。
 u16 col[]= {1U, 11778U, 7028U, 51115U, 48663U, 26081U, 17458U, 40223U, 30334U, 42368U, 14380U, 2223U, 49688U, 11217U, 44239U, 63445U};
 u16 T_lo[256]= {}, T_hi[256]= {};
 for(int v= 0; v < 256; ++v) {
  u16 lo= 0, hi= 0;
  for(int j= 0; j < 8; ++j)
   if((v >> j) & 1) {
    lo^= col[j];
    hi^= col[j + 8];
   }
  T_lo[v]= lo;
  T_hi[v]= hi;
 }
 u16 c= 1;
 for(u32 k= 0; k < 65535; ++k) {
  const u16 lo= T_lo[u8(c)] ^ T_hi[c >> 8];
  r.N[k]= EMBED.t[0][u8(lo)] ^ EMBED.t[1][lo >> 8];
  c= u16(c << 1) ^ (0x002DU & -u16(c >> 15));
 }
 return r;
}();
inline pair<u64, u64> pow_pair_full(u32 mp, u32 mn) { return {PW_FULL.P[mp], PW_FULL.N[mn]}; }
}  // namespace gf2_64_pow_subfield32
