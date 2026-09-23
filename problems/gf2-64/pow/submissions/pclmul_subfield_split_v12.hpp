#pragma once
// pclmul_subfield_split_v8_2 の「両 qword lane 活用」版 (byte_window_6_3 の subfield 版):
//
// imm=0x00 (low×low) と imm=0x11 (high×high) の 2 種類の mul2 を用意し、
// 1 本の ymm の q0..q3 全部に lane を詰めたまま 4 lane 分の乗算を行う:
//   facc = (f0, f1, f2, f3), tvec = (T0, T1, T2, T3) に対し
//   mul2  (imm 0x00): half0 = f0×T0, half1 = f2×T2 → (A0, A2)
//   mul2h (imm 0x11): half0 = f1×T1, half1 = f3×T3 → (A1, A3)
// frob4 の XOR 集約は packed 1 本 (vpxor 7 本) に戻り、v8 にあった extract ×4 も不要。
// T operand の詰めも 2 本 → packed 1 本。reduction は prod 配置が imm に依らず共通。
//
// 分割・結合は v8 系と同一:
//   a^r = frob36(a^{r_3}) · frob24(a^{r_2}) · frob12(a^{r_1}) · a^{r_0}
//   (12 bit × 4 lane, 各 lane 3 nibble, init + 2 反復, b は lane0 最下位に織り込み)
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_pow_subfield_split_v8_3 {
using gf2_64_pclmul::FROB12_BYTE;
using gf2_64_pclmul::frob16;
using gf2_64_pclmul::FROB24_BYTE;
using gf2_64_pclmul::frob3;
using gf2_64_pclmul::frob32;
using gf2_64_pclmul::frob48;
using gf2_64_pclmul::FROB6_BYTE;
using gf2_64_pclmul::mul;
using gf2_64_pclmul::sq;
// embed: low 16-bit subfield 識別子 → 64-bit poly 表現 (subfield 元の埋め込み)
constexpr u64 embed_idx(u16 idx) {
 static constexpr auto EMBED= []() {
  u64 SUBFIELD_BASIS[]= {0x0000000000000001ULL, 0x5fbfaec6aeac0002ULL, 0xb06c601895640004ULL, 0xb013b5277b7c0008ULL, 0xb5ebb915248a0010ULL, 0x109bb25b2c600020ULL, 0xbf3bd95bd4190040ULL, 0x0fc66342279b0080ULL, 0xb6418f5e57c50100ULL, 0xaa194bd4b83f0200ULL, 0x1b5217b4dcc70400ULL, 0xbb06fa73867a0800ULL, 0x006fd55b23331000ULL, 0x4ae8fb39198c2000ULL, 0xfbd141b29b4f4000ULL, 0x1d9ce1776be78000ULL};
  array<array<u64, 256>, 2> t{};
  for(int half= 0; half < 2; ++half)
   for(int i= 0; i < 256; ++i) {
    u64 v= 0;
    for(int b= 0; b < 8; ++b)
     if((i >> b) & 1) v^= SUBFIELD_BASIS[b + half * 8];
    t[half][i]= v;
   }
  return t;
 }();
 return EMBED[0][u8(idx)] ^ EMBED[1][idx >> 8];
}
// LN_SIGMA[u16(BETA^k)] = k, PW_SIGMA_IDX[k] = u16(BETA^k)
struct Tables {
 u16 LN_SIGMA[65536];
 u16 PW_SIGMA_IDX[65535];
};
constexpr auto TABLES= []() {
 // col[i] = u16(BETA^i) — BETA = 0x1f1af3ec55a22e02 の poly basis 累乗の低 16 bit
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
 Tables t{};
 // BETA の最小多項式は y^16 + y^5 + y^3 + y^2 + 1, lower bits = 0x002D
 u16 cur= 1;
 for(u32 k= 0; k < 65535; ++k) {
  u16 lo= T_lo[u8(cur)] ^ T_hi[cur >> 8];
  t.LN_SIGMA[lo]= u16(k);
  t.PW_SIGMA_IDX[k]= lo;
  cur= u16(cur << 1) ^ (0x002DU & -u16(cur >> 15));
 }
 t.LN_SIGMA[0]= 0;
 return t;
}();
const __m256i RED_TABLE= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
// 2 並列 mul: IMM で各 128bit half のどの qword を掛けるか選ぶ (0x00: low×low, 0x11: high×high)。
// reduction は prod 配置が IMM に依らないので共通。
template <int IMM> GNU_TARGET("vpclmulqdq") inline __m256i mul2t(const __m256i& a_vec, const __m256i& b_vec) {
 __m256i prod= _mm256_clmulepi64_epi128(a_vec, b_vec, IMM);
 __m256i d_full= _mm256_xor_si256(prod, _mm256_slli_epi64(prod, 1));
 __m256i red1_shift= _mm256_srli_si256(_mm256_xor_si256(d_full, _mm256_slli_epi64(d_full, 3)), 8);
 __m256i indices= _mm256_srli_si256(_mm256_srli_epi64(prod, 60), 8);
 return _mm256_xor_si256(_mm256_xor_si256(prod, red1_shift), _mm256_shuffle_epi8(RED_TABLE, indices));
}
GNU_TARGET("vpclmulqdq") inline __m256i mul2(const __m256i& a_vec, const __m256i& b_vec) { return mul2t<0x00>(a_vec, b_vec); }
GNU_TARGET("vpclmulqdq") inline __m256i mul2h(const __m256i& a_vec, const __m256i& b_vec) { return mul2t<0x11>(a_vec, b_vec); }
inline __m256i frob6_2lane(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(FROB6_BYTE[1][u8(a1 >> 8)], FROB6_BYTE[0][u8(a1)], FROB6_BYTE[1][u8(a0 >> 8)], FROB6_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB6_BYTE[3][u8(a1 >> 24)], FROB6_BYTE[2][u8(a1 >> 16)], FROB6_BYTE[3][u8(a0 >> 24)], FROB6_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB6_BYTE[5][u8(a1 >> 40)], FROB6_BYTE[4][u8(a1 >> 32)], FROB6_BYTE[5][u8(a0 >> 40)], FROB6_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB6_BYTE[7][u8(a1 >> 56)], FROB6_BYTE[6][u8(a1 >> 48)], FROB6_BYTE[7][u8(a0 >> 56)], FROB6_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob12_2lane(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(FROB12_BYTE[1][u8(a1 >> 8)], FROB12_BYTE[0][u8(a1)], FROB12_BYTE[1][u8(a0 >> 8)], FROB12_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB12_BYTE[3][u8(a1 >> 24)], FROB12_BYTE[2][u8(a1 >> 16)], FROB12_BYTE[3][u8(a0 >> 24)], FROB12_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB12_BYTE[5][u8(a1 >> 40)], FROB12_BYTE[4][u8(a1 >> 32)], FROB12_BYTE[5][u8(a0 >> 40)], FROB12_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB12_BYTE[7][u8(a1 >> 56)], FROB12_BYTE[6][u8(a1 >> 48)], FROB12_BYTE[7][u8(a0 >> 56)], FROB12_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob24_2lane(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(FROB24_BYTE[1][u8(a1 >> 8)], FROB24_BYTE[0][u8(a1)], FROB24_BYTE[1][u8(a0 >> 8)], FROB24_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB24_BYTE[3][u8(a1 >> 24)], FROB24_BYTE[2][u8(a1 >> 16)], FROB24_BYTE[3][u8(a0 >> 24)], FROB24_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB24_BYTE[5][u8(a1 >> 40)], FROB24_BYTE[4][u8(a1 >> 32)], FROB24_BYTE[5][u8(a0 >> 40)], FROB24_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB24_BYTE[7][u8(a1 >> 56)], FROB24_BYTE[6][u8(a1 >> 48)], FROB24_BYTE[7][u8(a0 >> 56)], FROB24_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline pair<u64, u64> unpack(const __m256i& vec) { return make_pair(u64(_mm256_extract_epi64(vec, 0)), u64(_mm256_extract_epi64(vec, 2))); }
GNU_TARGET("pclmul,vpclmulqdq") u64 pow(u64 a, u64 e) {
 if(!e) return 1;
 if(!a) return 0;
 constexpr u64 M_VAL= (~u64(0)) / 65535u;
 const u16 q= e / M_VAL;
 const u64 r= e - M_VAL * q;
 if(!r) {
  const u64 N2= mul(a, frob32(a));
  const u16 N= mul(N2, frob16(N2));
  return embed_idx(TABLES.PW_SIGMA_IDX[(u32(TABLES.LN_SIGMA[N]) * q) % 65535]);
 }

 // T[i] = a^i for i = 0..15、 binary-tree で 4 層に分けて VPCLMUL 並列化
 u64 T[8]= {1, a};
 u64 B;
 tie(T[2], B)= unpack(mul2(_mm256_set1_epi64x(a), _mm256_set_epi64x(0, frob32(a), 0, a)));
 // L2: T[3], T[4]
 __m256i T12= _mm256_set_epi64x(0, T[2], 0, a);
 tie(T[3], T[4])= unpack(mul2(T12, _mm256_set1_epi64x(T[2])));
 // L3: T[5..8]
 tie(T[5], T[6])= unpack(mul2(_mm256_set1_epi64x(T[4]), T12));
 tie(T[7], B)= unpack(mul2(_mm256_set_epi64x(0, frob16(B), 0, T[3]), _mm256_set_epi64x(0, B, 0, T[4])));

 //  8 0000000100000000 + 0 0000000000000001
 //  9 000000100000000  + 1 000000000000001
 // 10 00000100000000   + 2 00000000000001
 // 11 0000100000000    + 3 0000000000001
 // 12 000100000000     + 4 000000000001
 // 13 00100000000      + 5 00000000001
 // 14 0100000000       + 6 0000000001
 // 15 100000000        + 7 000000001
 // frob24 * 1
 auto [A6, A7]= unpack(mul2(frob24_2lane(T[(r >> 42) & 7], T[(r >> 45) & 7]), _mm256_set_epi64x(0, T[(r >> 21) & 7], 0, T[(r >> 18) & 7])));
 auto [A4, A5]= unpack(mul2(frob24_2lane(T[(r >> 36) & 7], T[(r >> 39) & 7]), _mm256_set_epi64x(0, T[(r >> 15) & 7], 0, T[(r >> 12) & 7])));
 __m256i A23= mul2(frob24_2lane(T[(r >> 30) & 7], T[(r >> 33) & 7]), _mm256_set_epi64x(0, T[(r >> 9) & 7], 0, T[(r >> 6) & 7]));
 __m256i A01= mul2(frob24_2lane(T[(r >> 24) & 7], T[(r >> 27) & 7]), _mm256_set_epi64x(0, T[(r >> 3) & 7], 0, T[r & 7]));
 // 4 0001000000010000 + 0 0000000100000001
 // 5 001000000010000  + 1 000000100000001
 // 6 01000000010000   + 2 00000100000001
 // 7 1000000010000    + 3 0000100000001
 // frob12 * 1
 auto [A2, A3]= unpack(mul2(frob12_2lane(A6, A7), A23));
 A01= mul2(frob12_2lane(A4, A5), A01);
 // 0100010001000100 + 0001000100010001
 // 100010001000100  + 001000100010001
 // frob6 * 1
 auto [A0, A1]= unpack(mul2(frob6_2lane(A2, A3), A01));
 // 1010101010101010 + 0101010101010101
 // frob3 * 1
 u64 bt= embed_idx(TABLES.PW_SIGMA_IDX[(u32(TABLES.LN_SIGMA[u16(B)]) * q) % 65535]);
 if(r >> 48) bt= mul(bt, frob48(a));
 return mul(mul(bt, A0), frob3(A1));
}
}  // namespace gf2_64_pow_subfield_split_v8_3
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as, const vector<u64>& es) {
  using gf2_64_pow_subfield_split_v8_3::pow;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= pow(as[i], es[i]);
  return ans;
 }
};
