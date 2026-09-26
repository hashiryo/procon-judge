#pragma once
// 基底 64 個をマスクで AND し、4 ビットずつ AVX2 で処理する版。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_frob2_1bit_mask_avx {
// 基底の像 BASIS[i] = frob2(1 << i)。sq を呼ぶのはここだけで、表はこの XOR の
// 足し合わせ (倍々) で広げる。表の形が変わっても作り方は同じ。
constexpr auto BASIS= []() {
 auto spread= [](u32 a) constexpr -> u64 {
  u64 x= a;
  x= (x | (x << 16)) & 0x0000FFFF0000FFFFull;
  x= (x | (x << 8)) & 0x00FF00FF00FF00FFull;
  x= (x | (x << 4)) & 0x0F0F0F0F0F0F0F0Full;
  x= (x | (x << 2)) & 0x3333333333333333ull;
  x= (x | (x << 1)) & 0x5555555555555555ull;
  return x;
 };
 constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 auto sq= [&](u64 a) constexpr -> u64 {
  u64 h= spread(u32(a >> 32));
  u64 d= h ^ (h << 1);
  return spread(u32(a)) ^ RED[h >> 60] ^ d ^ (d << 3);
 };
 array<u64, 64> b{};
 for(int i= 0; i < 64; ++i) b[i]= sq(sq(u64(1) << i));
 return b;
}();
// 基底 64 個 (512 B) を 4 個ずつ 256 bit でまとめて読み、マスクは比較命令で作る版。
// 基底は連続なので普通の 256 bit ロードが効き、表引き (gather) も詰め直しも要らない。
// 1 回の反復で 4 ビットぶんを処理する。
//
// ビットの選び方は「a を 4 ずつずらして broadcast し、固定の {1,2,4,8} と比べる」形に
// してある。反復ごとに違うマスク定数 ({1<<i, ...}) を使うと、定数の組み立てが毎回走る。
inline u64 frob2(u64 a) {
 const __m256i lane= _mm256_setr_epi64x(1, 2, 4, 8);
 __m256i acc= _mm256_setzero_si256();
 for(int i= 0; i < 64; i+= 4) {
  const __m256i m= _mm256_cmpeq_epi64(_mm256_and_si256(_mm256_set1_epi64x(a >> i), lane), lane);
  acc= _mm256_xor_si256(acc, _mm256_and_si256(_mm256_loadu_si256((const __m256i *)&BASIS[i]), m));
 }
 __m128i y= _mm_xor_si128(_mm256_castsi256_si128(acc), _mm256_extracti128_si256(acc, 1));
 return u64(_mm_cvtsi128_si64(y)) ^ u64(_mm_extract_epi64(y, 1));
}
}  // namespace gf2_64_frob2_1bit_mask_avx
inline vector<u64> run(const vector<u64>& as) {
 using gf2_64_frob2_1bit_mask_avx::frob2;
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= frob2(as[i]);
 return ans;
}
