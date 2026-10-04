#pragma once
// 今の log-any と同じ形。lane から 2 つの値を取り出してスカラの sq を 2 回呼び、set_epi64x で __m256i に詰め直す。
// sq の本体は Library の GF2p64 の sq と同じで、pshufb で 1 bit を 2 bit に広げ、上 2 bit で引く表と shift で reduce する。
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_sq2_scalar {
inline u64 sq(u64 a) {
 static constexpr u8 RED_SQ[4]= {0, 27, 90, 65};
 const __m128i MASK_LO= _mm_set1_epi8(0x0f);
 const __m128i SPR= _mm_setr_epi8(0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f, (char)0xc0, (char)0xc3, (char)0xcc, (char)0xcf, (char)0xf0, (char)0xf3, (char)0xfc, (char)0xff);
 __m128i v= _mm_set_epi64x(0, a);
 __m128i x= _mm_shuffle_epi8(SPR, _mm_and_si128(_mm_unpacklo_epi8(v, _mm_srli_epi16(v, 4)), MASK_LO));
 u64 d= x[1];
 return (x[0] & 0x5555555555555555) ^ RED_SQ[a >> 62] ^ d ^ (d << 3);
}
}  // namespace gf2_64_sq2_scalar
inline __m256i sq2(__m256i v) {
 using gf2_64_sq2_scalar::sq;
 const u64 a= u64(_mm256_extract_epi64(v, 0)), b= u64(_mm256_extract_epi64(v, 2));
 return _mm256_set_epi64x(0, (long long)sq(b), 0, (long long)sq(a));
}
