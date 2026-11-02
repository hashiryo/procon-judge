#pragma once
// zmod_layer_h の中身。zmod_layer_c (_layer.hpp) の層ごとの DP で、掛ける列を使う列だけに詰める。
// 層 w の T のうち、配る先は T に入らない組の頂点の列と 2w+1 の列だけなのに、zmod_layer_c は 2w + 2 列をすべて掛けていて、
// 積和の半分ほどを捨てていた。そこで組を下の s 個 (s = w / 2) と上の w - s 個に分け、上の組の部分集合 TH ごとに、
// 下の組の頂点の列 (2s 列) と、TH に入らない上の組の頂点の列と、2w+1 の列だけを並べた隣接行列 A2 を作り直す。行も、下の組の
// 頂点、TH の組の頂点、2w だけを並べる。TH を決めて下の組の部分集合 TL (2^s 個) を回すので、作り直す手間は 2^s 個の T で
// 割られる。層 18 では掛ける列が平均 40 列から 28 列ほどになる。層が 8 未満なら _layer.hpp の形 (全部の列) にする。
#include "_layer.hpp"
namespace hafnian_layer_h {
using namespace hafnian_layer;
#define HAFH_REP(X) X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9)
// 層 w で上の組の部分集合を THbits (bit s..w-1) に決めたときの、下の組の部分集合 TL の処理。A2 は行ごとに NV * 4 個の u64 で、
// 行は 0..2s-1 が下の組の頂点、2s + 2i と 2s + 2i + 1 が TH の i 番目の組 thp[i] の頂点、最後の行が 2w。列は 0..2s-1 が下の組の
// 頂点、2s + 2i と 2s + 2i + 1 が TH に入らない i 番目の組 chp[i] の頂点、2s + 2 nch が 2w+1。D は組 p の頂点の配列を、T から
// bit p を抜いた番号で引く。
template <int NV> [[gnu::noinline]] void layer_h(int w, int s, u32 THbits, const int* thp, int nth, const int* chp, int nch, const u64* __restrict A2, u32* __restrict D, size_t stride, u32* __restrict c) {
 const __m256i Mv= _mm256_set1_epi32(M), M2= _mm256_set1_epi32(2 * M), TOP= _mm256_set1_epi64x((long long)(1ull << 63)), NI= _mm256_set1_epi32(NIV);
 const u32 nl= 1u << s;
 const u64* aw= A2 + size_t(2 * s + 2 * nth) * NV * 4;
 const int K2= 2 * s + 2 * nch;
 alignas(32) u64 v[NV * 4];
 for(u32 TL= 0; TL < nl; ++TL) {
  const u32 T= THbits | TL;
  __m256i a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
  {
   const __m256i x= _mm256_set1_epi32(c[T]);
#define HAFH_INIT(q) \
 if constexpr(NV > q) a##q= _mm256_mul_epu32(x, _mm256_load_si256((const __m256i*)(aw + 4 * q)));
   HAFH_REP(HAFH_INIT)
#undef HAFH_INIT
  }
  int cnt= 1;
#define HAFH_KEEP(q) \
 if constexpr(NV > q) a##q= _mm256_sub_epi64(a##q, _mm256_min_epu32(M2, _mm256_and_si256(TOP, a##q)));
#define HAFH_MAC(q) \
 if constexpr(NV > q) a##q= _mm256_add_epi64(a##q, _mm256_add_epi64(_mm256_mul_epu32(x0, _mm256_load_si256((const __m256i*)(r0 + 4 * q))), _mm256_mul_epu32(x1, _mm256_load_si256((const __m256i*)(r1 + 4 * q)))));
  for(u32 t= TL; t; t&= t - 1) {
   const int p= __builtin_ctz(t);
   if(cnt > 6) {
    HAFH_REP(HAFH_KEEP)
    cnt= 0;
   }
   const u32 ix= drop_bit(T, p);
   const __m256i x0= _mm256_set1_epi32(D[size_t(2 * p) * stride + ix]), x1= _mm256_set1_epi32(D[size_t(2 * p + 1) * stride + ix]);
   const u64 *r0= A2 + size_t(2 * p) * NV * 4, *r1= r0 + NV * 4;
   HAFH_REP(HAFH_MAC)
   cnt+= 2;
  }
  for(int i= 0; i < nth; ++i) {
   const int p= thp[i];
   if(cnt > 6) {
    HAFH_REP(HAFH_KEEP)
    cnt= 0;
   }
   const u32 ix= drop_bit(T, p);
   const __m256i x0= _mm256_set1_epi32(D[size_t(2 * p) * stride + ix]), x1= _mm256_set1_epi32(D[size_t(2 * p + 1) * stride + ix]);
   const u64 *r0= A2 + size_t(2 * s + 2 * i) * NV * 4, *r1= r0 + NV * 4;
   HAFH_REP(HAFH_MAC)
   cnt+= 2;
  }
#undef HAFH_MAC
  HAFH_REP(HAFH_KEEP)
#undef HAFH_KEEP
#define HAFH_RED(q) \
 if constexpr(NV > q) { \
  __m256i r= _mm256_srli_epi64(_mm256_add_epi64(a##q, _mm256_mul_epu32(_mm256_mul_epu32(a##q, NI), Mv)), 32); \
  r= _mm256_min_epu32(r, _mm256_sub_epi32(r, M2)); \
  _mm256_store_si256((__m256i*)(v + 4 * q), _mm256_min_epu32(r, _mm256_sub_epi32(r, Mv))); \
 }
  HAFH_REP(HAFH_RED)
#undef HAFH_RED
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
inline u32 hafnian(int N, const vector<vector<u32>>& a) {
 constexpr int W0= 8;  // この層より下は全部の列を掛ける
 const int h= N / 2;
 if(h == 0) return 1;
 const size_t stride= max<size_t>(1, size_t(1) << max(0, h - 2));
 Region D(size_t(max(1, 2 * (h - 1))) * stride), C(size_t(1) << h);
 u32* c= C.p;
 c[0]= 1;
 vector<u64> Am((size_t)N * N);  // Montgomery の表現
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < N; ++j) Am[(size_t)i * N + j]= (u64(a[i][j]) << 32) % M;
 vector<u64> A((size_t)(N + 1) * 40 + 4);
 u64* Ap= reinterpret_cast<u64*>((reinterpret_cast<uintptr_t>(A.data()) + 31) & ~uintptr_t(31));
 for(int w= 0; w < h; ++w) {
  if(w < W0) {
   const int nv= (2 * w + 2 + 3) / 4;
   for(int e= 0; e <= 2 * w; ++e)
    for(int k= 0; k < nv * 4; ++k) Ap[size_t(e) * nv * 4 + k]= k < 2 * w + 2 ? Am[(size_t)e * N + k] : 0;
   switch(nv) {
#define HAFH_CASE(q) \
 case q + 1: layer<true, q + 1>(w, Ap, D.p, stride, c); break;
    HAFH_REP(HAFH_CASE)
#undef HAFH_CASE
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
   const int nv= (K + 3) / 4, W= nv * 4;
   auto put= [&](int r, int e) {
    u64* dst= Ap + size_t(r) * W;
    for(int q= 0; q < K; ++q) dst[q]= Am[(size_t)e * N + cols[q]];
    for(int q= K; q < W; ++q) dst[q]= 0;
   };
   int r= 0;
   for(int e= 0; e < 2 * s; ++e) put(r++, e);
   for(int i= 0; i < nth; ++i) put(r++, 2 * thp[i]), put(r++, 2 * thp[i] + 1);
   put(r++, 2 * w);
   switch(nv) {
#define HAFH_CASE(q) \
 case q + 1: layer_h<q + 1>(w, s, TH << s, thp, nth, chp, nch, Ap, D.p, stride, c); break;
    HAFH_REP(HAFH_CASE)
#undef HAFH_CASE
   }
  }
 }
 return c[(size_t(1) << h) - 1];
}
#undef HAFH_REP
}  // namespace hafnian_layer_h
