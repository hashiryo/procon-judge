#pragma once
// スカラの sq (Library の GF2p64 の sq。pshufb で 1 bit を 2 bit に広げる) を、そのまま 256 bit に広げた版。AVX2 の unpack と
// pshufb は 128 bit の lane ごとに動くので、qword 0 と 2 に置いた 2 つの値を、取り出さずに同じ命令で 2 つ同時に広げられる。
// 広げると、各 lane の下位 qword に下 32 bit の二乗の各 bit を隣の bit に複製したものが、上位 qword に d = h ^ (h << 1)
// (h は上 32 bit の二乗) が並ぶ。答えは (下位 & 0x5555...) ^ d ^ (d << 3) ^ RED_SQ[a >> 62] で、スカラの sq と同じ式。
// RED_SQ も pshufb で 2 lane 同時に引く。添字は入力の上 2 bit から取るので、待ちの鎖には乗らない。
#include "_shared/gf2-64/_common.hpp"
inline __m256i sq2(__m256i v) {
 const __m256i MASK_LO= _mm256_set1_epi8(0x0f), EVEN= _mm256_set1_epi64x(0x5555555555555555);
 const __m256i SPR= _mm256_setr_epi8(0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f, (char)0xc0, (char)0xc3, (char)0xcc, (char)0xcf, (char)0xf0, (char)0xf3, (char)0xfc, (char)0xff, 0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f, (char)0xc0, (char)0xc3, (char)0xcc, (char)0xcf, (char)0xf0, (char)0xf3, (char)0xfc, (char)0xff);
 const __m256i RED_SQ= _mm256_setr_epi8(0, 27, 90, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 90, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
 const __m256i x= _mm256_shuffle_epi8(SPR, _mm256_and_si256(_mm256_unpacklo_epi8(v, _mm256_srli_epi16(v, 4)), MASK_LO));
 const __m256i d= _mm256_srli_si256(x, 8);
 const __m256i red= _mm256_shuffle_epi8(RED_SQ, _mm256_srli_epi64(v, 62));
 return _mm256_xor_si256(_mm256_xor_si256(_mm256_and_si256(x, EVEN), red), _mm256_xor_si256(d, _mm256_slli_epi64(d, 3)));
}
