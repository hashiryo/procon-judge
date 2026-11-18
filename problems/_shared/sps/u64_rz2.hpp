#pragma once
// 書き比べ: 法 2^64 の集合冪級数の exp と log の AVX2 の核の 2 つ目 (rz2)。rz1 (u64_rz.hpp) は段ごとに i bit の変換を 3 回行い、大きい
// 配列を bit ごとに読み書きするので、NeoLibrary の ranked_xor と同じく区画と列に分け、変換の領域で閉じた形にする。
//  - OR のランク付きのゼータ変換でも、余りの項 (rank が集合の大きさより大きい項) は足し算と掛け算を通しても余りの項のままなので、
//    exp は変換の領域で Ĥ[y ∪ {i}] = Ĥ[y] (1 + t F̂_i[y]) を上の要素から順に、log は各段の t F̂_i / F̂_lo を各点の割り算で求め、
//    逆変換は最後に 1 回だけ行う。どちらも rank n までで切れる (n bit の集合の大きさは n 以下)。
//  - 位置の下の 11 bit (レーンの 2 bit と塊の 9 bit) を区画とし、区画の中の変換 (区画の段) は区画ごとに L2 の中で行う。上の bit は、
//    下の 11 bit が同じ位置の塊 (列) を作業の配列に集めて、上の段の変換、各点の計算、上の段の逆変換を続けて行う。
//  - 大きい配列の塊 (区画 j、列 c) は rank [popcount(j), popcount(j) + 11] の 12 個だけを持つ。区画の段の順変換の値 (rank は
//    popcount(j) から popcount(j) + popcount(c) + 2) と、区画の段の逆変換に要る値 (popcount(j) + popcount(c) から popcount(j) + 11) は
//    この範囲に収まる。
// 小さい n (13 未満) と区画 0 (下の 11 bit の部分) は rz1 で求める。比べ終えたら、採るものを NeoLibrary に入れて消す。
#include "_shared/sps/u64_rz.hpp"
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace sps_u64b {
using sps_u64::add;
using sps_u64::fin;
using sps_u64::lane_zeta;
using sps_u64::mac;
using sps_u64::sub;
using sps_u64::u64;
using sps_u64::V;
using sps_u64::valloc;
#define U64B_AI [[gnu::always_inline]] inline
constexpr int KC= 9, M0= KC + 2, CW= M0 + 1;
U64B_AI V hi32(V x) { return _mm256_srli_epi64(x, 32); }
inline V* huge_alloc(std::size_t n) {
 const std::size_t H= std::size_t(1) << 21;
 const std::size_t bytes= (n * sizeof(V) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
#endif
 return static_cast<V*>(p);
}
// 区画の段の順変換: 2^11 個の値 src から、塊 2^9 個 (幅 CW) の dst へ。rank は区画の起点からの相対で、塊 q は [0, popcount(q) + 2] を持つ。
inline void chunk_fwd(const u64* src, V* dst) {
 const V M0v= _mm256_setr_epi64x(-1, 0, 0, 0), M1v= _mm256_setr_epi64x(0, -1, -1, 0), M2v= _mm256_setr_epi64x(0, 0, 0, -1);
 for(int q= 0; q < (1 << KC); ++q) {
  const V x= _mm256_loadu_si256((const V*)(src + 4 * q));
  V* b= dst + q * CW;
  const int c= __builtin_popcount(q);
  b[c]= lane_zeta<false>(_mm256_and_si256(x, M0v));
  b[c + 1]= lane_zeta<false>(_mm256_and_si256(x, M1v));
  b[c + 2]= lane_zeta<false>(_mm256_and_si256(x, M2v));
 }
 for(int kk= 0; kk < KC; ++kk) {
  const int d= 1 << kk;
  for(int q0= 0; q0 < (1 << KC); q0+= 2 * d)
   for(int q= q0; q < q0 + d; ++q) {
    const V* s= dst + q * CW;
    V* t= dst + (q + d) * CW;
    const int lo= __builtin_popcount(q >> kk), hi= __builtin_popcount(q) + 2;
    t[lo]= s[lo];
    for(int r= lo + 1; r <= hi; ++r) t[r]= add(t[r], s[r]);
   }
 }
}
// 区画の段の逆変換: 塊 q が相対の rank [popcount(q), 11] を持つ src から、位置 y の相対の rank popcount(y) を out に取り出す。
inline void chunk_inv(V* src, u64* out) {
 for(int kk= 0; kk < KC; ++kk) {
  const int d= 1 << kk;
  for(int q0= 0; q0 < (1 << KC); q0+= 2 * d)
   for(int q= q0; q < q0 + d; ++q) {
    const V* s= src + q * CW;
    V* t= src + (q + d) * CW;
    const int a= __builtin_popcount(q) + 1, b= __builtin_popcount(q) + KC - kk - __builtin_popcount(q >> kk) + 2;
    for(int r= a; r <= b; ++r) t[r]= sub(t[r], s[r]);
   }
 }
 const V M0v= _mm256_setr_epi64x(-1, 0, 0, 0), M1v= _mm256_setr_epi64x(0, -1, -1, 0), M2v= _mm256_setr_epi64x(0, 0, 0, -1);
 for(int q= 0; q < (1 << KC); ++q) {
  const V* b= src + q * CW;
  const int c= __builtin_popcount(q);
  V r= _mm256_and_si256(lane_zeta<true>(b[c]), M0v);
  r= _mm256_or_si256(r, _mm256_and_si256(lane_zeta<true>(b[c + 1]), M1v));
  r= _mm256_or_si256(r, _mm256_and_si256(lane_zeta<true>(b[c + 2]), M2v));
  _mm256_storeu_si256((V*)(out + 4 * q), r);
 }
}
U64B_AI V* blk(V* big, int j, int c) { return big + ((std::size_t(j) << KC) | std::size_t(c)) * CW; }
// 列の塊 j の窓 [lo(j), hi(j)] を、塊の下の bit から上の段のゼータ変換 (塊は 0..NJ-1、bit は 0..nb-1)
template <class Lo, class Hi> U64B_AI void col_zeta(V* Z, int S, int NJ, int nb, const Lo& lo0, const Hi& hi0) {
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
 V* big= huge_alloc((std::size_t(NJ) << KC) * CW);
 chunk_fwd(h.data(), big);
 for(int j= 1; j < NJ; ++j) chunk_fwd(f.data() + (std::size_t(j) << M0), blk(big, j, 0));
 V* Z= valloc(std::size_t(NJ) * S);
 alignas(32) V Fl[32], Fh[32], Hh[32];
 for(int c= 0; c < (1 << KC); ++c) {
  const int pcc= __builtin_popcount(c), w0= pcc + 2;
  for(int j= 0; j < NJ; ++j) {
   const V* b= blk(big, j, c);
   V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= 0; s <= w0; ++s) z[s]= b[s];
  }
  for(int r= w0 + 1; r <= n; ++r) Z[r]= _mm256_setzero_si256();
  for(int i= M0; i < n; ++i) {
   const int L= 1 << (i - M0);
   V* ZL= Z + std::size_t(L) * S;
   // f_i の塊 (列の L..2L-1) を上の段で変換して t F̂_i に (rank は要素 i のぶん 1 ずれている)
   col_zeta(ZL, S, L, i - M0, [](int q, int kk) { return 1 + __builtin_popcount(q >> kk); }, [&](int q) { return 1 + __builtin_popcount(q) + w0; });
   for(int q= 0; q < L; ++q) {
    const V* H= Z + std::size_t(q) * S;
    V* T= ZL + std::size_t(q) * S;
    const int wF= 1 + __builtin_popcount(q) + w0;
    for(int e= 1; e <= wF; ++e) Fl[e]= T[e], Fh[e]= hi32(T[e]);
    for(int k= 0; k <= n; ++k) Hh[k]= hi32(H[k]);
    // T[r] = H[r] + sum_{e=1}^{min(r, wF)} H[r-e] F[e] を 2 つずつ (r と r + 1)
    T[0]= H[0];
    int r= 1;
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
   }
  }
  // 上の段の逆変換。塊 j に要るのは rank [popcount(j) + popcount(c), popcount(j) + 11 + 残りの上の bit のうち j に無いものの数]
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
  for(int j= 1; j < NJ; ++j) {
   V* b= blk(big, j, c);
   const V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= pcc; s <= M0; ++s) b[s]= z[s];
  }
 }
 for(int j= 1; j < NJ; ++j) chunk_inv(blk(big, j, 0), h.data() + (std::size_t(j) << M0));
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
 V* big= huge_alloc((std::size_t(NJ) << KC) * CW);
 for(int j= 0; j < NJ; ++j) chunk_fwd(f.data() + (std::size_t(j) << M0), blk(big, j, 0));
 V* Z= valloc(std::size_t(NJ) * S);
 alignas(32) V Bl[32], Bh[32], Q[32], Qh[32];
 for(int c= 0; c < (1 << KC); ++c) {
  const int pcc= __builtin_popcount(c), w0= pcc + 2;
  for(int j= 0; j < NJ; ++j) {
   const V* b= blk(big, j, c);
   V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= 0; s <= w0; ++s) z[s]= b[s];
  }
  // 上の段を全部変換する。塊 j は rank [0, popcount(j) + popcount(c) + 2] を持つ
  col_zeta(Z, S, NJ, U, [](int q, int kk) { return __builtin_popcount(q >> kk); }, [&](int q) { return __builtin_popcount(q) + w0; });
  // 段 i (上から): 列の 0..L-1 が F̂_lo、L..2L-1 が F̂_lo + t F̂_i。t Q_i = t F̂_i / F̂_lo を L..2L-1 に置いて、上の段を逆変換する
  for(int i= n - 1; i >= M0; --i) {
   const int L= 1 << (i - M0);
   V* ZL= Z + std::size_t(L) * S;
   for(int q= 0; q < L; ++q) {
    const V* B= Z + std::size_t(q) * S;
    V* T= ZL + std::size_t(q) * S;
    const int wB= __builtin_popcount(q) + w0;
    for(int e= 1; e <= wB; ++e) Bl[e]= B[e], Bh[e]= hi32(B[e]);
    Q[0]= Qh[0]= _mm256_setzero_si256();
    for(int r= 1; r <= i + 1; ++r) {
     V lo= _mm256_setzero_si256(), cc= lo;
     const int em= std::min(r - 1, wB);
     for(int e= 1; e <= em; ++e) mac(lo, cc, Bl[e], Bh[e], Q[r - e], Qh[r - e]);
     const V tf= r <= wB ? sub(T[r], B[r]) : r == wB + 1 ? T[r] : _mm256_setzero_si256();
     Q[r]= sub(tf, fin(lo, cc)), Qh[r]= hi32(Q[r]);
    }
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
  for(int j= 1; j < NJ; ++j) {
   V* b= blk(big, j, c);
   const V* z= Z + std::size_t(j) * S + __builtin_popcount(j);
   for(int s= pcc; s <= M0; ++s) b[s]= z[s];
  }
 }
 for(int j= 1; j < NJ; ++j) chunk_inv(blk(big, j, 0), g.data() + (std::size_t(j) << M0));
 std::free(Z), std::free(big);
 return g;
}
#undef U64B_AI
}  // namespace sps_u64b
