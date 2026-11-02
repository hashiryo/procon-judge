#pragma once
// zmod_layer と zmod_layer_c の中身。yosupo_216351 と同じ部分集合の DP を、最大の組ごとの層に分け、配る値をまとめて AVX2 で求める版。法は 998244353。
// 頂点 2p と 2p+1 を組 p (p < h = N/2) とする。完全マッチングに組の辺を重ねると交互の閉路に分かれるので、閉路を最大の組の
// 小さい順に 1 本ずつ作る。閉路は最大の組 w の頂点 2w から出て、マッチングの辺 (重み a) で別の組の頂点 k に入り、組の辺で
// k^1 へ移る、を繰り返し、2w+1 へ戻って閉じる。
//   c[S]: 組の集合 S をちょうど覆い、閉路がすべて閉じた重みの和 (c[0] = 1、答えは c[全体])。
//   D_e[T]: S = {w} ∪ T (T ⊆ [0, w)) を覆い、最大の組 w の閉路がいま頂点 e (e は T の組の頂点) で開いている重みの和。
//   最大の組 w の閉路を作り始めたところの重みは c[T] で、開いている端は 2w。
// 層 w の T を小さい順に見て、v_T[k] = c[T] a[2w][k] + sum_{p ∈ T} (D_{2p}[T] a[2p][k] + D_{2p+1}[T] a[2p+1][k]) を、
// k の向きに 4 個ずつ (u64 の和にためて) 求める。v_T[2w+1] が閉じた重み c[S] で、p ∉ T (p < w) の組へ進むと
// D_{2p+1}[T + 2^p] = v_T[2p]、D_{2p}[T + 2^p] = v_T[2p+1] になる (どちらも配る元はこの T だけ)。T + 2^p > T なので、
// T を小さい順に見れば、読む前に書き終わっている。a は Montgomery の表現 (2^32 倍) で持ち、和を Montgomery の還元で 2^32 で
// 割れば、v は普通の表現で M 未満になる。積は M^2 未満なので、8 項ごとに最上位の bit が立っていれば 2M 2^32 を引いて
// 2^63 未満に保つ (行列積の核と同じ)。
// D_e は頂点ごとに部分集合の番号で引く配列にし、T を小さい順に見るとき、どの頂点の配列も前から順に読み書きされるように
// する。組 p の頂点の配列は p を含む T でしか引かないので、COMPACT なら T から bit p を抜いた番号 (pext) で引いて、配列を
// 半分 (層 w で 2^{w-1} 個) に詰める。手間は層 w で 2^w 個の T ごとに (2|T| + 1) (2w + 2) 回の積和。
#include "../common.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace hafnian_layer {
// T から bit p を抜いた番号。
[[gnu::always_inline]] inline u32 drop_bit(u32 T, int p) {
#ifdef __BMI2__
 return _pext_u32(T, ~(1u << p));
#else
 const u32 low= (1u << p) - 1;
 return (T & low) | ((T >> 1) & ~low);
#endif
}
constexpr u32 M= MOD;
constexpr u32 NIV= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
// n 個の u32 の領域。Linux で 2^19 byte 以上なら、2 MB 境界の mmap に置いて huge page を頼み、先にページを用意させる。
struct Region {
 u32* p= nullptr;
 size_t mapped= 0;
 explicit Region(size_t n) {
#ifdef __linux__
  constexpr size_t H= size_t(1) << 21;
  if(n * sizeof(u32) >= (size_t(1) << 19)) {
   const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
   void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m != MAP_FAILED) {
    char *q= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
    if(a != q) munmap(q, a - q);
    munmap(a + bytes, q + H - a);
    madvise(a, bytes, MADV_HUGEPAGE);
    madvise(a, bytes, 23);  // MADV_POPULATE_WRITE (Linux 5.14 から)
    p= reinterpret_cast<u32*>(a), mapped= bytes;
    return;
   }
  }
#endif
  p= static_cast<u32*>(::operator new(n * sizeof(u32)));
 }
 ~Region() {
#ifdef __linux__
  if(mapped) {
   munmap(p, mapped);
   return;
  }
#endif
  ::operator delete(p);
 }
 Region(const Region&)= delete;
 Region& operator=(const Region&)= delete;
};
#define HAF_REP(X) X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9)
// 層 w。A は行 e (0 <= e <= 2w) ごとに NV * 4 個の u64 (列 k < 2w + 2 の a[e][k] の Montgomery の表現、残りは 0)。
// D は頂点 e の配列の先頭 (D + e * stride)、c は組の集合ごとの閉じた重み。
template <bool COMPACT, int NV> [[gnu::noinline]] void layer(int w, const u64* __restrict A, u32* __restrict D, size_t stride, u32* __restrict c) {
 const __m256i Mv= _mm256_set1_epi32(M), M2= _mm256_set1_epi32(2 * M), TOP= _mm256_set1_epi64x((long long)(1ull << 63)), NI= _mm256_set1_epi32(NIV);
 const u32 bw= 1u << w;
 const u64* aw= A + size_t(2 * w) * NV * 4;
 alignas(32) u64 v[NV * 4];
 for(u32 T= 0; T < bw; ++T) {
  __m256i a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
  {
   const __m256i x= _mm256_set1_epi32(c[T]);
#define HAF_INIT(q) \
 if constexpr(NV > q) a##q= _mm256_mul_epu32(x, _mm256_load_si256((const __m256i*)(aw + 4 * q)));
   HAF_REP(HAF_INIT)
#undef HAF_INIT
  }
  int cnt= 1;
  for(u32 t= T; t; t&= t - 1) {
   const int p= __builtin_ctz(t);
   if(cnt > 6) {
#define HAF_KEEP(q) \
 if constexpr(NV > q) a##q= _mm256_sub_epi64(a##q, _mm256_min_epu32(M2, _mm256_and_si256(TOP, a##q)));
    HAF_REP(HAF_KEEP)
    cnt= 0;
   }
   const u32 ix= COMPACT ? drop_bit(T, p) : T;
   const __m256i x0= _mm256_set1_epi32(D[size_t(2 * p) * stride + ix]), x1= _mm256_set1_epi32(D[size_t(2 * p + 1) * stride + ix]);
   const u64 *r0= A + size_t(2 * p) * NV * 4, *r1= r0 + NV * 4;
#define HAF_MAC(q) \
 if constexpr(NV > q) a##q= _mm256_add_epi64(a##q, _mm256_add_epi64(_mm256_mul_epu32(x0, _mm256_load_si256((const __m256i*)(r0 + 4 * q))), _mm256_mul_epu32(x1, _mm256_load_si256((const __m256i*)(r1 + 4 * q)))));
   HAF_REP(HAF_MAC)
#undef HAF_MAC
   cnt+= 2;
  }
  HAF_REP(HAF_KEEP)  // 2^63 未満にしてから還元する
#undef HAF_KEEP
#define HAF_RED(q) \
 if constexpr(NV > q) { \
  __m256i r= _mm256_srli_epi64(_mm256_add_epi64(a##q, _mm256_mul_epu32(_mm256_mul_epu32(a##q, NI), Mv)), 32); \
  r= _mm256_min_epu32(r, _mm256_sub_epi32(r, M2)); \
  _mm256_store_si256((__m256i*)(v + 4 * q), _mm256_min_epu32(r, _mm256_sub_epi32(r, Mv))); \
 }
  HAF_REP(HAF_RED)
#undef HAF_RED
  c[bw + T]= u32(v[2 * w + 1]);
  for(u32 t= ~T & (bw - 1); t; t&= t - 1) {
   const int p= __builtin_ctz(t);
   const u32 ix= COMPACT ? drop_bit(T, p) : T | (1u << p);
   D[size_t(2 * p + 1) * stride + ix]= u32(v[2 * p]);
   D[size_t(2 * p) * stride + ix]= u32(v[2 * p + 1]);
  }
 }
}
template <bool COMPACT> u32 hafnian(int N, const vector<vector<u32>>& a) {
 const int h= N / 2;
 if(h == 0) return 1;
 const size_t stride= COMPACT ? max<size_t>(1, size_t(1) << max(0, h - 2)) : size_t(1) << (h - 1);
 Region D(size_t(max(1, 2 * (h - 1))) * stride), C(size_t(1) << h);
 u32* c= C.p;
 c[0]= 1;
 vector<u64> A((size_t)N * 40 + 4);
 u64* Ap= reinterpret_cast<u64*>((reinterpret_cast<uintptr_t>(A.data()) + 31) & ~uintptr_t(31));
 for(int w= 0; w < h; ++w) {
  const int nv= (2 * w + 2 + 3) / 4;
  for(int e= 0; e <= 2 * w; ++e)
   for(int k= 0; k < nv * 4; ++k) Ap[size_t(e) * nv * 4 + k]= k < 2 * w + 2 ? (u64(a[e][k]) << 32) % M : 0;
  switch(nv) {
#define HAF_CASE(q) \
 case q + 1: layer<COMPACT, q + 1>(w, Ap, D.p, stride, c); break;
   HAF_REP(HAF_CASE)
#undef HAF_CASE
  }
 }
 return c[(size_t(1) << h) - 1];
}
}  // namespace hafnian_layer
#undef HAF_REP
