#pragma once
// 書き比べ: 法 2^64 の集合冪級数の exp と log の AVX2 の核の 3 つ目 (rz3)。rz2 (u64_rz2.hpp、区画と列に分けて変換の領域で閉じた形) に
// 次の 4 つを旗で足す (どれも既定は 1)。
//  - RZ3_PF: 列ごとの段で、今の列を計算している間に、次の列で大きい配列から読む塊を L2 へ先に読む (列の塊は区画 1 つぶん離れている)。
//  - RZ3_VW: 大きい配列の塊の幅を列ごとに変える。列 c の塊は、区画の段の順変換の値 (相対の rank 0 から popcount(c) + 2) と、区画の段の
//    逆変換に要る値 (相対の rank popcount(c) から 11 を、場所 0 から置く) の多い方の幅だけを持つ。区画の幅は 12 × 512 から 4470 に減る。
//  - RZ3_R4: 区画の段の変換を 2 段ずつ 1 回の読み書きで行う (9 段は 2 段 × 4 と 1 段)。窓の端の rank は、値の無い塊を 0 とした式で求める。
//  - RZ3_ML: u64 の積の和を、a_lo b_lo (vpmuludq) と、(a_lo b_hi, a_hi b_lo) の下位 32 bit (vpmulld、b は 32 bit の半分を入れ替えて置く)
//    の 2 回の掛け算で求める。上位 32 bit の値 (a_hi と b_hi) を前もって作らずに済む。
// 比べ終えたら、採るものを NeoLibrary に入れて消す。
#include "_shared/sps/u64_rz.hpp"
#ifdef __linux__
#include <sys/mman.h>
#endif
#ifndef RZ3_PF
#define RZ3_PF 1
#endif
#ifndef RZ3_VW
#define RZ3_VW 1
#endif
#ifndef RZ3_R4
#define RZ3_R4 1
#endif
#ifndef RZ3_ML
#define RZ3_ML 1
#endif
namespace sps_u64c {
using sps_u64::add;
using sps_u64::fin;
using sps_u64::lane_zeta;
using sps_u64::mac;
using sps_u64::sub;
using sps_u64::u64;
using sps_u64::V;
using sps_u64::valloc;
#define U64C_AI [[gnu::always_inline]] inline
constexpr int KC= 9, M0= KC + 2, NB= 1 << KC;
struct Layout {
 int off[NB + 1];
 constexpr Layout(): off() {
  int o= 0;
  for(int c= 0; c < NB; ++c) {
   off[c]= o;
   const int p= __builtin_popcount(c);
   o+= RZ3_VW ? std::max(p + 3, M0 + 1 - p) : M0 + 1;
  }
  off[NB]= o;
 }
};
constexpr Layout LY{};
constexpr int CS= LY.off[NB];  // 区画の幅 (V の数)
U64C_AI int ibase(int c) { return RZ3_VW ? __builtin_popcount(c) : 0; }  // 逆変換の値を置く場所の起点の相対の rank
U64C_AI V hi32(V x) { return _mm256_srli_epi64(x, 32); }
U64C_AI V swap32(V x) { return _mm256_shuffle_epi32(x, 0xB1); }
// RZ3_ML の積の和: lo = sum a_lo b_lo、c = 32 bit ごとの sum (a_lo b_hi, a_hi b_lo)。結果は lo + ((c_lo + c_hi) << 32)
U64C_AI void mac2(V& lo, V& c, V a, V b, V bs) { lo= add(lo, _mm256_mul_epu32(a, b)), c= _mm256_add_epi32(c, _mm256_mullo_epi32(a, bs)); }
U64C_AI V fin2(V lo, V c) { return add(lo, _mm256_slli_epi64(add(c, _mm256_srli_epi64(c, 32)), 32)); }
U64C_AI void pf_block(const V* p, int n) {
 const std::uintptr_t e= std::uintptr_t(p + n);
 for(std::uintptr_t a= std::uintptr_t(p) & ~std::uintptr_t(63); a < e; a+= 64) __builtin_prefetch((const void*)a, 0, 2);
}
inline V* huge_alloc(std::size_t n) {
 const std::size_t H= std::size_t(1) << 21;
 const std::size_t bytes= (n * sizeof(V) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
#endif
 return static_cast<V*>(p);
}
// 区画の段の順変換: 2^11 個の値 src から区画 dst へ。塊 q は相対の rank r を場所 r に置き、[0, popcount(q) + 2] を持つ。
inline void chunk_fwd(const u64* src, V* dst) {
 const V M0v= _mm256_setr_epi64x(-1, 0, 0, 0), M1v= _mm256_setr_epi64x(0, -1, -1, 0), M2v= _mm256_setr_epi64x(0, 0, 0, -1);
 for(int q= 0; q < NB; ++q) {
  const V x= _mm256_loadu_si256((const V*)(src + 4 * q));
  V* b= dst + LY.off[q];
  const int c= __builtin_popcount(q);
  b[c]= lane_zeta<false>(_mm256_and_si256(x, M0v));
  b[c + 1]= lane_zeta<false>(_mm256_and_si256(x, M1v));
  b[c + 2]= lane_zeta<false>(_mm256_and_si256(x, M2v));
 }
 int kk= 0;
#if RZ3_R4
 for(; kk + 2 <= KC; kk+= 2) {
  const int d= 1 << kk;
  for(int q0= 0; q0 < NB; q0+= 4 * d)
   for(int q= q0; q < q0 + d; ++q) {
    V *b0= dst + LY.off[q], *b1= dst + LY.off[q + d], *b2= dst + LY.off[q + 2 * d], *b3= dst + LY.off[q + 3 * d];
    // 前の窓: b0 [lo, hi]、b1 と b2 [lo + 1, hi + 1]、b3 [lo + 2, hi + 2]。後は b1 と b2 が [lo, hi + 1]、b3 が [lo, hi + 2]
    const int lo= __builtin_popcount(q >> kk), hi= __builtin_popcount(q) + 2;
    {
     const V v0= b0[lo];
     b1[lo]= v0, b2[lo]= v0, b3[lo]= v0;
    }
    {
     const V v0= b0[lo + 1], v1= b1[lo + 1], v2= b2[lo + 1];
     b1[lo + 1]= add(v1, v0), b2[lo + 1]= add(v2, v0), b3[lo + 1]= add(add(v1, v2), v0);
    }
    for(int r= lo + 2; r <= hi; ++r) {
     const V v0= b0[r], v1= b1[r], v2= b2[r], v3= b3[r];
     const V s01= add(v0, v1);
     b1[r]= s01, b2[r]= add(v2, v0), b3[r]= add(add(v3, v2), s01);
    }
    b3[hi + 1]= add(add(b3[hi + 1], b2[hi + 1]), b1[hi + 1]);
   }
 }
#endif
 for(; kk < KC; ++kk) {
  const int d= 1 << kk;
  for(int q0= 0; q0 < NB; q0+= 2 * d)
   for(int q= q0; q < q0 + d; ++q) {
    const V* s= dst + LY.off[q];
    V* t= dst + LY.off[q + d];
    const int lo= __builtin_popcount(q >> kk), hi= __builtin_popcount(q) + 2;
    t[lo]= s[lo];
    for(int r= lo + 1; r <= hi; ++r) t[r]= add(t[r], s[r]);
   }
 }
}
// 区画の段の逆変換: 塊 q は相対の rank [popcount(q), 11] を場所 rank - ibase(q) に持つ。位置 y の相対の rank popcount(y) を out に取り出す。
inline void chunk_inv(V* src, u64* out) {
 int kk= 0;
#if RZ3_R4
 for(; kk + 2 <= KC; kk+= 2) {
  const int d= 1 << kk;
  for(int q0= 0; q0 < NB; q0+= 4 * d)
   for(int q= q0; q < q0 + d; ++q) {
    // rank で引けるように、塊の先頭から起点をずらした場所
    V *s0= src + LY.off[q] - ibase(q), *s1= src + LY.off[q + d] - ibase(q + d), *s2= src + LY.off[q + 2 * d] - ibase(q + 2 * d), *s3= src + LY.off[q + 3 * d] - ibase(q + 3 * d);
    // 要る範囲: s1 と s2 は [c + 1, T - 1]、s3 は [c + 2, T] (T は s0 に要る範囲の上端)
    const int c= __builtin_popcount(q), T= c + KC - kk - __builtin_popcount(q >> kk) + 2;
    {
     const V v0= s0[c + 1];
     s1[c + 1]= sub(s1[c + 1], v0), s2[c + 1]= sub(s2[c + 1], v0);
    }
    for(int r= c + 2; r < T; ++r) {
     const V v0= s0[r], v1= s1[r], v2= s2[r], v3= s3[r];
     const V m1= sub(v1, v0);
     s1[r]= m1, s2[r]= sub(v2, v0), s3[r]= sub(sub(v3, v2), m1);
    }
    s3[T]= add(sub(sub(s3[T], s2[T]), s1[T]), s0[T]);
   }
 }
#endif
 for(; kk < KC; ++kk) {
  const int d= 1 << kk;
  for(int q0= 0; q0 < NB; q0+= 2 * d)
   for(int q= q0; q < q0 + d; ++q) {
    const V* s= src + LY.off[q] - ibase(q);
    V* t= src + LY.off[q + d] - ibase(q + d);
    const int a= __builtin_popcount(q) + 1, b= __builtin_popcount(q) + KC - kk - __builtin_popcount(q >> kk) + 2;
    for(int r= a; r <= b; ++r) t[r]= sub(t[r], s[r]);
   }
 }
 const V M0v= _mm256_setr_epi64x(-1, 0, 0, 0), M1v= _mm256_setr_epi64x(0, -1, -1, 0), M2v= _mm256_setr_epi64x(0, 0, 0, -1);
 for(int q= 0; q < NB; ++q) {
  const V* b= src + LY.off[q] - ibase(q);
  const int c= __builtin_popcount(q);
  V r= _mm256_and_si256(lane_zeta<true>(b[c]), M0v);
  r= _mm256_or_si256(r, _mm256_and_si256(lane_zeta<true>(b[c + 1]), M1v));
  r= _mm256_or_si256(r, _mm256_and_si256(lane_zeta<true>(b[c + 2]), M2v));
  _mm256_storeu_si256((V*)(out + 4 * q), r);
 }
}
// 列の塊 j の窓を、塊の下の bit から上の段のゼータ変換
template <class Lo, class Hi> U64C_AI void col_zeta(V* Z, int S, int NJ, int nb, const Lo& lo0, const Hi& hi0) {
 for(int kk= 0; kk < nb; ++kk) {
  const int d= 1 << kk;
  for(int q0= 0; q0 < NJ; q0+= 2 * d)
   for(int q= q0; q < q0 + d; ++q) {
    const V* s= Z + std::size_t(q) * S;
    V* t= Z + std::size_t(q + d) * S;
    const int lo= lo0(q, kk), hi= hi0(q);
    t[lo]= s[lo];
    for(int r= lo + 1; r <= hi; ++r) t[r]= add(t[r], s[r]);
   }
 }
}
// 大きい配列の、区画 j と列 c の塊
U64C_AI V* blk(V* big, int j, int c) { return big + std::size_t(j) * CS + LY.off[c]; }
inline std::vector<u64> exp(const std::vector<u64>& f) {
 const std::size_t N= f.size();
 const int n= __builtin_ctzll(N);
 if(n < M0 + 2) return sps_u64::exp(f);
 const int U= n - M0, NJ= 1 << U, S= n + 1;
 std::vector<u64> h(N);
 {
  const std::vector<u64> h0= sps_u64::exp(std::vector<u64>(f.begin(), f.begin() + (std::size_t(1) << M0)));
  std::copy(h0.begin(), h0.end(), h.begin());
 }
 V* big= huge_alloc(std::size_t(NJ) * CS);
 chunk_fwd(h.data(), big);
 for(int j= 1; j < NJ; ++j) chunk_fwd(f.data() + (std::size_t(j) << M0), big + std::size_t(j) * CS);
 V* Z= valloc(std::size_t(NJ) * S);
 alignas(32) V Fl[32], Fh[32];
#if !RZ3_ML
 alignas(32) V Hh[32];
#endif
 for(int c= 0; c < NB; ++c) {
  const int pcc= __builtin_popcount(c), w0= pcc + 2;
#if RZ3_PF
  const int wn= c + 1 < NB ? LY.off[c + 2] - LY.off[c + 1] : 0;
  auto pf= [&](int j) {
   if(wn) pf_block(blk(big, j, c + 1), wn);
  };
  pf(0);
#endif
  for(int j= 0; j < NJ; ++j) {
   const V* b= blk(big, j, c);
   V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= 0; s <= w0; ++s) z[s]= b[s];
  }
  for(int r= w0 + 1; r <= n; ++r) Z[r]= _mm256_setzero_si256();
  for(int i= M0; i < n; ++i) {
   const int L= 1 << (i - M0);
   V* ZL= Z + std::size_t(L) * S;
   col_zeta(ZL, S, L, i - M0, [](int q, int kk) { return 1 + __builtin_popcount(q >> kk); }, [&](int q) { return 1 + __builtin_popcount(q) + w0; });
   for(int q= 0; q < L; ++q) {
#if RZ3_PF
    pf(L + q);
#endif
    const V* H= Z + std::size_t(q) * S;
    V* T= ZL + std::size_t(q) * S;
    const int wF= 1 + __builtin_popcount(q) + w0;
    T[0]= H[0];
    int r= 1;
#if RZ3_ML
    for(int e= 1; e <= wF; ++e) Fl[e]= T[e], Fh[e]= swap32(T[e]);
    for(; r + 1 <= n; r+= 2) {
     V lo0= _mm256_setzero_si256(), c0= lo0, lo1= lo0, c1= lo0;
     V y= H[r];
     const int em= std::min(r, wF);
     for(int e= 1; e <= em; ++e) {
      const V x= H[r - e];
      mac2(lo1, c1, y, Fl[e], Fh[e]);
      mac2(lo0, c0, x, Fl[e], Fh[e]);
      y= x;
     }
     if(r + 1 <= wF) mac2(lo1, c1, y, Fl[r + 1], Fh[r + 1]);
     T[r]= add(H[r], fin2(lo0, c0)), T[r + 1]= add(H[r + 1], fin2(lo1, c1));
    }
    if(r == n) {
     V lo0= _mm256_setzero_si256(), c0= lo0;
     for(int e= 1; e <= std::min(r, wF); ++e) mac2(lo0, c0, H[r - e], Fl[e], Fh[e]);
     T[r]= add(H[r], fin2(lo0, c0));
    }
#else
    for(int e= 1; e <= wF; ++e) Fl[e]= T[e], Fh[e]= hi32(T[e]);
    for(int k= 0; k <= n; ++k) Hh[k]= hi32(H[k]);
    for(; r + 1 <= n; r+= 2) {
     V lo0= _mm256_setzero_si256(), c0= lo0, lo1= lo0, c1= lo0;
     V y= H[r], yh= Hh[r];
     const int em= std::min(r, wF);
     for(int e= 1; e <= em; ++e) {
      const V x= H[r - e], xh= Hh[r - e];
      mac(lo1, c1, y, yh, Fl[e], Fh[e]);
      mac(lo0, c0, x, xh, Fl[e], Fh[e]);
      y= x, yh= xh;
     }
     if(r + 1 <= wF) mac(lo1, c1, y, yh, Fl[r + 1], Fh[r + 1]);
     T[r]= add(H[r], fin(lo0, c0)), T[r + 1]= add(H[r + 1], fin(lo1, c1));
    }
    if(r == n) {
     V lo0= _mm256_setzero_si256(), c0= lo0;
     for(int e= 1; e <= std::min(r, wF); ++e) mac(lo0, c0, H[r - e], Hh[r - e], Fl[e], Fh[e]);
     T[r]= add(H[r], fin(lo0, c0));
    }
#endif
   }
  }
  for(int kk= 0; kk < U; ++kk) {
   const int d= 1 << kk;
   for(int q0= 0; q0 < NJ; q0+= 2 * d)
    for(int q= q0; q < q0 + d; ++q) {
     const V* s= Z + std::size_t(q) * S;
     V* t= Z + std::size_t(q + d) * S;
     const int a= __builtin_popcount(q) + pcc + 1, b= __builtin_popcount(q) + M0 + U - kk - __builtin_popcount(q >> kk);
     for(int r= a; r <= b; ++r) t[r]= sub(t[r], s[r]);
    }
  }
  const int ib= ibase(c);
  for(int j= 1; j < NJ; ++j) {
   V* b= blk(big, j, c) - ib;
   const V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= pcc; s <= M0; ++s) b[s]= z[s];
  }
 }
 for(int j= 1; j < NJ; ++j) chunk_inv(big + std::size_t(j) * CS, h.data() + (std::size_t(j) << M0));
 std::free(Z), std::free(big);
 return h;
}
inline std::vector<u64> log(const std::vector<u64>& f) {
 const std::size_t N= f.size();
 const int n= __builtin_ctzll(N);
 if(n < M0 + 2) return sps_u64::log(f);
 const int U= n - M0, NJ= 1 << U, S= n + 1;
 std::vector<u64> g(N);
 {
  const std::vector<u64> g0= sps_u64::log(std::vector<u64>(f.begin(), f.begin() + (std::size_t(1) << M0)));
  std::copy(g0.begin(), g0.end(), g.begin());
 }
 V* big= huge_alloc(std::size_t(NJ) * CS);
 for(int j= 0; j < NJ; ++j) chunk_fwd(f.data() + (std::size_t(j) << M0), big + std::size_t(j) * CS);
 V* Z= valloc(std::size_t(NJ) * S);
 alignas(32) V Bl[32], Bh[32], Q[32];
#if !RZ3_ML
 alignas(32) V Qh[32];
#endif
 for(int c= 0; c < NB; ++c) {
  const int pcc= __builtin_popcount(c), w0= pcc + 2;
#if RZ3_PF
  const int wn= c + 1 < NB ? LY.off[c + 2] - LY.off[c + 1] : 0;
  auto pf= [&](int j) {
   if(wn) pf_block(blk(big, j, c + 1), wn);
  };
  pf(0);
#endif
  for(int j= 0; j < NJ; ++j) {
   const V* b= blk(big, j, c);
   V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= 0; s <= w0; ++s) z[s]= b[s];
  }
  col_zeta(Z, S, NJ, U, [](int q, int kk) { return __builtin_popcount(q >> kk); }, [&](int q) { return __builtin_popcount(q) + w0; });
  for(int i= n - 1; i >= M0; --i) {
   const int L= 1 << (i - M0);
   V* ZL= Z + std::size_t(L) * S;
   for(int q= 0; q < L; ++q) {
#if RZ3_PF
    pf(L + q);
#endif
    const V* B= Z + std::size_t(q) * S;
    V* T= ZL + std::size_t(q) * S;
    const int wB= __builtin_popcount(q) + w0;
    Q[0]= _mm256_setzero_si256();
#if RZ3_ML
    for(int e= 1; e <= wB; ++e) Bl[e]= B[e], Bh[e]= swap32(B[e]);
    for(int r= 1; r <= i + 1; ++r) {
     V lo= _mm256_setzero_si256(), cc= lo;
     for(int e= std::min(r - 1, wB); e >= 1; --e) mac2(lo, cc, Q[r - e], Bl[e], Bh[e]);
     const V tf= r <= wB ? sub(T[r], B[r]) : r == wB + 1 ? T[r] : _mm256_setzero_si256();
     Q[r]= sub(tf, fin2(lo, cc));
    }
#else
    for(int e= 1; e <= wB; ++e) Bl[e]= B[e], Bh[e]= hi32(B[e]);
    Qh[0]= Q[0];
    for(int r= 1; r <= i + 1; ++r) {
     V lo= _mm256_setzero_si256(), cc= lo;
     for(int e= std::min(r - 1, wB); e >= 1; --e) mac(lo, cc, Bl[e], Bh[e], Q[r - e], Qh[r - e]);
     const V tf= r <= wB ? sub(T[r], B[r]) : r == wB + 1 ? T[r] : _mm256_setzero_si256();
     Q[r]= sub(tf, fin(lo, cc)), Qh[r]= hi32(Q[r]);
    }
#endif
    for(int r= 0; r <= i + 1; ++r) T[r]= Q[r];
   }
   for(int kk= 0; kk < i - M0; ++kk) {
    const int d= 1 << kk;
    for(int q0= 0; q0 < L; q0+= 2 * d)
     for(int q= q0; q < q0 + d; ++q) {
      const V* s= ZL + std::size_t(q) * S;
      V* t= ZL + std::size_t(q + d) * S;
      const int a= __builtin_popcount(q) + 2 + pcc, b= 1 + __builtin_popcount(q) + i - kk - __builtin_popcount(q >> kk);
      for(int r= a; r <= b; ++r) t[r]= sub(t[r], s[r]);
     }
   }
  }
  const int ib= ibase(c);
  for(int j= 1; j < NJ; ++j) {
   V* b= blk(big, j, c) - ib;
   const V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= pcc; s <= M0; ++s) b[s]= z[s];
  }
 }
 for(int j= 1; j < NJ; ++j) chunk_inv(big + std::size_t(j) * CS, g.data() + (std::size_t(j) << M0));
 std::free(Z), std::free(big);
 return g;
}
#undef U64C_AI
}  // namespace sps_u64c
