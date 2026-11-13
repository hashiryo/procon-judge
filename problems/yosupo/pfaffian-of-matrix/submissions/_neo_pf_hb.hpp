#pragma once
// 確かめ用。NeoLibrary 60945a2 の pfaffian_internal::zmod_v (neo/linalg/pfaffian.hpp) の写しで、作業の行列 a だけを
// linalg_internal::hvec<u64> (値の初期化で 0 を書いてから写す) から HugeBuf<u64> (huge page を頼んで先にページを用意させ、
// 0 では埋めない) に替えたもの。入力は src(i, j) で受ける (Matrix<Z> からでも vector<vector<u32>> からでも読める)。
#include "../common.hpp"
#include "neo/linalg/pfaffian.hpp"
#include "_shared/linalg/lazy_elim_v.hpp"
namespace neo_pf_hb {
using namespace pfaffian_internal;
template <class Z, class Src> Z zmod_v(int n, Src src) {
 const int w= (n + 3) & ~3, n4= w;
 const u32 M= Z::mod();
 const u64 R= (u64(1) << 32) % M;
 const Mont mt(M);
 lazy_elim_v::HugeBuf<u64> a((size_t)n * w + 8);
 for(int i= 0; i < n; ++i) {
  u64* r= a.p + (size_t)i * w;
  for(int j= 0; j < n; ++j) r[j]= src(i, j);
  for(int j= n; j < w; ++j) r[j]= 0;
 }
 std::fill(a.p + (size_t)n * w, a.p + (size_t)n * w + 8, 0ull);
 auto row= [&](int r) { return a.p + (size_t)r * w; };
 std::vector<int> pos(n);
 std::iota(pos.begin(), pos.end(), 0);
 std::vector<u64> C((size_t)K * n4), U((size_t)K * w + 8), zero(w + 8);
 std::vector<u32> F((size_t)K * n4), V(n4);
 const u64* Ub[K];
 Z res= Z::raw(1);
 int bt= 0, first= 1, c0= -1000, ne= 0;
 auto fill_cache= [&](int cs) {
  c0= cs;
  for(int t= first & ~3; t < n; t+= 4) {
   const u64* r0= row(pos[t]) + cs;
   const u64* r1= t + 1 < n ? row(pos[t + 1]) + cs : zero.data();
   const u64* r2= t + 2 < n ? row(pos[t + 2]) + cs : zero.data();
   const u64* r3= t + 3 < n ? row(pos[t + 3]) + cs : zero.data();
   for(int h= 0; h < 8; h+= 4) {
    const __m256i x0= _mm256_loadu_si256((const __m256i*)(r0 + h)), x1= _mm256_loadu_si256((const __m256i*)(r1 + h));
    const __m256i x2= _mm256_loadu_si256((const __m256i*)(r2 + h)), x3= _mm256_loadu_si256((const __m256i*)(r3 + h));
    const __m256i t0= _mm256_unpacklo_epi64(x0, x1), t1= _mm256_unpackhi_epi64(x0, x1);
    const __m256i t2= _mm256_unpacklo_epi64(x2, x3), t3= _mm256_unpackhi_epi64(x2, x3);
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 0) * n4 + t], _mm256_permute2x128_si256(t0, t2, 0x20));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 1) * n4 + t], _mm256_permute2x128_si256(t1, t3, 0x20));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 2) * n4 + t], _mm256_permute2x128_si256(t0, t2, 0x31));
    _mm256_storeu_si256((__m256i*)&C[(size_t)(h + 3) * n4 + t], _mm256_permute2x128_si256(t1, t3, 0x31));
   }
  }
 };
 auto flush= [&](int col) {
  if(!bt) return;
  for(int s= bt; s < K; ++s) Ub[s]= zero.data();
  for(int t= first; t < n; ++t) {
   u32 f[K], any= 0;
   for(int s= 0; s < K; ++s) any|= (f[s]= s < bt ? F[(size_t)s * n4 + t] : 0);
   if(any) update_row(row(pos[t]), Ub, f, col & ~3, w, R);
  }
  for(int s= 0; s < bt; ++s) std::fill(&F[(size_t)s * n4], &F[(size_t)s * n4] + n4, 0u);
  bt= 0, c0= -1000;
 };
 for(int c= 0; c + 1 < n; ++c) {
  if(c < c0 || c >= c0 + 8) fill_cache(c);
  const u64* cc= &C[(size_t)(c - c0) * n4];
  __m256i us[K];
  for(int s= 0; s < bt; ++s) us[s]= _mm256_set1_epi64x(Ub[s][c]);
  int sel= -1;
  for(int t= first & ~3; t < n4; t+= 4) {
   __m256i x= _mm256_loadu_si256((const __m256i*)(cc + t));
   for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&F[(size_t)s * n4 + t])), us[s]));
   const __m256i y= mt.redc(x);
   _mm_storeu_si128((__m128i*)&V[t], pack_lo(y));
   if(sel < 0) {
    int nz= ~_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(y, _mm256_setzero_si256()))) & 15;
    if(t < first) nz&= ~((1 << (first - t)) - 1);
    if(nz) sel= t + __builtin_ctz(nz);
   }
  }
  if(sel >= n) sel= -1;
  if(sel < 0) {
   if(!(c & 1)) return Z();
   first= c + 2;
   continue;
  }
  if(sel != c + 1) {
   std::swap(pos[c + 1], pos[sel]), std::swap(V[c + 1], V[sel]);
   for(int q= 0; q < K; ++q) std::swap(C[(size_t)q * n4 + c + 1], C[(size_t)q * n4 + sel]);
   for(int s= 0; s < bt; ++s) std::swap(F[(size_t)s * n4 + c + 1], F[(size_t)s * n4 + sel]);
   for(int t= c + 1; t < n; ++t) {
    u64* r= row(pos[t]);
    std::swap(r[c + 1], r[sel]);
   }
   for(int s= 0; s < bt; ++s) std::swap(U[(size_t)s * w + c + 1], U[(size_t)s * w + sel]);
   const int q1= c + 1 - c0, q2= sel - c0;
   if(q1 < 8 && q2 < 8) {
    for(int t= first & ~3; t < n4; ++t) std::swap(C[(size_t)q1 * n4 + t], C[(size_t)q2 * n4 + t]);
   } else if(q1 < 8) {
    for(int t= first; t < n; ++t) C[(size_t)q1 * n4 + t]= row(pos[t])[c + 1];
   }
   res= -res;
  }
  const int p= pos[c + 1];
  if(!(c & 1)) res*= -Z::raw(V[c + 1]), ++ne;
  u64* up= &U[(size_t)bt * w];
  {
   __m256i fs[K];
   for(int s= 0; s < bt; ++s) fs[s]= _mm256_set1_epi64x(F[(size_t)s * n4 + c + 1]);
   const u64* ap= row(p);
   for(int j= c & ~3; j < w; j+= 4) {
    __m256i x= _mm256_loadu_si256((const __m256i*)(ap + j));
    for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(Ub[s] + j)), fs[s]));
    _mm256_storeu_si256((__m256i*)(up + j), mt.redc(x));
   }
  }
  {
   const u32 ci= (Z::raw(V[c + 1]).inv() * Z::raw(u32(R))).val(), cs= u32((u64(ci) << 32) / M);
   const __m256i Cv= _mm256_set1_epi64x(ci), CS= _mm256_set1_epi64x(cs);
   u32* fb= &F[(size_t)bt * n4];
   for(int t= (c + 2) & ~3; t < n4; t+= 4) {
    const __m256i vv= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&V[t]));
    const __m256i qq= _mm256_srli_epi64(_mm256_mul_epu32(vv, CS), 32);
    __m256i x= _mm256_sub_epi64(_mm256_mul_epu32(vv, Cv), _mm256_mul_epu32(qq, mt.M));
    x= _mm256_min_epu32(x, _mm256_sub_epi32(x, mt.M32));
    x= _mm256_sub_epi32(mt.M32, x);
    x= _mm256_min_epu32(x, _mm256_sub_epi32(x, mt.M32));
    _mm_storeu_si128((__m128i*)(fb + t), pack_lo(x));
   }
  }
  Ub[bt++]= up;
  first= c + 2;
  if(bt == K) flush(c + 1);
 }
 return res * Z(R).pow(ne);
}
}  // namespace neo_pf_hb
