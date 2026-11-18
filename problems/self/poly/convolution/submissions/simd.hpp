#pragma once
// AVX2 だけの素朴な畳み込み (大きさによる切り替えなし)。出力 32 個ずつの塊ごとに a_i を配り、b の 32 個との積を u64 の 8 本の
// レジスタにためる (偶数番目と奇数番目を _mm256_mul_epu32 で別に足し、8 回ごとに 2^32 の位を畳む)。b の前後に 32 個の 0 を置く。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <vector>
namespace conv_simd {
constexpr u32 M= 998244353;
constexpr u32 R32= u32((u64(1) << 32) % M);
constexpr int PAD= 32;
inline __m256i fold(__m256i x) { return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_set1_epi64x(R32)), _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffll))); }
inline void conv32(const u32* a, int n, const u32* bp, int m, int k0, u32* out) {
 const int ilo= std::max(0, k0 - m + 1), ihi= std::min(n - 1, k0 + 31);
 __m256i e0= _mm256_setzero_si256(), e1= e0, e2= e0, e3= e0, o0= e0, o1= e0, o2= e0, o3= e0;
 for(int i= ilo; i <= ihi;) {
  const int iend= std::min(ihi + 1, i + 8);
  for(; i < iend; ++i) {
   const __m256i x= _mm256_set1_epi32(a[i]);
   const u32* q= bp + PAD + k0 - i;
   const __m256i b0= _mm256_loadu_si256((const __m256i*)q), b1= _mm256_loadu_si256((const __m256i*)(q + 8));
   const __m256i b2= _mm256_loadu_si256((const __m256i*)(q + 16)), b3= _mm256_loadu_si256((const __m256i*)(q + 24));
   e0= _mm256_add_epi64(e0, _mm256_mul_epu32(x, b0)), o0= _mm256_add_epi64(o0, _mm256_mul_epu32(x, _mm256_srli_epi64(b0, 32)));
   e1= _mm256_add_epi64(e1, _mm256_mul_epu32(x, b1)), o1= _mm256_add_epi64(o1, _mm256_mul_epu32(x, _mm256_srli_epi64(b1, 32)));
   e2= _mm256_add_epi64(e2, _mm256_mul_epu32(x, b2)), o2= _mm256_add_epi64(o2, _mm256_mul_epu32(x, _mm256_srli_epi64(b2, 32)));
   e3= _mm256_add_epi64(e3, _mm256_mul_epu32(x, b3)), o3= _mm256_add_epi64(o3, _mm256_mul_epu32(x, _mm256_srli_epi64(b3, 32)));
  }
  e0= fold(e0), e1= fold(e1), e2= fold(e2), e3= fold(e3), o0= fold(o0), o1= fold(o1), o2= fold(o2), o3= fold(o3);
 }
 alignas(32) u64 ev[16], od[16];
 _mm256_store_si256((__m256i*)ev, e0), _mm256_store_si256((__m256i*)(ev + 4), e1), _mm256_store_si256((__m256i*)(ev + 8), e2), _mm256_store_si256((__m256i*)(ev + 12), e3);
 _mm256_store_si256((__m256i*)od, o0), _mm256_store_si256((__m256i*)(od + 4), o1), _mm256_store_si256((__m256i*)(od + 8), o2), _mm256_store_si256((__m256i*)(od + 12), o3);
 for(int j= 0; j < 16; ++j) out[2 * j]= u32(ev[j] % M), out[2 * j + 1]= u32(od[j] % M);
}
}  // namespace conv_simd
inline vector<u32> run(const vector<u32>& a, const vector<u32>& b) {
 using namespace conv_simd;
 const int n= int(a.size()), m= int(b.size()), len= n + m - 1;
 static thread_local vector<u32> bp;
 bp.assign(m + 2 * PAD, 0);
 std::copy(b.begin(), b.end(), bp.begin() + PAD);
 vector<u32> c(len + 32);
 for(int k0= 0; k0 < len; k0+= 32) conv32(a.data(), n, bp.data(), m, k0, c.data() + k0);
 c.resize(len);
 return c;
}
