#pragma once
// GF(2^64) の current best mul: GNU_TARGET("pclmul") + 多項式 P(x) = x^64+x^4+x^3+x+1 で reduce。
//
// 利用側ルール: gf2-64-mul/algos/* は本ファイルを使ってはいけない (mul の比較対象なので)。
// それ以外の problem (sq/div/pow/sqrt/log) は building block として使用 OK。
#include "_common.hpp"
namespace gf2_64_pclmul {
const __m256i RED_TABLE= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
GNU_TARGET("vpclmulqdq") inline __m256i mul2(const __m256i& a_vec, const __m256i& b_vec) {
 __m256i prod= _mm256_clmulepi64_epi128(a_vec, b_vec, 0);
 __m256i d_full= _mm256_xor_si256(prod, _mm256_slli_epi64(prod, 1));
 __m256i red1_shift= _mm256_srli_si256(_mm256_xor_si256(d_full, _mm256_slli_epi64(d_full, 3)), 8);
 __m256i indices= _mm256_srli_si256(_mm256_srli_epi64(prod, 60), 8);
 return _mm256_xor_si256(_mm256_xor_si256(prod, red1_shift), _mm256_shuffle_epi8(RED_TABLE, indices));
}
inline pair<u64, u64> unpack(const __m256i& vec) { return make_pair(u64(_mm256_extract_epi64(vec, 0)), u64(_mm256_extract_epi64(vec, 2))); }
}
