#pragma once
// a^r0 (r0 < 2^32) を窓で回す部分 (提出ではない)。組み方の違う 6 通りを置いてある。
//
// どれも a^r0 = Π_j frob_{w j}(T[d_j]) (d_j は r0 の w bit ごとの桁, T[i] = a^i) を、
// 2 つずつ frob で束ねる木にして潰す。返すのは最後に掛ける 2 因子で、積が a^r0。
// 部分体側の 2 因子と一緒に mul2 へ渡せるよう、掛け合わせる前の形にしてある。
//
// 桁が細いほど T の用意が軽くなり、木の段が増えて frob の表引きが増える。
// 葉に掛かる frob は「frob(a) の冪表」に置き換えられる (frob_k(T[d]) = frob_k(a)^d) ので、
// 基底を増やして木の下の段の frob を消す形も試せる。表引きと乗算のどちらが高く付くかは
// 環境によって違うはずで、そこを測るために並べてある:
//   win32_4bit       T[0..15] を mul2 7 本で用意、木は mul2 3 本 + frob 2lane 3 本
//   win32_4bit_sq    同じ木で T の偶数冪を sq で作る (mul2 3 本 + sq 7 回)
//   win32_3bit       T[0..7] を mul2 2 本で用意、木は mul2 5 本 + frob 2lane 5 本
//   win32_2bit       T[0..3] を mul 1 本で用意、木は mul2 7 本 + frob 2lane 7 本
//   win32_2bit_dual  基底 2 本 (a, frob16(a))、木は mul2 7 本 + frob 2lane 3 本
//   win32_2bit_quad  基底 4 本 (a, frob8(a), frob16(a), frob24(a))、frob 2lane 1 本
#include "_subfield32.hpp"
namespace gf2_64_pow_subfield32 {
// 4 bit × 8 桁。T[0..15] の組み方は byte_window_6_5 と同じ。
inline pair<u64, u64> win32_4bit(u64 a, u32 r0) {
 u64 T[16]= {1, a, sq(a)};
 __m256i T12= _mm256_set_epi64x(0, T[2], 0, a);
 __m256i T34= mul2(T12, _mm256_set1_epi64x(T[2]));
 tie(T[3], T[4])= unpack(T34);
 __m256i T4= _mm256_set1_epi64x(T[4]);
 __m256i T56= mul2(T4, T12);
 tie(T[5], T[6])= unpack(T56);
 tie(T[7], T[8])= unpack(mul2(T4, T34));
 __m256i T8= _mm256_set1_epi64x(T[8]);
 tie(T[9], T[10])= unpack(mul2(T8, T12));
 tie(T[11], T[12])= unpack(mul2(T8, T34));
 tie(T[13], T[14])= unpack(mul2(T8, T56));
 T[15]= mul(T[7], T[8]);
 // 4 00010000 + 0 00000001     01000100 + 00010001     10101010 + 01010101
 // 5 0010000  + 1 0000001      1000100  + 0010001
 // 6 010000   + 2 000001    →                       →
 // 7 10000    + 3 00001
 // frob16 * 1                  frob8 * 1               frob4 * 1
 __m256i A01= mul2(frob16_2lane(T[(r0 >> 16) & 0xF], T[(r0 >> 20) & 0xF]), _mm256_set_epi64x(0, T[(r0 >> 4) & 0xF], 0, T[r0 & 0xF]));
 auto [A2, A3]= unpack(mul2(frob16_2lane(T[(r0 >> 24) & 0xF], T[(r0 >> 28) & 0xF]), _mm256_set_epi64x(0, T[(r0 >> 12) & 0xF], 0, T[(r0 >> 8) & 0xF])));
 auto [B0, B1]= unpack(mul2(frob8_2lane(A2, A3), A01));
 return {B0, gf2_64_pclmul::frob4(B1)};
}
// 4 bit × 8 桁。T[0..15] を偶数は sq、奇数は a 倍で作る版 (mul2 7 本 → 3 本 + sq 7 回)。
inline pair<u64, u64> win32_4bit_sq(u64 a, u32 r0) {
 __m256i av= _mm256_set1_epi64x(a);
 u64 T[16]= {1, a, sq(a)};
 T[4]= sq(T[2]);
 T[8]= sq(T[4]);
 tie(T[3], T[5])= unpack(mul2(_mm256_set_epi64x(0, T[4], 0, T[2]), av));
 T[6]= sq(T[3]);
 T[10]= sq(T[5]);
 T[12]= sq(T[6]);
 tie(T[7], T[9])= unpack(mul2(_mm256_set_epi64x(0, T[8], 0, T[6]), av));
 T[14]= sq(T[7]);
 tie(T[11], T[13])= unpack(mul2(_mm256_set_epi64x(0, T[12], 0, T[10]), av));
 T[15]= mul(T[14], a);
 __m256i A01= mul2(frob16_2lane(T[(r0 >> 16) & 0xF], T[(r0 >> 20) & 0xF]), _mm256_set_epi64x(0, T[(r0 >> 4) & 0xF], 0, T[r0 & 0xF]));
 auto [A2, A3]= unpack(mul2(frob16_2lane(T[(r0 >> 24) & 0xF], T[(r0 >> 28) & 0xF]), _mm256_set_epi64x(0, T[(r0 >> 12) & 0xF], 0, T[(r0 >> 8) & 0xF])));
 auto [B0, B1]= unpack(mul2(frob8_2lane(A2, A3), A01));
 return {B0, gf2_64_pclmul::frob4(B1)};
}
// 3 bit × 11 桁 (33 bit ぶんだが上の桁は 2 bit しか立たない)。位置 j, j+4, j+8 を
// frob12 / frob24 で 1 つにまとめてから、frob6 と frob3 で 4 つを畳む。
inline pair<u64, u64> win32_3bit(u64 a, u32 r0) {
 u64 T[8]= {1, a, sq(a)};
 __m256i T12= _mm256_set_epi64x(0, T[2], 0, a);
 tie(T[3], T[4])= unpack(mul2(T12, _mm256_set1_epi64x(T[2])));
 tie(T[5], T[6])= unpack(mul2(_mm256_set1_epi64x(T[4]), T12));
 T[7]= mul(T[3], T[4]);
 // 位置 8,9,10 (frob24) と 4,5,6,7 (frob12) を先に潰す。10 の桁は 2 bit ぶんしかない。
 __m256i Q01= mul2(frob24_2lane(T[(r0 >> 24) & 7], T[(r0 >> 27) & 7]), frob12_2lane(T[(r0 >> 12) & 7], T[(r0 >> 15) & 7]));
 // 位置 11 は無いので、その lane だけ frob24 の入力を 1 にして素通しさせる
 __m256i Q23= mul2(frob24_2lane(T[(r0 >> 30) & 7], 1), frob12_2lane(T[(r0 >> 18) & 7], T[(r0 >> 21) & 7]));
 // X_j = T[d_j] · (frob12 と frob24 の積)
 auto [X0, X1]= unpack(mul2(Q01, _mm256_set_epi64x(0, T[(r0 >> 3) & 7], 0, T[r0 & 7])));
 auto [X2, X3]= unpack(mul2(Q23, _mm256_set_epi64x(0, T[(r0 >> 9) & 7], 0, T[(r0 >> 6) & 7])));
 auto [Y0, Y1]= unpack(mul2(frob6_2lane(X2, X3), _mm256_set_epi64x(0, X1, 0, X0)));
 return {Y0, gf2_64_pclmul::frob3(Y1)};
}
// 2 bit × 16 桁。T は a^2, a^3 だけ。木は 4 段。
inline pair<u64, u64> win32_2bit(u64 a, u32 r0) {
 const u64 T2= sq(a), T[4]= {1, a, T2, mul(a, T2)};
 __m256i A01= mul2(frob16_2lane(T[(r0 >> 16) & 3], T[(r0 >> 18) & 3]), _mm256_set_epi64x(0, T[(r0 >> 2) & 3], 0, T[r0 & 3]));
 __m256i A23= mul2(frob16_2lane(T[(r0 >> 20) & 3], T[(r0 >> 22) & 3]), _mm256_set_epi64x(0, T[(r0 >> 6) & 3], 0, T[(r0 >> 4) & 3]));
 __m256i A45= mul2(frob16_2lane(T[(r0 >> 24) & 3], T[(r0 >> 26) & 3]), _mm256_set_epi64x(0, T[(r0 >> 10) & 3], 0, T[(r0 >> 8) & 3]));
 auto [A6, A7]= unpack(mul2(frob16_2lane(T[(r0 >> 28) & 3], T[(r0 >> 30) & 3]), _mm256_set_epi64x(0, T[(r0 >> 14) & 3], 0, T[(r0 >> 12) & 3])));
 auto [A4, A5]= unpack(A45);
 __m256i B01= mul2(frob8_2lane(A4, A5), A01);
 auto [B2, B3]= unpack(mul2(frob8_2lane(A6, A7), A23));
 auto [C0, C1]= unpack(mul2(frob4_2lane(B2, B3), B01));
 return {C0, gf2_64_pclmul::frob2(C1)};
}
// 2 bit × 16 桁、基底 2 本。葉に掛かる frob16 は「frob16(a) の冪表」に押し込めるので
// (frob16(T[d]) = frob16(a)^d)、木の 1 段目の frob 4 本が表 1 本の用意に化ける。
inline pair<u64, u64> win32_2bit_dual(u64 a, u32 r0) {
 const u64 b= frob16(a), a2= sq(a), b2= sq(b);
 u64 T[4]= {1, a, a2, 0}, U[4]= {1, b, b2, 0};
 tie(T[3], U[3])= unpack(mul2(_mm256_set_epi64x(0, b2, 0, a2), _mm256_set_epi64x(0, b, 0, a)));
 __m256i A01= mul2(_mm256_set_epi64x(0, U[(r0 >> 18) & 3], 0, U[(r0 >> 16) & 3]), _mm256_set_epi64x(0, T[(r0 >> 2) & 3], 0, T[r0 & 3]));
 __m256i A23= mul2(_mm256_set_epi64x(0, U[(r0 >> 22) & 3], 0, U[(r0 >> 20) & 3]), _mm256_set_epi64x(0, T[(r0 >> 6) & 3], 0, T[(r0 >> 4) & 3]));
 __m256i A45= mul2(_mm256_set_epi64x(0, U[(r0 >> 26) & 3], 0, U[(r0 >> 24) & 3]), _mm256_set_epi64x(0, T[(r0 >> 10) & 3], 0, T[(r0 >> 8) & 3]));
 auto [A6, A7]= unpack(mul2(_mm256_set_epi64x(0, U[(r0 >> 30) & 3], 0, U[(r0 >> 28) & 3]), _mm256_set_epi64x(0, T[(r0 >> 14) & 3], 0, T[(r0 >> 12) & 3])));
 auto [A4, A5]= unpack(A45);
 __m256i B01= mul2(frob8_2lane(A4, A5), A01);
 auto [B2, B3]= unpack(mul2(frob8_2lane(A6, A7), A23));
 auto [C0, C1]= unpack(mul2(frob4_2lane(B2, B3), B01));
 return {C0, gf2_64_pclmul::frob2(C1)};
}
// 2 bit × 16 桁、基底 4 本 (a, frob8(a), frob16(a), frob24(a))。木の 2 段目の frob も
// 基底に押し込む。frob の表引きは frob8 が 3 回と最後の 2 本だけになる。
inline pair<u64, u64> win32_2bit_quad(u64 a, u32 r0) {
 using gf2_64_pclmul::frob8;
 const u64 a1= frob8(a), a2= frob8(a1), a3= frob8(a2);
 u64 T0[4]= {1, a, sq(a), 0}, T1[4]= {1, a1, sq(a1), 0}, T2[4]= {1, a2, sq(a2), 0}, T3[4]= {1, a3, sq(a3), 0};
 tie(T0[3], T1[3])= unpack(mul2(_mm256_set_epi64x(0, T1[2], 0, T0[2]), _mm256_set_epi64x(0, a1, 0, a)));
 tie(T2[3], T3[3])= unpack(mul2(_mm256_set_epi64x(0, T3[2], 0, T2[2]), _mm256_set_epi64x(0, a3, 0, a2)));
 // B_j = T0[d_j] T1[d_{j+4}] T2[d_{j+8}] T3[d_{j+12}]  (j = 0..3)
 __m256i P01= mul2(_mm256_set_epi64x(0, T1[(r0 >> 10) & 3], 0, T1[(r0 >> 8) & 3]), _mm256_set_epi64x(0, T0[(r0 >> 2) & 3], 0, T0[r0 & 3]));
 __m256i P23= mul2(_mm256_set_epi64x(0, T1[(r0 >> 14) & 3], 0, T1[(r0 >> 12) & 3]), _mm256_set_epi64x(0, T0[(r0 >> 6) & 3], 0, T0[(r0 >> 4) & 3]));
 __m256i Q01= mul2(_mm256_set_epi64x(0, T3[(r0 >> 26) & 3], 0, T3[(r0 >> 24) & 3]), _mm256_set_epi64x(0, T2[(r0 >> 18) & 3], 0, T2[(r0 >> 16) & 3]));
 __m256i Q23= mul2(_mm256_set_epi64x(0, T3[(r0 >> 30) & 3], 0, T3[(r0 >> 28) & 3]), _mm256_set_epi64x(0, T2[(r0 >> 22) & 3], 0, T2[(r0 >> 20) & 3]));
 __m256i B01= mul2(P01, Q01);
 auto [B2, B3]= unpack(mul2(P23, Q23));
 auto [C0, C1]= unpack(mul2(frob4_2lane(B2, B3), B01));
 return {C0, gf2_64_pclmul::frob2(C1)};
}
}  // namespace gf2_64_pow_subfield32
