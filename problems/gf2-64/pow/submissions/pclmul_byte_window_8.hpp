#pragma once
// pclmul_byte_window_6 の frob4_4lane 配置換え解消版:
//
// 6 は frob4_4lane が packed ymm (q0..q3) で XOR を畳んだ後 extract ×4 で GPR に下ろし、
// loop 側で set_epi64x ×2 により mul2 の operand 配置 (qword 0, 2) へ載せ直していた。
// _mm256_clmulepi64_epi128(a, b, 0) は各 128bit half の下位 qword しか読まないので、
// XOR 集約を最初から (q0, q2) 配置の ymm 2 本 (lane 0,1 用 / lane 2,3 用) で行えば、
// frob の結果ベクタをそのまま mul2 に直結でき、毎反復の extract ×4 + 再構築 ×2 が消える
// (q1, q3 は不定のままで良い)。vpxor は 7 → 14 本に増えるが、table load は同じ 32 本で
// 2 個詰めは vpinsrq 不要 (vmovq + vinserti128) なので詰めコストはほぼ同等。
// mul2 出力側の unpack は残る — 次の frob の byte 抽出 (アドレス計算) は GPR でしか
// できないため。
//
// 分割・結合は 6 と同一:
//   a^e = frob48(a^{e_3}) · frob32(a^{e_2}) · frob16(a^{e_1}) · a^{e_0}
//   (16 bit × 4 lane, 各 lane 4 nibble, init + 3 反復, 結合は frob ×3 + mul2 + mul)
//
// 必要な拡張: VPCLMULQDQ + AVX2 (Intel Ice Lake / AMD Zen3 以降, dashboard EPYC 7763 で動作)。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/mul2.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_pow_byte_window_6_2 {
using gf2_64_pclmul::frob16;
using gf2_64_pclmul::FROB16_BYTE;
using gf2_64_pclmul::FROB32_BYTE;
using gf2_64_pclmul::frob4;
using gf2_64_pclmul::FROB4_BYTE;
using gf2_64_pclmul::FROB8_BYTE;
using gf2_64_pclmul::mul;
using gf2_64_pclmul::mul2;
using gf2_64_pclmul::sq;
using gf2_64_pclmul::unpack;
inline __m256i frob4_2lane(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(FROB4_BYTE[1][u8(a1 >> 8)], FROB4_BYTE[0][u8(a1)], FROB4_BYTE[1][u8(a0 >> 8)], FROB4_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB4_BYTE[3][u8(a1 >> 24)], FROB4_BYTE[2][u8(a1 >> 16)], FROB4_BYTE[3][u8(a0 >> 24)], FROB4_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB4_BYTE[5][u8(a1 >> 40)], FROB4_BYTE[4][u8(a1 >> 32)], FROB4_BYTE[5][u8(a0 >> 40)], FROB4_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB4_BYTE[7][u8(a1 >> 56)], FROB4_BYTE[6][u8(a1 >> 48)], FROB4_BYTE[7][u8(a0 >> 56)], FROB4_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob8_2lane(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(FROB8_BYTE[1][u8(a1 >> 8)], FROB8_BYTE[0][u8(a1)], FROB8_BYTE[1][u8(a0 >> 8)], FROB8_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB8_BYTE[3][u8(a1 >> 24)], FROB8_BYTE[2][u8(a1 >> 16)], FROB8_BYTE[3][u8(a0 >> 24)], FROB8_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB8_BYTE[5][u8(a1 >> 40)], FROB8_BYTE[4][u8(a1 >> 32)], FROB8_BYTE[5][u8(a0 >> 40)], FROB8_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB8_BYTE[7][u8(a1 >> 56)], FROB8_BYTE[6][u8(a1 >> 48)], FROB8_BYTE[7][u8(a0 >> 56)], FROB8_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob16_2lane(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(FROB16_BYTE[1][u8(a1 >> 8)], FROB16_BYTE[0][u8(a1)], FROB16_BYTE[1][u8(a0 >> 8)], FROB16_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB16_BYTE[3][u8(a1 >> 24)], FROB16_BYTE[2][u8(a1 >> 16)], FROB16_BYTE[3][u8(a0 >> 24)], FROB16_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB16_BYTE[5][u8(a1 >> 40)], FROB16_BYTE[4][u8(a1 >> 32)], FROB16_BYTE[5][u8(a0 >> 40)], FROB16_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB16_BYTE[7][u8(a1 >> 56)], FROB16_BYTE[6][u8(a1 >> 48)], FROB16_BYTE[7][u8(a0 >> 56)], FROB16_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob32_2lane(u64 a0, u64 a1) {
 __m256i vA= _mm256_set_epi64x(FROB32_BYTE[1][u8(a1 >> 8)], FROB32_BYTE[0][u8(a1)], FROB32_BYTE[1][u8(a0 >> 8)], FROB32_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB32_BYTE[3][u8(a1 >> 24)], FROB32_BYTE[2][u8(a1 >> 16)], FROB32_BYTE[3][u8(a0 >> 24)], FROB32_BYTE[2][u8(a0 >> 16)]);
 __m256i vC= _mm256_set_epi64x(FROB32_BYTE[5][u8(a1 >> 40)], FROB32_BYTE[4][u8(a1 >> 32)], FROB32_BYTE[5][u8(a0 >> 40)], FROB32_BYTE[4][u8(a0 >> 32)]);
 __m256i vD= _mm256_set_epi64x(FROB32_BYTE[7][u8(a1 >> 56)], FROB32_BYTE[6][u8(a1 >> 48)], FROB32_BYTE[7][u8(a0 >> 56)], FROB32_BYTE[6][u8(a0 >> 48)]);
 __m256i y= _mm256_xor_si256(_mm256_xor_si256(vA, vB), _mm256_xor_si256(vC, vD));
 return _mm256_xor_si256(y, _mm256_srli_si256(y, 8));
}
inline __m256i frob32_4lane(u64 a0, u64 a1, u64 a2, u64 a3) {
 __m256i vA= _mm256_set_epi64x(FROB32_BYTE[0][u8(a3)], FROB32_BYTE[0][u8(a2)], FROB32_BYTE[0][u8(a1)], FROB32_BYTE[0][u8(a0)]);
 __m256i vB= _mm256_set_epi64x(FROB32_BYTE[1][u8(a3 >> 8)], FROB32_BYTE[1][u8(a2 >> 8)], FROB32_BYTE[1][u8(a1 >> 8)], FROB32_BYTE[1][u8(a0 >> 8)]);
 __m256i vC= _mm256_set_epi64x(FROB32_BYTE[2][u8(a3 >> 16)], FROB32_BYTE[2][u8(a2 >> 16)], FROB32_BYTE[2][u8(a1 >> 16)], FROB32_BYTE[2][u8(a0 >> 16)]);
 __m256i vD= _mm256_set_epi64x(FROB32_BYTE[3][u8(a3 >> 24)], FROB32_BYTE[3][u8(a2 >> 24)], FROB32_BYTE[3][u8(a1 >> 24)], FROB32_BYTE[3][u8(a0 >> 24)]);
 __m256i vE= _mm256_set_epi64x(FROB32_BYTE[4][u8(a3 >> 32)], FROB32_BYTE[4][u8(a2 >> 32)], FROB32_BYTE[4][u8(a1 >> 32)], FROB32_BYTE[4][u8(a0 >> 32)]);
 __m256i vF= _mm256_set_epi64x(FROB32_BYTE[5][u8(a3 >> 40)], FROB32_BYTE[5][u8(a2 >> 40)], FROB32_BYTE[5][u8(a1 >> 40)], FROB32_BYTE[5][u8(a0 >> 40)]);
 __m256i vG= _mm256_set_epi64x(FROB32_BYTE[6][u8(a3 >> 48)], FROB32_BYTE[6][u8(a2 >> 48)], FROB32_BYTE[6][u8(a1 >> 48)], FROB32_BYTE[6][u8(a0 >> 48)]);
 __m256i vH= _mm256_set_epi64x(FROB32_BYTE[7][u8(a3 >> 56)], FROB32_BYTE[7][u8(a2 >> 56)], FROB32_BYTE[7][u8(a1 >> 56)], FROB32_BYTE[7][u8(a0 >> 56)]);
 __m256i vAB= _mm256_xor_si256(vA, vB);
 __m256i vCD= _mm256_xor_si256(vC, vD);
 __m256i vEF= _mm256_xor_si256(vE, vF);
 __m256i vGH= _mm256_xor_si256(vG, vH);
 return _mm256_xor_si256(_mm256_xor_si256(vAB, vCD), _mm256_xor_si256(vEF, vGH));
}
GNU_TARGET("pclmul,vpclmulqdq") u64 pow(u64 a, u64 e) {
 if(e == 0) return 1;
 // T[i] = a^i for i = 0..15、 binary-tree で 4 層に分けて VPCLMUL 並列化 (6 と同一)
 u64 T[16]= {1, a, sq(a)};
 // L2: T[3], T[4]
 __m256i T12= _mm256_set_epi64x(0, T[2], 0, a);
 __m256i T34= mul2(T12, _mm256_set1_epi64x(T[2]));
 tie(T[3], T[4])= unpack(T34);
 // L3: T[5..8]
 __m256i T4= _mm256_set1_epi64x(T[4]);
 __m256i T56= mul2(T4, T12);
 tie(T[5], T[6])= unpack(T56);
 tie(T[7], T[8])= unpack(mul2(T4, T34));
 // L4: T[9..15] (T[15] は単独 mul)
 __m256i T8= _mm256_set1_epi64x(T[8]);
 tie(T[9], T[10])= unpack(mul2(T8, T12));
 tie(T[11], T[12])= unpack(mul2(T8, T34));
 tie(T[13], T[14])= unpack(mul2(T8, T56));
 T[15]= mul(T[7], T[8]);
 // メイン loop: 16 bit × 4 lane lockstep (各 lane 4 nibble, init + 3 反復)

 //  8 0000000100000000 + 0 0000000000000001
 //  9 000000100000000  + 1 000000000000001
 // 10 00000100000000   + 2 00000000000001
 // 11 0000100000000    + 3 0000000000001
 // 12 000100000000     + 4 000000000001
 // 13 00100000000      + 5 00000000001
 // 14 0100000000       + 6 0000000001
 // 15 100000000        + 7 000000001
 // frob32 * 1
 __m256i f32h= frob32_4lane(T[(e >> 48) & 0xF], T[(e >> 56) & 0xF], T[(e >> 52) & 0xF], T[(e >> 60) & 0xF]);
 __m256i f32l= frob32_4lane(T[(e >> 32) & 0xF], T[(e >> 40) & 0xF], T[(e >> 36) & 0xF], T[(e >> 44) & 0xF]);
 auto [A6, A7]= unpack(mul2<0x01>(f32h, _mm256_set_epi64x(0, T[(e >> 28) & 0xF], 0, T[(e >> 24) & 0xF])));
 auto [A4, A5]= unpack(mul2(f32h, _mm256_set_epi64x(0, T[(e >> 20) & 0xF], 0, T[(e >> 16) & 0xF])));
 __m256i A23= mul2<0x01>(f32l, _mm256_set_epi64x(0, T[(e >> 12) & 0xF], 0, T[(e >> 8) & 0xF]));
 __m256i A01= mul2(f32l, _mm256_set_epi64x(0, T[(e >> 4) & 0xF], 0, T[e & 0xF]));
 // 4 0001000000010000 + 0 0000000100000001
 // 5 001000000010000  + 1 000000100000001
 // 6 01000000010000   + 2 00000100000001
 // 7 1000000010000    + 3 0000100000001
 // frob16 * 1
 auto [A2, A3]= unpack(mul2(frob16_2lane(A6, A7), A23));
 A01= mul2(frob16_2lane(A4, A5), A01);
 // 0100010001000100 + 0001000100010001
 // 100010001000100  + 001000100010001
 // frob8 * 1
 auto [A0, A1]= unpack(mul2(frob8_2lane(A2, A3), A01));
 // 1010101010101010 + 0101010101010101
 return mul(frob4(A1), A0);
}
}  // namespace gf2_64_pow_byte_window_6_2
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as, const vector<u64>& es) {
  using gf2_64_pow_byte_window_6_2::pow;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= pow(as[i], es[i]);
  return ans;
 }
};
