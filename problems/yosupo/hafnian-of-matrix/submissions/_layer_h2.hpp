#pragma once
// zmod_layer_h2 の中身。zmod_layer_h (_layer_h.hpp) の層 8 以上の核で、積和の和を 5 本ずつに分けて回す。
// zmod_layer_h は列の向きの和を最大 10 本 (40 列) まとめて持っていて、係数の行と定数を合わせると ymm の 16 本を超え、x64 の
// GCC 15 が和の一部と係数の行をループの中でスタックに退避していた。和を前の 5 本と後ろの 5 本に分け、T ごとに組の並びを 2 回
// たどる (D の値の読み込みは 2 倍になるが、1 個ずつの読み込みなので軽い)。1 回の和は 5 本以下なので、和と係数と定数が
// レジスタに収まる。
#include "_layer_h.hpp"
namespace hafnian_layer_h2 {
using namespace hafnian_layer;
#define HAFH2_REP(X) X(0) X(1) X(2) X(3) X(4)
// 列の向きの和のうち、ベクトル [Q0, Q0 + NQ) (NQ <= 5) を求めて v に書く。A2 の行は W 個の u64 (W = NV * 4)。
template <int W, int Q0, int NQ> [[gnu::always_inline]] inline void pass(u32 T, u32 TL, int s, const int* thp, int nth, const u64* __restrict A2, const u64* __restrict aw, const u32* __restrict D, size_t stride, const u32* __restrict c, u64* __restrict v) {
 const __m256i Mv= _mm256_set1_epi32(M), M2= _mm256_set1_epi32(2 * M), TOP= _mm256_set1_epi64x((long long)(1ull << 63)), NI= _mm256_set1_epi32(NIV);
 __m256i a0, a1, a2, a3, a4;
 {
  const __m256i x= _mm256_set1_epi32(c[T]);
#define HAFH2_INIT(q) \
 if constexpr(NQ > q) a##q= _mm256_mul_epu32(x, _mm256_load_si256((const __m256i*)(aw + 4 * (Q0 + q))));
  HAFH2_REP(HAFH2_INIT)
#undef HAFH2_INIT
 }
 int cnt= 1;
#define HAFH2_KEEP(q) \
 if constexpr(NQ > q) a##q= _mm256_sub_epi64(a##q, _mm256_min_epu32(M2, _mm256_and_si256(TOP, a##q)));
#define HAFH2_MAC(q) \
 if constexpr(NQ > q) a##q= _mm256_add_epi64(a##q, _mm256_add_epi64(_mm256_mul_epu32(x0, _mm256_load_si256((const __m256i*)(r0 + 4 * (Q0 + q)))), _mm256_mul_epu32(x1, _mm256_load_si256((const __m256i*)(r1 + 4 * (Q0 + q))))));
 for(u32 t= TL; t; t&= t - 1) {
  const int p= __builtin_ctz(t);
  if(cnt > 6) {
   HAFH2_REP(HAFH2_KEEP)
   cnt= 0;
  }
  const u32 ix= drop_bit(T, p);
  const __m256i x0= _mm256_set1_epi32(D[size_t(2 * p) * stride + ix]), x1= _mm256_set1_epi32(D[size_t(2 * p + 1) * stride + ix]);
  const u64 *r0= A2 + size_t(2 * p) * W, *r1= r0 + W;
  HAFH2_REP(HAFH2_MAC)
  cnt+= 2;
 }
 for(int i= 0; i < nth; ++i) {
  const int p= thp[i];
  if(cnt > 6) {
   HAFH2_REP(HAFH2_KEEP)
   cnt= 0;
  }
  const u32 ix= drop_bit(T, p);
  const __m256i x0= _mm256_set1_epi32(D[size_t(2 * p) * stride + ix]), x1= _mm256_set1_epi32(D[size_t(2 * p + 1) * stride + ix]);
  const u64 *r0= A2 + size_t(2 * s + 2 * i) * W, *r1= r0 + W;
  HAFH2_REP(HAFH2_MAC)
  cnt+= 2;
 }
#undef HAFH2_MAC
 HAFH2_REP(HAFH2_KEEP)
#undef HAFH2_KEEP
#define HAFH2_RED(q) \
 if constexpr(NQ > q) { \
  __m256i r= _mm256_srli_epi64(_mm256_add_epi64(a##q, _mm256_mul_epu32(_mm256_mul_epu32(a##q, NI), Mv)), 32); \
  r= _mm256_min_epu32(r, _mm256_sub_epi32(r, M2)); \
  _mm256_store_si256((__m256i*)(v + 4 * (Q0 + q)), _mm256_min_epu32(r, _mm256_sub_epi32(r, Mv))); \
 }
 HAFH2_REP(HAFH2_RED)
#undef HAFH2_RED
}
#undef HAFH2_REP
// _layer_h.hpp の layer_h と同じ入出力で、和を 5 本ずつ 2 回に分けて求める。
template <int NV> [[gnu::noinline]] void layer_h2(int w, int s, u32 THbits, const int* thp, int nth, const int* chp, int nch, const u64* __restrict A2, u32* __restrict D, size_t stride, u32* __restrict c) {
 constexpr int W= NV * 4;
 const u32 nl= 1u << s;
 const u64* aw= A2 + size_t(2 * s + 2 * nth) * W;
 const int K2= 2 * s + 2 * nch;
 alignas(32) u64 v[W];
 for(u32 TL= 0; TL < nl; ++TL) {
  const u32 T= THbits | TL;
  pass<W, 0, (NV < 5 ? NV : 5)>(T, TL, s, thp, nth, A2, aw, D, stride, c, v);
  if constexpr(NV > 5) pass<W, 5, NV - 5>(T, TL, s, thp, nth, A2, aw, D, stride, c, v);
  c[(1u << w) + T]= u32(v[K2]);
  for(u32 t= ~TL & (nl - 1); t; t&= t - 1) {
   const int p= __builtin_ctz(t);
   const u32 ix= drop_bit(T, p);
   D[size_t(2 * p + 1) * stride + ix]= u32(v[2 * p]);
   D[size_t(2 * p) * stride + ix]= u32(v[2 * p + 1]);
  }
  for(int i= 0; i < nch; ++i) {
   const int p= chp[i];
   const u32 ix= drop_bit(T, p);
   D[size_t(2 * p + 1) * stride + ix]= u32(v[2 * s + 2 * i]);
   D[size_t(2 * p) * stride + ix]= u32(v[2 * s + 2 * i + 1]);
  }
 }
}
// _layer_h.hpp の hafnian と同じ手順で、層 8 以上の核を layer_h2 にしたもの。
inline u32 hafnian(int N, const vector<vector<u32>>& a) {
 constexpr int W0= 8;
 const int h= N / 2;
 if(h == 0) return 1;
 const size_t stride= max<size_t>(1, size_t(1) << max(0, h - 2));
 Region D(size_t(max(1, 2 * (h - 1))) * stride), C(size_t(1) << h);
 u32* c= C.p;
 c[0]= 1;
 vector<u64> Am((size_t)N * N);
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < N; ++j) Am[(size_t)i * N + j]= (u64(a[i][j]) << 32) % M;
 vector<u64> A((size_t)(N + 1) * 40 + 4);
 u64* Ap= reinterpret_cast<u64*>((reinterpret_cast<uintptr_t>(A.data()) + 31) & ~uintptr_t(31));
#define HAFH2_REP(X) X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9)
 for(int w= 0; w < h; ++w) {
  if(w < W0) {
   const int nv= (2 * w + 2 + 3) / 4;
   for(int e= 0; e <= 2 * w; ++e)
    for(int k= 0; k < nv * 4; ++k) Ap[size_t(e) * nv * 4 + k]= k < 2 * w + 2 ? Am[(size_t)e * N + k] : 0;
   switch(nv) {
#define HAFH2_CASE(q) \
 case q + 1: layer<true, q + 1>(w, Ap, D.p, stride, c); break;
    HAFH2_REP(HAFH2_CASE)
#undef HAFH2_CASE
   }
   continue;
  }
  const int s= w / 2, nh= w - s;
  int thp[20], chp[20], cols[48];
  for(u32 TH= 0; TH < (1u << nh); ++TH) {
   int nth= 0, nch= 0;
   for(int i= 0; i < nh; ++i) (TH >> i & 1 ? thp[nth++] : chp[nch++])= s + i;
   int K= 0;
   for(int k= 0; k < 2 * s; ++k) cols[K++]= k;
   for(int i= 0; i < nch; ++i) cols[K++]= 2 * chp[i], cols[K++]= 2 * chp[i] + 1;
   cols[K++]= 2 * w + 1;
   const int nv= (K + 3) / 4, Wd= nv * 4;
   auto put= [&](int r, int e) {
    u64* dst= Ap + size_t(r) * Wd;
    for(int q= 0; q < K; ++q) dst[q]= Am[(size_t)e * N + cols[q]];
    for(int q= K; q < Wd; ++q) dst[q]= 0;
   };
   int r= 0;
   for(int e= 0; e < 2 * s; ++e) put(r++, e);
   for(int i= 0; i < nth; ++i) put(r++, 2 * thp[i]), put(r++, 2 * thp[i] + 1);
   put(r++, 2 * w);
   switch(nv) {
#define HAFH2_CASE(q) \
 case q + 1: layer_h2<q + 1>(w, s, TH << s, thp, nth, chp, nch, Ap, D.p, stride, c); break;
    HAFH2_REP(HAFH2_CASE)
#undef HAFH2_CASE
   }
  }
 }
#undef HAFH2_REP
 return c[(size_t(1) << h) - 1];
}
}  // namespace hafnian_layer_h2
