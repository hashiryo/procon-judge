#pragma once
// sq2(v) = mul2(v, v) を、128 bit の pclmulqdq 2 回で作る版。lane ごとに取り出して掛け、setr_m128i で 256 bit に戻してから、
// reduce は __m256i のまま 2 lane 並列でやる。本体は Library の GF2p64 の mul2<0> (arm と、VPCLMULQDQ の無い x86 で
// 使う形) と同じ。
#include "_shared/gf2-64/_common.hpp"
inline __m256i sq2(__m256i v) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 const __m128i lo= _mm256_castsi256_si128(v), hi= _mm256_extracti128_si256(v, 1);
 __m256i prod= _mm256_setr_m128i(_mm_clmulepi64_si128(lo, lo, 0), _mm_clmulepi64_si128(hi, hi, 0));
 __m256i h= _mm256_srli_si256(prod, 8);
 __m256i d= _mm256_xor_si256(h, _mm256_slli_epi64(h, 1));
 return _mm256_xor_si256(_mm256_xor_si256(prod, _mm256_shuffle_epi8(RED256, _mm256_srli_epi64(h, 60))), _mm256_xor_si256(d, _mm256_slli_epi64(d, 3)));
}
