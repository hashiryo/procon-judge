#pragma once
// 「F_2^32 部分体 + CRT」で a^e の上位を表引きにする版の土台 (提出ではない)。
//
// e = q (2^32+1) + r (0 ≤ r ≤ 2^32) と割ると b = a^(2^32+1) は F_2^32 の元になる。
// (2^64-1) / (2^32-1) = 2^32+1 なので、この指数がちょうど F_2^32 への落とし方。
// b^q を表引きで済ませれば、窓で回すのは r の 32 bit だけになる。2^16-1 で割る既存の
// subfield_split は q < 2^16 しか表に載せられず 48 bit を窓で回していた。
//
// P = 2^16+1 = 65537, N = 2^16-1 = 65535 とおく。F_2^32^* は位数 PN の巡回群で、P と N は
// 互いに素だから b = u v (u ∈ μ_P, v ∈ μ_N = F_2^16^*) と一意に分かれ、
//   b^q = h_P^(log_{h_P}(u) q mod P) · h_N^(log_{h_N}(v) q mod N)
// になる。v の側は t = b^P = b^(2^16+1) = b · frob16(b) が v^P なので、log 表の値に
// P' = P^-1 mod N = 32768 を掛けておけば t の log から直に指数が出る (gf2-64-log の
// pohlig 版が 2699 を畳んでいるのと同じ)。u の側の log の引き方は 2 通り作ってあり、
// _subfield32_norm.hpp と _subfield32_ratio.hpp がそれぞれを足す。
//
// pow 表は 256 で割った商と余りの 2 段にしてある (h^m = h^(256 hi) · h^lo)。65537 要素を
// 直接持つと 512 KB だが、257 + 256 要素なら 4 KB で L1 に載る。掛け算が 1 本増えるだけ。
//
// 表は全部 constexpr で焼く。65537 回の walk を回す版があるので、表は生配列で持つこと
// (std::array の operator[] は clang の constexpr step 上限をすぐ使い切る)。
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/mul2.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_pow_subfield32 {
using gf2_64_pclmul::frob16;
using gf2_64_pclmul::mul;
using gf2_64_pclmul::mul2;
using gf2_64_pclmul::sq;
using gf2_64_pclmul::unpack;
constexpr u32 MOD_P= 65537, MOD_N= 65535;
constexpr u32 CRT_FOLD= 32768;            // = P^-1 mod N = N^-1 mod P
constexpr u64 SPLIT= (u64(1) << 32) + 1;  // e = q SPLIT + r
constexpr u64 H_P= 0x1c1e79669b95a7ceULL;  // 位数 65537 の元 (gf2-64-log の pohlig 版と同じ)
// F_2 線型写像の 8x256 byte table。constexpr の step を食わないよう生配列で持つ。
struct ByteTable {
 u64 t[8][256];
};
constexpr ByteTable expand(const u64 (&basis)[64]) {
 ByteTable r{};
 for(int p= 0; p < 8; ++p)
  for(int j= 0; j < 8; ++j) {
   const u64 v= basis[8 * p + j];
   for(int b= 0; b < (1 << j); ++b) r.t[p][(1 << j) | b]= r.t[p][b] ^ v;
  }
 return r;
}
constexpr u64 apply_bt(const ByteTable &T, u64 a) { return T.t[0][u8(a)] ^ T.t[1][u8(a >> 8)] ^ T.t[2][u8(a >> 16)] ^ T.t[3][u8(a >> 24)] ^ T.t[4][u8(a >> 32)] ^ T.t[5][u8(a >> 40)] ^ T.t[6][u8(a >> 48)] ^ T.t[7][u8(a >> 56)]; }
// c 倍の byte table。基底の像は x 倍 (shift + reduce) を 64 回繰り返すだけで出る。
constexpr ByteTable mul_table(u64 c) {
 u64 basis[64]{};
 basis[0]= c;
 for(int i= 1; i < 64; ++i) basis[i]= (basis[i - 1] << 1) ^ (IRRED_LOW & -(basis[i - 1] >> 63));
 return expand(basis);
}
constexpr ByteTable MUL_HP= mul_table(H_P);
// frob.hpp の表は std::array なので、walk で引く用に生配列へ写しておく。
constexpr ByteTable FROB16_RAW= []() {
 ByteTable r{};
 for(int p= 0; p < 8; ++p)
  for(int b= 0; b < 256; ++b) r.t[p][b]= gf2_64_pclmul::FROB16_BYTE[p][b];
 return r;
}();
// 部分体 F_2^16 の埋め込み。SUBFIELD_BASIS[i] は下位 16 bit がちょうど 1 << i なので、
// 元の下位 16 bit がそのまま識別子になり、embed_idx がその逆写像になる。
struct EmbedTable {
 u64 t[2][256];
};
constexpr EmbedTable EMBED= []() {
 u64 SUBFIELD_BASIS[]= {0x0000000000000001ULL, 0x5fbfaec6aeac0002ULL, 0xb06c601895640004ULL, 0xb013b5277b7c0008ULL, 0xb5ebb915248a0010ULL, 0x109bb25b2c600020ULL, 0xbf3bd95bd4190040ULL, 0x0fc66342279b0080ULL, 0xb6418f5e57c50100ULL, 0xaa194bd4b83f0200ULL, 0x1b5217b4dcc70400ULL, 0xbb06fa73867a0800ULL, 0x006fd55b23331000ULL, 0x4ae8fb39198c2000ULL, 0xfbd141b29b4f4000ULL, 0x1d9ce1776be78000ULL};
 EmbedTable r{};
 for(int half= 0; half < 2; ++half)
  for(int j= 0; j < 8; ++j)
   for(int b= 0; b < (1 << j); ++b) r.t[half][(1 << j) | b]= r.t[half][b] ^ SUBFIELD_BASIS[j + half * 8];
 return r;
}();
constexpr u64 embed_idx(u16 idx) { return EMBED.t[0][u8(idx)] ^ EMBED.t[1][idx >> 8]; }
// μ_P (位数 65537) の 2 段 pow 表。log 表の方は norm / ratio のヘッダが足す。
struct MuPow {
 u64 HI[257];
 u64 LO[256];
};
constexpr MuPow MU_PW= []() {
 MuPow r{};
 u64 cur= 1;
 for(u32 k= 0; k < 256; ++k) {
  r.LO[k]= cur;
  cur= apply_bt(MUL_HP, cur);
 }
 const ByteTable step= mul_table(cur);  // cur = h_P^256
 cur= 1;
 for(u32 i= 0; i < 257; ++i) {
  r.HI[i]= cur;
  cur= apply_bt(step, cur);
 }
 return r;
}();
// F_2^16^* 側: 生成元 BETA = 0x1f1af3ec55a22e02 の log / pow。col[j] = u16(BETA^j) で、
// BETA の最小多項式は y^16 + y^5 + y^3 + y^2 + 1 (lower bits = 0x002D)。
// LN には P' = 32768 を掛けた値が入っている。
struct SigTables {
 u16 LN[65536];
 u64 HI[256];
 u64 LO[256];
};
constexpr SigTables SIG= []() {
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
 SigTables r{};
 u16 cur= 1;
 u32 v= 0;  // (k * P') mod N
 for(u32 k= 0; k < 65535; ++k) {
  const u16 lo= T_lo[u8(cur)] ^ T_hi[cur >> 8];
  r.LN[lo]= u16(v);
  if(k < 256) r.LO[k]= embed_idx(lo);
  if(!(k & 255)) r.HI[k >> 8]= embed_idx(lo);
  cur= u16(cur << 1) ^ (0x002DU & -u16(cur >> 15));
  v+= CRT_FOLD;
  if(v >= MOD_N) v-= MOD_N;
 }
 r.LN[0]= 0;
 return r;
}();
// u ∈ μ_P の log (mp) と v ∈ μ_N の log (mn) から b^q の 2 因子を作る (2 段の表を使う版)。
// 呼ぶ側が窓の結果と一緒に mul2 へ渡せるよう、掛け合わせる前の形で返している。
// 表を 65537 要素そのまま持つ版は _subfield32_pwfull.hpp にある。
inline pair<u64, u64> pow_pair(u32 mp, u32 mn) { return unpack(mul2(_mm256_set_epi64x(0, SIG.HI[mn >> 8], 0, MU_PW.HI[mp >> 8]), _mm256_set_epi64x(0, SIG.LO[mn & 255], 0, MU_PW.LO[mp & 255]))); }
// 窓の畳み込みで使う frobK の 2 lane 版 (byte_window 系と同じ形)。結果は q0, q2 に入るので
// そのまま mul2 の operand にできる。呼ぶ側で使うものだけが実体化される。
inline __m256i frob2_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB2_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB2_BYTE[1][u8(a1 >> 8)], FROB2_BYTE[0][u8(a1)], FROB2_BYTE[1][u8(a0 >> 8)], FROB2_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB2_BYTE[3][u8(a1 >> 24)], FROB2_BYTE[2][u8(a1 >> 16)], FROB2_BYTE[3][u8(a0 >> 24)], FROB2_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB2_BYTE[5][u8(a1 >> 40)], FROB2_BYTE[4][u8(a1 >> 32)], FROB2_BYTE[5][u8(a0 >> 40)], FROB2_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB2_BYTE[7][u8(a1 >> 56)], FROB2_BYTE[6][u8(a1 >> 48)], FROB2_BYTE[7][u8(a0 >> 56)], FROB2_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob3_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB3_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB3_BYTE[1][u8(a1 >> 8)], FROB3_BYTE[0][u8(a1)], FROB3_BYTE[1][u8(a0 >> 8)], FROB3_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB3_BYTE[3][u8(a1 >> 24)], FROB3_BYTE[2][u8(a1 >> 16)], FROB3_BYTE[3][u8(a0 >> 24)], FROB3_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB3_BYTE[5][u8(a1 >> 40)], FROB3_BYTE[4][u8(a1 >> 32)], FROB3_BYTE[5][u8(a0 >> 40)], FROB3_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB3_BYTE[7][u8(a1 >> 56)], FROB3_BYTE[6][u8(a1 >> 48)], FROB3_BYTE[7][u8(a0 >> 56)], FROB3_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob4_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB4_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB4_BYTE[1][u8(a1 >> 8)], FROB4_BYTE[0][u8(a1)], FROB4_BYTE[1][u8(a0 >> 8)], FROB4_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB4_BYTE[3][u8(a1 >> 24)], FROB4_BYTE[2][u8(a1 >> 16)], FROB4_BYTE[3][u8(a0 >> 24)], FROB4_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB4_BYTE[5][u8(a1 >> 40)], FROB4_BYTE[4][u8(a1 >> 32)], FROB4_BYTE[5][u8(a0 >> 40)], FROB4_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB4_BYTE[7][u8(a1 >> 56)], FROB4_BYTE[6][u8(a1 >> 48)], FROB4_BYTE[7][u8(a0 >> 56)], FROB4_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob6_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB6_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB6_BYTE[1][u8(a1 >> 8)], FROB6_BYTE[0][u8(a1)], FROB6_BYTE[1][u8(a0 >> 8)], FROB6_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB6_BYTE[3][u8(a1 >> 24)], FROB6_BYTE[2][u8(a1 >> 16)], FROB6_BYTE[3][u8(a0 >> 24)], FROB6_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB6_BYTE[5][u8(a1 >> 40)], FROB6_BYTE[4][u8(a1 >> 32)], FROB6_BYTE[5][u8(a0 >> 40)], FROB6_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB6_BYTE[7][u8(a1 >> 56)], FROB6_BYTE[6][u8(a1 >> 48)], FROB6_BYTE[7][u8(a0 >> 56)], FROB6_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob8_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB8_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB8_BYTE[1][u8(a1 >> 8)], FROB8_BYTE[0][u8(a1)], FROB8_BYTE[1][u8(a0 >> 8)], FROB8_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB8_BYTE[3][u8(a1 >> 24)], FROB8_BYTE[2][u8(a1 >> 16)], FROB8_BYTE[3][u8(a0 >> 24)], FROB8_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB8_BYTE[5][u8(a1 >> 40)], FROB8_BYTE[4][u8(a1 >> 32)], FROB8_BYTE[5][u8(a0 >> 40)], FROB8_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB8_BYTE[7][u8(a1 >> 56)], FROB8_BYTE[6][u8(a1 >> 48)], FROB8_BYTE[7][u8(a0 >> 56)], FROB8_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob12_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB12_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB12_BYTE[1][u8(a1 >> 8)], FROB12_BYTE[0][u8(a1)], FROB12_BYTE[1][u8(a0 >> 8)], FROB12_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB12_BYTE[3][u8(a1 >> 24)], FROB12_BYTE[2][u8(a1 >> 16)], FROB12_BYTE[3][u8(a0 >> 24)], FROB12_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB12_BYTE[5][u8(a1 >> 40)], FROB12_BYTE[4][u8(a1 >> 32)], FROB12_BYTE[5][u8(a0 >> 40)], FROB12_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB12_BYTE[7][u8(a1 >> 56)], FROB12_BYTE[6][u8(a1 >> 48)], FROB12_BYTE[7][u8(a0 >> 56)], FROB12_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob16_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB16_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB16_BYTE[1][u8(a1 >> 8)], FROB16_BYTE[0][u8(a1)], FROB16_BYTE[1][u8(a0 >> 8)], FROB16_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB16_BYTE[3][u8(a1 >> 24)], FROB16_BYTE[2][u8(a1 >> 16)], FROB16_BYTE[3][u8(a0 >> 24)], FROB16_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB16_BYTE[5][u8(a1 >> 40)], FROB16_BYTE[4][u8(a1 >> 32)], FROB16_BYTE[5][u8(a0 >> 40)], FROB16_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB16_BYTE[7][u8(a1 >> 56)], FROB16_BYTE[6][u8(a1 >> 48)], FROB16_BYTE[7][u8(a0 >> 56)], FROB16_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob24_2lane(u64 a0, u64 a1) {
 using gf2_64_pclmul::FROB24_BYTE;
 __m256i vA= _mm256_set_epi64x(FROB24_BYTE[1][u8(a1 >> 8)], FROB24_BYTE[0][u8(a1)], FROB24_BYTE[1][u8(a0 >> 8)], FROB24_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB24_BYTE[3][u8(a1 >> 24)], FROB24_BYTE[2][u8(a1 >> 16)], FROB24_BYTE[3][u8(a0 >> 24)], FROB24_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB24_BYTE[5][u8(a1 >> 40)], FROB24_BYTE[4][u8(a1 >> 32)], FROB24_BYTE[5][u8(a0 >> 40)], FROB24_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB24_BYTE[7][u8(a1 >> 56)], FROB24_BYTE[6][u8(a1 >> 48)], FROB24_BYTE[7][u8(a0 >> 56)], FROB24_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
}  // namespace gf2_64_pow_subfield32
