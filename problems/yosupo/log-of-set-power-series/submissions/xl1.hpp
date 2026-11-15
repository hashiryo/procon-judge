#pragma once
// 集合冪級数の log。rank 付きの XOR 変換 (補集合の対称で半分) の領域で、上の要素から 1 つずつ割る。
//   h_i[S'] = h[S' ∪ {i}]、h_{<i} を h を [0, i) に制限したものとすると、log の上の要素が i の部分 f_i は h_i / h_{<i}。
//   変換の領域では f̂_i = ĥ_i / ĥ_{<i} (各点の t の冪級数の割り算、ĥ_{<i} の定数項は h[∅] = 1) で、rank i までで切る。
//   ĥ_{<i+1}[y ±] = ĥ_{<i}[y] ± t ĥ_i[y] と伸ばすので、h_i を 1 つずつ変換するだけでよい。f̂_i は i bit の逆変換で取り出す。
//   部分集合の大きさより rank の大きい項は大きさと等しい rank に混ざらないので、捨ててよい (exp の xe1 と同じ理由)。
//   割り算は先頭が 1 なので整数の割り算を使わない。値は全部 R 倍で持つ (Q'_k = A'_k - REDC(Σ B'_j Q'_{k-j}) が閉じる)。
// 変換と窓は subset-convolution の xw1 と同じ形。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif

namespace xl1 {
using u32= std::uint32_t;
using u64= std::uint64_t;
using V= __m256i;
#define XL_AI __attribute__((always_inline)) inline

constexpr u32 P= 998244353, P2= 2 * P;
constexpr u32 inv32(u32 n) {
 u32 x= n;
 for(int i= 0; i < 5; ++i) x*= 2u - n * x;
 return x;
}
constexpr u32 NPINV= 0u - inv32(P);
inline u32 pw(u64 a, u64 e) {
 u64 r= 1;
 for(a%= P; e; e>>= 1, a= a * a % P)
  if(e & 1) r= r * a % P;
 return (u32)r;
}
constexpr int KC= 9;

XL_AI V ld(const u32* p) { return _mm256_load_si256((const V*)p); }
XL_AI void st(u32* p, V x) { _mm256_store_si256((V*)p, x); }
XL_AI V cst(u32 x) { return _mm256_set1_epi32((int)x); }
XL_AI V red2(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P2))); }
XL_AI V neg2(V x) { return _mm256_sub_epi32(cst(P2), x); }
XL_AI V add2(V a, V b) { return red2(_mm256_add_epi32(a, b)); }
XL_AI V sub2(V a, V b) { return red2(_mm256_add_epi32(a, neg2(b))); }
XL_AI V norm1(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P))); }
XL_AI V mul64(V a, V b) { return _mm256_mul_epu32(a, b); }
XL_AI V redc2(V e, V o) {
 const V pv= _mm256_set1_epi64x(P), nv= _mm256_set1_epi64x(NPINV);
 V te= _mm256_add_epi64(e, mul64(mul64(e, nv), pv));
 V to= _mm256_add_epi64(o, mul64(mul64(o, nv), pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(te, 32), to, 0xAA);
}
XL_AI V lane_wht(V x) {
 V t= _mm256_shuffle_epi32(x, 0xB1);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xAA)));
 t= _mm256_shuffle_epi32(x, 0x4E);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xCC)));
 t= _mm256_permute2x128_si256(x, x, 0x01);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xF0)));
 return x;
}
XL_AI V mpc(int j) {
 auto b= [&](int r) { return __builtin_popcount(r) == j ? -1 : 0; };
 return _mm256_setr_epi32(b(0), b(1), b(2), b(3), b(4), b(5), b(6), b(7));
}
// 塊には rank r の値を場所 r - sb に置き、場所は [0, ns) だけを持つ。
XL_AI void init_block(u32* X, const u32* in, int lo, int sb, int ns) {
 V lo_= _mm256_loadu_si256((const V*)in), hi= _mm256_loadu_si256((const V*)(in + 8));
 __m256 lf= _mm256_castsi256_ps(lo_), hf= _mm256_castsi256_ps(hi);
 V E= _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(lf, hf, 0x88)), 0xD8);
 V O= _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(lf, hf, 0xDD)), 0xD8);
 const V M0= mpc(0), M1= mpc(1), M2= mpc(2), M3= mpc(3);
 V v[5];
 v[0]= _mm256_broadcastd_epi32(_mm256_castsi256_si128(E));
 v[1]= lane_wht(_mm256_or_si256(_mm256_and_si256(E, M1), _mm256_and_si256(O, M0)));
 v[2]= lane_wht(_mm256_or_si256(_mm256_and_si256(E, M2), _mm256_and_si256(O, M1)));
 v[3]= lane_wht(_mm256_or_si256(_mm256_and_si256(E, M3), _mm256_and_si256(O, M2)));
 V o7= _mm256_permutevar8x32_epi32(O, cst(7));
 v[4]= _mm256_blend_epi32(o7, neg2(o7), 0x96);
 for(int j= 0; j < 5; ++j)
  if(int s= lo + j - sb; s >= 0 && s < ns) st(X + 8 * s, v[j]);
}
XL_AI void fwd_pair(u32* X, u32* Y, int lo, int k, int sb, int ns) {
 if(int s= lo - sb; s >= 0 && s < ns) st(Y + 8 * s, ld(X + 8 * s));
 const int a= std::max(lo + 1 - sb, 0), b= std::min(lo + 4 + k - sb, ns - 1);
 for(int s= a; s <= b; ++s) {
  V x= ld(X + 8 * s), y= ld(Y + 8 * s);
  st(X + 8 * s, add2(x, y)), st(Y + 8 * s, sub2(x, y));
 }
 if(int s= lo + 5 + k - sb; s >= 0 && s < ns) {
  V y= ld(Y + 8 * s);
  st(X + 8 * s, y), st(Y + 8 * s, neg2(y));
 }
}
XL_AI void inv_pair(u32* X, u32* Y, int lo, int k, int sb, int ns) {
 if(int s= lo - sb; s >= 0 && s < ns) st(X + 8 * s, add2(ld(X + 8 * s), ld(Y + 8 * s)));
 const int a= std::max(lo + 1 - sb, 0), b= std::min(lo + 4 + k - sb, ns - 1);
 for(int s= a; s <= b; ++s) {
  V x= ld(X + 8 * s), y= ld(Y + 8 * s);
  st(X + 8 * s, add2(x, y)), st(Y + 8 * s, sub2(x, y));
 }
 if(int s= lo + 5 + k - sb; s >= 0 && s < ns) st(Y + 8 * s, sub2(ld(X + 8 * s), ld(Y + 8 * s)));
}
// 割り算と伸ばし。A = ĥ'_i (rank 0..i を場所 0..i)、B = ĥ'_{<i} (rank 1..i を場所 0..i-1、rank 0 は 1)。
// Q' = A / B を rank i まで求めて A の場所に書く。upd なら B を ĥ'_{<i+1} (rank 1..i+1) にし、上の半分を Bhi に書く。
XL_AI void step_block(u32* A, u32* B, u32* Bhi, int i, bool upd) {
 alignas(32) u32 Be[20 * 8], Bo[20 * 8], Qe[21 * 8], Qo[21 * 8];
 for(int j= 1; j <= i; ++j) {
  V x= norm1(ld(B + 8 * (j - 1)));
  st(Be + 8 * j, x), st(Bo + 8 * j, _mm256_srli_epi64(x, 32));
 }
 const V z= _mm256_setzero_si256();
 {
  V q= ld(A);
  V qn= norm1(q);
  st(Qe, qn), st(Qo, _mm256_srli_epi64(qn, 32));
 }
 alignas(32) u32 Q[21 * 8];
 st(Q, ld(A));
 for(int k= 1; k <= i; ++k) {
  V S;
  if(k <= 12) {
   V e= z, o= z;
   for(int j= 1; j <= k; ++j) {
    e= _mm256_add_epi64(e, mul64(ld(Be + 8 * j), ld(Qe + 8 * (k - j))));
    o= _mm256_add_epi64(o, mul64(ld(Bo + 8 * j), ld(Qo + 8 * (k - j))));
   }
   S= red2(redc2(e, o));
  } else {
   V e1= z, o1= z, e2= z, o2= z;
   for(int j= 1; j <= 12; ++j) {
    e1= _mm256_add_epi64(e1, mul64(ld(Be + 8 * j), ld(Qe + 8 * (k - j))));
    o1= _mm256_add_epi64(o1, mul64(ld(Bo + 8 * j), ld(Qo + 8 * (k - j))));
   }
   for(int j= 13; j <= k; ++j) {
    e2= _mm256_add_epi64(e2, mul64(ld(Be + 8 * j), ld(Qe + 8 * (k - j))));
    o2= _mm256_add_epi64(o2, mul64(ld(Bo + 8 * j), ld(Qo + 8 * (k - j))));
   }
   S= add2(red2(redc2(e1, o1)), red2(redc2(e2, o2)));
  }
  V q= sub2(ld(A + 8 * k), S);
  st(Q + 8 * k, q);
  V qn= norm1(q);
  st(Qe + 8 * k, qn), st(Qo + 8 * k, _mm256_srli_epi64(qn, 32));
 }
 if(upd)
  for(int r= 1; r <= i + 1; ++r) {
   V b= r <= i ? ld(B + 8 * (r - 1)) : z, a= ld(A + 8 * (r - 1));
   st(Bhi + 8 * (r - 1), sub2(b, a));
   st(B + 8 * (r - 1), add2(b, a));
  }
 for(int k= 0; k <= i; ++k) st(A + 8 * k, ld(Q + 8 * k));
}
// 塊 q のレーンの逆変換と f_i の取り出し。rank r は場所 r (sb = 0)。out = Q K R^{-1}。
XL_AI void out_block(const u32* X, int c, int ns, u32* out, V Kc) {
 const V M0= mpc(0), M1= mpc(1), M2= mpc(2), M3= mpc(3), z= _mm256_setzero_si256();
 V v[5];
 for(int j= 0; j < 5; ++j) {
  int s= c + j;
  v[j]= s < ns ? lane_wht(ld(X + 8 * s)) : z;
 }
 V ev= _mm256_or_si256(_mm256_or_si256(_mm256_and_si256(v[0], M0), _mm256_and_si256(v[1], M1)), _mm256_or_si256(_mm256_and_si256(v[2], M2), _mm256_and_si256(v[3], M3)));
 V od= _mm256_or_si256(_mm256_or_si256(_mm256_and_si256(v[1], M0), _mm256_and_si256(v[2], M1)), _mm256_or_si256(_mm256_and_si256(v[3], M2), _mm256_and_si256(v[4], M3)));
 V ul= _mm256_unpacklo_epi32(ev, od), uh= _mm256_unpackhi_epi32(ev, od);
 V q[2]= {_mm256_permute2x128_si256(ul, uh, 0x20), _mm256_permute2x128_si256(ul, uh, 0x31)};
 for(int t= 0; t < 2; ++t) {
  V e= mul64(q[t], Kc), o= mul64(_mm256_srli_epi64(q[t], 32), Kc);
  _mm256_storeu_si256((V*)(out + 8 * t), norm1(redc2(e, o)));
 }
}

inline u32* huge_alloc(size_t bytes) {
 const size_t H= size_t(1) << 21;
 bytes= (bytes + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
#endif
 return (u32*)p;
}
// 素朴な log (3^m)。h[0] = 1。
inline void naive_log(int m, const u32* h, u32* f) {
 f[0]= 0;
 for(int i= 0; i < m; ++i)
  for(int s= 0; s < (1 << i); ++s) {
   u64 t= h[(1 << i) | s];
   for(int a= s; a; a= (a - 1) & s) t= (t + u64(P - h[a]) * f[(1 << i) | (s ^ a)]) % P;
   f[(1 << i) | s]= u32(t % P);
  }
}

template <int N> void log_n(const u32* h, u32* out) {
 constexpr int NSB= N - 1, BSB= NSB * 8, NSF= N, BSF= NSF * 8;
 u32 *B= huge_alloc((size_t(1) << (N - 5)) * BSB * 4), *F= huge_alloc((size_t(1) << (N - 5)) * BSF * 4);
 u32* hs= huge_alloc(size_t(4) << N);
 {  // hs = h R
  const V r2= cst(u32((u64(1) << 32) % P * ((u64(1) << 32) % P) % P));
  for(size_t s= 0; s < (size_t(1) << N); s+= 8) {
   V x= _mm256_loadu_si256((const V*)(h + s));
   _mm256_store_si256((V*)(hs + s), norm1(redc2(mul64(x, r2), mul64(_mm256_srli_epi64(x, 32), r2))));
  }
 }
 naive_log(4, h, out);
 init_block(B, hs, 0, 1, NSB);
 alignas(32) static u32 cF[(size_t(1) << (N - 5 > KC ? N - 5 - KC : 0)) * BSF];
 for(int i= 4; i < N; ++i) {
  const int nb= i - 4, K= std::min(nb, KC), H= nb - K, ns= i + 1;
  const size_t cnt= size_t(1) << nb, csz= size_t(1) << K;
  const bool upd= i + 1 < N;
  const u32* in= hs + (size_t(1) << i);
  for(size_t q0= 0; q0 < cnt; q0+= csz) {
   for(size_t q= q0; q < q0 + csz; ++q) init_block(F + q * BSF, in + 16 * q, __builtin_popcountll(q), 0, ns);
   for(int k= 0; k < K; ++k)
    for(size_t q= q0; q < q0 + csz; ++q)
     if(!(q >> k & 1)) fwd_pair(F + q * BSF, F + (q | size_t(1) << k) * BSF, __builtin_popcountll(q >> (k + 1)), k, 0, ns);
  }
  if(H == 0) {
   for(size_t q= 0; q < cnt; ++q) step_block(F + q * BSF, B + q * BSB, B + (q + cnt) * BSB, i, upd);
  } else {
   const size_t hs_= size_t(1) << H;
   for(size_t j= 0; j < csz; ++j) {
    for(size_t h_= 0; h_ < hs_; ++h_) std::memcpy(cF + h_ * BSF, F + ((h_ << K) + j) * BSF, size_t(ns) * 32);
    for(int kk= 0; kk < H; ++kk)
     for(size_t h_= 0; h_ < hs_; ++h_)
      if(!(h_ >> kk & 1)) fwd_pair(cF + h_ * BSF, cF + (h_ | size_t(1) << kk) * BSF, __builtin_popcountll(h_ >> (kk + 1)), kk + K, 0, ns);
    for(size_t h_= 0; h_ < hs_; ++h_) {
     size_t q= (h_ << K) + j;
     step_block(cF + h_ * BSF, B + q * BSB, B + (q + cnt) * BSB, i, upd);
    }
    for(int kk= H; kk--;)
     for(size_t h_= 0; h_ < hs_; ++h_)
      if(!(h_ >> kk & 1)) inv_pair(cF + h_ * BSF, cF + (h_ | size_t(1) << kk) * BSF, __builtin_popcountll(h_ >> (kk + 1)), kk + K, 0, ns);
    for(size_t h_= 0; h_ < hs_; ++h_) std::memcpy(F + ((h_ << K) + j) * BSF, cF + h_ * BSF, size_t(ns) * 32);
   }
  }
  // f̂_i の下の段の逆変換と取り出し (i bit の変換なので 2/2^i を掛け、R 倍を外す)
  const V Kc= cst(pw(2, u64(P - 1) * 64 - (i - 1)));
  u32* o= out + (size_t(1) << i);
  for(size_t q0= 0; q0 < cnt; q0+= csz) {
   for(int k= K; k--;)
    for(size_t q= q0; q < q0 + csz; ++q)
     if(!(q >> k & 1)) inv_pair(F + q * BSF, F + (q | size_t(1) << k) * BSF, __builtin_popcountll(q >> (k + 1)), k, 0, ns);
   for(size_t q= q0; q < q0 + csz; ++q) out_block(F + q * BSF, __builtin_popcountll(q), ns, o + 16 * q, Kc);
  }
 }
 std::free(B), std::free(F), std::free(hs);
}

inline std::vector<u32> log(int n, const std::vector<u32>& h) {
 std::vector<u32> out(size_t(1) << n);
 if(n < 5) {
  naive_log(n, h.data(), out.data());
  return out;
 }
 switch(n) {
#define XL_CASE(m) \
 case m: log_n<m>(h.data(), out.data()); break;
  XL_CASE(5) XL_CASE(6) XL_CASE(7) XL_CASE(8) XL_CASE(9) XL_CASE(10) XL_CASE(11) XL_CASE(12) XL_CASE(13) XL_CASE(14) XL_CASE(15) XL_CASE(16) XL_CASE(17) XL_CASE(18) XL_CASE(19) XL_CASE(20)
#undef XL_CASE
 }
 return out;
}
}  // namespace xl1

inline std::vector<xl1::u32> run(int N, const std::vector<xl1::u32>& b) { return xl1::log(N, b); }
