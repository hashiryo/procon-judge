#pragma once
// 集合冪級数の exp。rank 付きの XOR 変換 (補集合の対称で半分) の領域で、1 bit ずつ伸ばす。
//   G^{(i)} = exp(b を [0, i) に制限したもの)、f_i[S'] = b[S' ∪ {i}] (S' ⊆ [0, i)) とすると G^{(i+1)} = [G^{(i)}, G^{(i)} * f_i]。
//   rank 付きの XOR 変換では、要素 i の分の t を掛けて Ĝ^{(i+1)}[y ± ] = Ĝ^{(i)}[y] (1 ± t f̂_i[y]) になる。
//   部分集合の大きさより rank の大きい項 (重なる組から出るごみ) は、掛けても足してもごみのままで、大きさと等しい rank の
//   項に混ざらない。そのため途中で逆変換して rank を取り出す必要がなく、最後に 1 回だけ逆変換する。ごみは捨ててよいので、
//   Ĝ^{(i)} は rank i までで切る。割り算を使わない。
// 変換と窓 (値の 0 でない rank の範囲だけを変換する) は subset-convolution の xw1 と同じ形。
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

namespace xe1 {
using u32= std::uint32_t;
using u64= std::uint64_t;
using V= __m256i;
#define XE_AI __attribute__((always_inline)) inline

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
constexpr int KC= 9;  // 区画の塊の数は 2^KC

XE_AI V ld(const u32* p) { return _mm256_load_si256((const V*)p); }
XE_AI void st(u32* p, V x) { _mm256_store_si256((V*)p, x); }
XE_AI V cst(u32 x) { return _mm256_set1_epi32((int)x); }
XE_AI V red2(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P2))); }
XE_AI V neg2(V x) { return _mm256_sub_epi32(cst(P2), x); }
XE_AI V add2(V a, V b) { return red2(_mm256_add_epi32(a, b)); }
XE_AI V sub2(V a, V b) { return red2(_mm256_add_epi32(a, neg2(b))); }
XE_AI V norm1(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P))); }
XE_AI V mul64(V a, V b) { return _mm256_mul_epu32(a, b); }
XE_AI V redc2(V e, V o) {
 const V pv= _mm256_set1_epi64x(P), nv= _mm256_set1_epi64x(NPINV);
 V te= _mm256_add_epi64(e, mul64(mul64(e, nv), pv));
 V to= _mm256_add_epi64(o, mul64(mul64(o, nv), pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(te, 32), to, 0xAA);
}
XE_AI V lane_wht(V x) {
 V t= _mm256_shuffle_epi32(x, 0xB1);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xAA)));
 t= _mm256_shuffle_epi32(x, 0x4E);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xCC)));
 t= _mm256_permute2x128_si256(x, x, 0x01);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xF0)));
 return x;
}
XE_AI V mpc(int j) {
 auto b= [&](int r) { return __builtin_popcount(r) == j ? -1 : 0; };
 return _mm256_setr_epi32(b(0), b(1), b(2), b(3), b(4), b(5), b(6), b(7));
}
// 塊には rank r の値を場所 r - sb に置き、場所は [0, ns) だけを持つ。
// 入力 16 個 (rank は lo + popcount) を置いてレーンの WHT まで済ませる。lo は塊の上の bit の数 (+ 全体の rank のずれ)。
XE_AI void init_block(u32* X, const u32* in, int lo, int sb, int ns) {
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
// 塊の bit k の変換。X (bit k が 0) は rank [lo, lo+4+k]、Y は [lo+1, lo+5+k] を持ち、どちらも [lo, lo+5+k] になる。
XE_AI void fwd_pair(u32* X, u32* Y, int lo, int k, int sb, int ns) {
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
// 逆変換の塊の bit k。どちらも rank [lo, lo+5+k] を持ち、X は [lo, lo+4+k]、Y は [lo+1, lo+5+k] を残す。
XE_AI void inv_pair(u32* X, u32* Y, int lo, int k, int sb, int ns) {
 if(int s= lo - sb; s >= 0 && s < ns) st(X + 8 * s, add2(ld(X + 8 * s), ld(Y + 8 * s)));
 const int a= std::max(lo + 1 - sb, 0), b= std::min(lo + 4 + k - sb, ns - 1);
 for(int s= a; s <= b; ++s) {
  V x= ld(X + 8 * s), y= ld(Y + 8 * s);
  st(X + 8 * s, add2(x, y)), st(Y + 8 * s, sub2(x, y));
 }
 if(int s= lo + 5 + k - sb; s >= 0 && s < ns) st(Y + 8 * s, sub2(ld(X + 8 * s), ld(Y + 8 * s)));
}
// 1 bit 伸ばす。G (塊、rank 1..i を場所 0..i-1) と F (f̂_i を R 倍したもの、rank 0..i を場所 0..i) から、
// D = Ĝ t f̂ (rank i+1 まで) を作り、Glo = G + D、Ghi = G - D を rank 1..i+1 (場所 0..i) に書く。Glo は G と同じ場所でよい。
XE_AI void step_block(const u32* G, u32* Glo, u32* Ghi, const u32* F, int i) {
 alignas(32) u32 Ge[20 * 8], Go[20 * 8], Fe[21 * 8], Fo[21 * 8];
 for(int a= 1; a <= i; ++a) {
  V x= norm1(ld(G + 8 * (a - 1)));
  st(Ge + 8 * a, x), st(Go + 8 * a, _mm256_srli_epi64(x, 32));
 }
 const V lo32= _mm256_set1_epi64x(0xffffffffu);
 for(int b= 0; b <= i; ++b) {
  V y= norm1(ld(F + 8 * b));
  st(Fe + 8 * b, _mm256_and_si256(y, lo32)), st(Fo + 8 * b, _mm256_srli_epi64(y, 32));
 }
 const V z= _mm256_setzero_si256();
 for(int d= 1; d <= i + 1; ++d) {
  // D_d = F_{d-1} + sum_{a=1}^{d-1} G_a F_{d-1-a}
  V D;
  if(d - 1 <= 12) {
   V e= ld(Fe + 8 * (d - 1)), o= ld(Fo + 8 * (d - 1));
   for(int a= 1; a < d; ++a) {
    e= _mm256_add_epi64(e, mul64(ld(Ge + 8 * a), ld(Fe + 8 * (d - 1 - a))));
    o= _mm256_add_epi64(o, mul64(ld(Go + 8 * a), ld(Fo + 8 * (d - 1 - a))));
   }
   D= red2(redc2(e, o));
  } else {
   V e1= ld(Fe + 8 * (d - 1)), o1= ld(Fo + 8 * (d - 1)), e2= z, o2= z;
   for(int a= 1; a <= 12; ++a) {
    e1= _mm256_add_epi64(e1, mul64(ld(Ge + 8 * a), ld(Fe + 8 * (d - 1 - a))));
    o1= _mm256_add_epi64(o1, mul64(ld(Go + 8 * a), ld(Fo + 8 * (d - 1 - a))));
   }
   for(int a= 13; a < d; ++a) {
    e2= _mm256_add_epi64(e2, mul64(ld(Ge + 8 * a), ld(Fe + 8 * (d - 1 - a))));
    o2= _mm256_add_epi64(o2, mul64(ld(Go + 8 * a), ld(Fo + 8 * (d - 1 - a))));
   }
   D= add2(red2(redc2(e1, o1)), red2(redc2(e2, o2)));
  }
  V g= d <= i ? ld(G + 8 * (d - 1)) : z;
  st(Ghi + 8 * (d - 1), sub2(g, D));
  st(Glo + 8 * (d - 1), add2(g, D));
 }
}
// 塊 q のレーンの逆変換と答えの取り出し。out = Q K R^{-1}。rank r は場所 r - 1。
XE_AI void out_block(const u32* X, int c, int ns, u32* out, V Kc) {
 const V M0= mpc(0), M1= mpc(1), M2= mpc(2), M3= mpc(3), z= _mm256_setzero_si256();
 V v[5];
 for(int j= 0; j < 5; ++j) {
  int s= c + j - 1;
  v[j]= s >= 0 && s < ns ? lane_wht(ld(X + 8 * s)) : z;
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
// 素朴な exp (3^m)。
inline void naive_exp(int m, const u32* b, u32* h) {
 h[0]= 1;
 for(int i= 0; i < m; ++i)
  for(int s= 0; s < (1 << i); ++s) {
   u64 t= 0;
   for(int a= s;; a= (a - 1) & s) {
    t= (t + u64(h[a]) * b[(1 << i) | (s ^ a)]) % P;
    if(!a) break;
   }
   h[(1 << i) | s]= u32(t);
  }
}

template <int N> void exp_n(const u32* b, u32* out) {
 constexpr int NS= N, BS= NS * 8, NB= N - 4;
 const size_t nblk= size_t(1) << NB;
 u32 *G= huge_alloc(nblk * BS * 4), *F= huge_alloc((nblk >> 1) * BS * 4);
 u32* bs= huge_alloc((size_t(4) << N));
 {  // bs = b R
  const V r2= cst(u32((u64(1) << 32) % P * ((u64(1) << 32) % P) % P));
  for(size_t s= 0; s < (size_t(1) << N); s+= 8) {
   V x= _mm256_loadu_si256((const V*)(b + s));
   _mm256_store_si256((V*)(bs + s), norm1(redc2(mul64(x, r2), mul64(_mm256_srli_epi64(x, 32), r2))));
  }
 }
 {  // G^{(4)}
  alignas(32) u32 h[16];
  naive_exp(4, b, h);
  init_block(G, h, 0, 1, NS);
 }
 alignas(32) static u32 cF[(size_t(1) << (NB > KC + 1 ? NB - 1 - KC : 0)) * BS];
 for(int i= 4; i < N; ++i) {
  const int nb= i - 4;  // f̂_i と Ĝ^{(i)} の塊は 2^nb 個
  const int K= std::min(nb, KC), H= nb - K;
  const size_t cnt= size_t(1) << nb, csz= size_t(1) << K;
  const u32* in= bs + (size_t(1) << i);
  // 区画ごとに f̂_i の下の段まで (rank 0..i を場所 0..i)
  for(size_t q0= 0; q0 < cnt; q0+= csz) {
   for(size_t q= q0; q < q0 + csz; ++q) init_block(F + q * BS, in + 16 * q, __builtin_popcountll(q), 0, i + 1);
   for(int k= 0; k < K; ++k)
    for(size_t q= q0; q < q0 + csz; ++q)
     if(!(q >> k & 1)) fwd_pair(F + q * BS, F + (q | size_t(1) << k) * BS, __builtin_popcountll(q >> (k + 1)), k, 0, i + 1);
  }
  if(H == 0) {
   for(size_t q= 0; q < cnt; ++q) step_block(G + q * BS, G + q * BS, G + (q + cnt) * BS, F + q * BS, i);
  } else {
   const size_t hs= size_t(1) << H;
   for(size_t j= 0; j < csz; ++j) {
    for(size_t h= 0; h < hs; ++h) std::memcpy(cF + h * BS, F + ((h << K) + j) * BS, size_t(i + 1) * 32);
    for(int kk= 0; kk < H; ++kk)
     for(size_t h= 0; h < hs; ++h)
      if(!(h >> kk & 1)) fwd_pair(cF + h * BS, cF + (h | size_t(1) << kk) * BS, __builtin_popcountll(h >> (kk + 1)), kk + K, 0, i + 1);
    for(size_t h= 0; h < hs; ++h) {
     size_t q= (h << K) + j;
     step_block(G + q * BS, G + q * BS, G + (q + cnt) * BS, cF + h * BS, i);
    }
   }
  }
 }
 // 逆変換 (上の段は列ごと、下の段は区画ごと) と取り出し
 constexpr int K= NB < KC ? NB : KC, H= NB - K;
 const size_t csz= size_t(1) << K;
 if constexpr(H > 0) {
  constexpr size_t hs= size_t(1) << H;
  u32* cG= (u32*)std::aligned_alloc(64, hs * BS * 4);
  for(size_t j= 0; j < csz; ++j) {
   for(size_t h= 0; h < hs; ++h) std::memcpy(cG + h * BS, G + ((h << K) + j) * BS, BS * 4);
   for(int kk= H; kk--;)
    for(size_t h= 0; h < hs; ++h)
     if(!(h >> kk & 1)) inv_pair(cG + h * BS, cG + (h | size_t(1) << kk) * BS, __builtin_popcountll(h >> (kk + 1)), kk + K, 1, NS);
   for(size_t h= 0; h < hs; ++h) std::memcpy(G + ((h << K) + j) * BS, cG + h * BS, BS * 4);
  }
  std::free(cG);
 }
 const V Kc= cst(u32(u64(pw(2, u64(P - 1) * 64 - (N - 1))) * ((u64(1) << 32) % P) % P));
 for(size_t q0= 0; q0 < nblk; q0+= csz) {
  for(int k= K; k--;)
   for(size_t q= q0; q < q0 + csz; ++q)
    if(!(q >> k & 1)) inv_pair(G + q * BS, G + (q | size_t(1) << k) * BS, __builtin_popcountll(q >> (k + 1)), k, 1, NS);
  for(size_t q= q0; q < q0 + csz; ++q) out_block(G + q * BS, __builtin_popcountll(q), NS, out + 16 * q, Kc);
 }
 out[0]= 1;
 std::free(G), std::free(F), std::free(bs);
}

inline std::vector<u32> exp(int n, const std::vector<u32>& b) {
 std::vector<u32> out(size_t(1) << n);
 if(n < 5) {
  naive_exp(n, b.data(), out.data());
  return out;
 }
 switch(n) {
#define XE_CASE(m) \
 case m: exp_n<m>(b.data(), out.data()); break;
  XE_CASE(5) XE_CASE(6) XE_CASE(7) XE_CASE(8) XE_CASE(9) XE_CASE(10) XE_CASE(11) XE_CASE(12) XE_CASE(13) XE_CASE(14) XE_CASE(15) XE_CASE(16) XE_CASE(17) XE_CASE(18) XE_CASE(19) XE_CASE(20)
#undef XE_CASE
 }
 return out;
}
}  // namespace xe1

inline std::vector<xe1::u32> run(int N, const std::vector<xe1::u32>& b) { return xe1::exp(N, b); }

// ハーネスの形 (Solver: 構築 / run / answer) に合わせる。計測するのは上の run の呼び出しだけ。
struct Solver {
 int N;
 const vector<u32>& b;
 vector<u32> r;
 Solver(int N_, const vector<u32>& b_): N(N_), b(b_) {}
 void run() { r= ::run(N, b); }
 vector<u32> answer() const { return r; }
};
