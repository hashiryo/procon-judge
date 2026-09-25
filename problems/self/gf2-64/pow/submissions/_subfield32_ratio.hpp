#pragma once
// _subfield32.hpp の続き: μ_P 成分の log を b の F_2^16 座標の比から引く方 (提出ではない)。
//
// u = (b の μ_P 成分) を取る写像は準同型で、核が F_2^16^* なので、log_{h_P}(u) は
// b の属する類 [b] ∈ F_2^32^* / F_2^16^* (= 射影直線 P^1(F_2^16) の 65537 点) だけで
// 決まる。だから s = b^(2^16-1) を作らずに、類の番号を直に添字にすればよい。
//
// φ を Tr_{F_2^32/F_2^16}(φ) = φ + frob16(φ) = 1 に取ると、b ∈ F_2^32 は
//   b_1 = b + frob16(b),  b_0 = b + φ b_1
// で b = b_0 + φ b_1 (b_0, b_1 ∈ F_2^16) と分かれる。b_1 は frob16 の XOR で只、b_0 は
// φ 倍の 16 bit 入力 byte table 2 本 (1 KB) で出る。類は比 b_0 / b_1 で決まるので、
// F_2^16 の log 表の差 (SIG.LN は P' 倍が畳んであるが、差を添字にするぶんには全単射
// なのでそのまま使える) を添字にした 65535 要素の表を引けばよい。
// b_1 = 0 (b ∈ F_2^16, 類 ∞) と b_0 = 0 の 2 つの類だけ添字が作れないので別扱い。
//
// s を作る 4 回の乗算と frob が消える代わりに、F_2^16 の log 表を 1 回余計に引く。
#include "_subfield32.hpp"
namespace gf2_64_pow_subfield32 {
// φ: Tr_{F_2^32/F_2^16}(φ) = 1 を満たす元 (b = b_0 + φ b_1 の φ)
constexpr u64 PHI= 0x025bb4340671c0c5ULL;
static_assert((PHI ^ apply_bt(FROB16_RAW, PHI)) == 1, "PHI の相対トレースが 1 でない");
// LC.t[h][i]: φ 倍の下位 16 bit (入力は F_2^16 の識別子)。生配列で持つのは 65537 回の
// walk から引くため (std::array だと constexpr の step 上限に当たる)。
struct HalfTable {
 u16 t[2][256];
};
constexpr HalfTable LC= []() {
 const ByteTable m= mul_table(PHI);
 u16 basis[16]{};
 for(int i= 0; i < 16; ++i) basis[i]= u16(apply_bt(m, embed_idx(u16(1 << i))));
 HalfTable t{};
 for(int half= 0; half < 2; ++half)
  for(int j= 0; j < 8; ++j)
   for(int b= 0; b < (1 << j); ++b) t.t[half][(1 << j) | b]= t.t[half][b] ^ basis[j + half * 8];
 return t;
}();
// 類の番号 (b_0 / b_1 の log の差) → log_{h_P}(u)。K0 は b_0 = 0 の類のぶん。
struct MuCls {
 u32 LN[65535];
 u32 K0;
};
constexpr MuCls MU_CLS= []() {
 MuCls r{};
 u64 cur= 1;
 for(u32 k= 0; k < 65537; ++k) {
  const u16 b1= u16(cur ^ apply_bt(FROB16_RAW, cur));
  const u16 b0= u16(u16(cur) ^ LC.t[0][u8(b1)] ^ LC.t[1][b1 >> 8]);
  if(b1 == 0) {          // 類 ∞ = F_2^16 自身。u = 1 なので log は 0
  } else if(b0 == 0) {   // もう 1 つの特別な類
   r.K0= k;
  } else {
   u32 idx= SIG.LN[b0] + MOD_N - SIG.LN[b1];
   if(idx >= MOD_N) idx-= MOD_N;
   r.LN[idx]= k;
  }
  cur= apply_bt(MUL_HP, cur);
 }
 return r;
}();
// b = a^(2^32+1) ∈ F_2^32 と q から、b^q = h_P^mp · h_N^mn となる (mp, mn) を返す。
// そこから元を作るのは pow_pair (_subfield32.hpp) か pow_pair_full (_subfield32_pwfull.hpp)。
inline pair<u32, u32> subfield_exp2(u64 b, u32 q) {
 const u64 fb= frob16(b);
 const u64 t= mul(b, fb);  // b^(2^16+1) = b^P
 const u16 b1= u16(b ^ fb), b0= u16(u16(b) ^ LC.t[0][u8(b1)] ^ LC.t[1][b1 >> 8]);
 u32 idx= SIG.LN[b0] + MOD_N - SIG.LN[b1];
 if(idx >= MOD_N) idx-= MOD_N;
 u32 kp= MU_CLS.LN[idx];
 if(!b0) kp= MU_CLS.K0;
 if(!b1) kp= 0;
 return {u32(u64(kp) * q % MOD_P), u32(u64(SIG.LN[u16(t)]) * q % MOD_N)};
}
}  // namespace gf2_64_pow_subfield32
