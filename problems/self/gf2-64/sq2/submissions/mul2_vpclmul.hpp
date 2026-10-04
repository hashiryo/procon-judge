#pragma once
// sq2(v) = mul2(v, v)。256 bit の vpclmulqdq 1 回で 2 lane の a·a を作り、reduce も __m256i のまま 2 lane 並列でやる。
// 本体は Library の GF2p64 の mul2<1> (x86 で VPCLMULQDQ があるときに使う形) と同じ。arm では SIMDe が 256 bit の
// clmul を遅い形で組むので、Library は arm では mul2<0> (mul2_pclmul.hpp と同じ形) を使う。
#include "_shared/gf2-64/_common.hpp"
inline __m256i sq2(__m256i v) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 __m256i prod= _mm256_clmulepi64_epi128(v, v, 0);
 __m256i h= _mm256_srli_si256(prod, 8);
 __m256i d= _mm256_xor_si256(h, _mm256_slli_epi64(h, 1));
 return _mm256_xor_si256(_mm256_xor_si256(prod, _mm256_shuffle_epi8(RED256, _mm256_srli_epi64(h, 60))), _mm256_xor_si256(d, _mm256_slli_epi64(d, 3)));
}
