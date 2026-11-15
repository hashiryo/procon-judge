#pragma once
// 多項式と集合冪級数の合成 f(b)。rank 付きの XOR 変換 (補集合の対称で半分) の領域で 1 bit ずつ伸ばす。
//   c0 = b[∅]、g = b - c0、e_k = f^{(k)}(c0) とすると f(b) = Σ_k e_k g^k / k! = E(g) (E は e を係数に持つ EGF)。
//   G_j^{(i)} = E^{(j)}(g を [0, i) に制限したもの) とすると G_j^{(i+1)} = [G_j^{(i)}, G_{j+1}^{(i)} * g_i] (g_i[S'] = g[S' ∪ {i}])。
//   変換の領域では Ĝ_j^{(i+1)}[y ±] = Ĝ_j^{(i)}[y] ± t Ĝ_{j+1}^{(i)}[y] ĝ_i[y] で、最後に Ĝ_0^{(N)} を 1 回だけ逆変換する。
//   部分集合の大きさより rank の大きい項は大きさと等しい rank に混ざらないので、Ĝ^{(i)} は rank i までで切る。
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

namespace xc1 {
using u32= std::uint32_t;
using u64= std::uint64_t;
using V= __m256i;
#define XC_AI __attribute__((always_inline)) inline

constexpr u32 P= 998244353, P2= 2 * P;
constexpr u32 inv32(u32 n) {
 u32 x= n;
 for(int i= 0; i < 5; ++i) x*= 2u - n * x;
 return x;
}
constexpr u32 NPINV= 0u - inv32(P);
constexpr u32 R1= u32((u64(1) << 32) % P), R2= u32(u64(R1) * R1 % P);
inline u32 pw(u64 a, u64 e) {
 u64 r= 1;
 for(a%= P; e; e>>= 1, a= a * a % P)
  if(e & 1) r= r * a % P;
 return (u32)r;
}
constexpr int KC= 9;

XC_AI V ld(const u32* p) { return _mm256_load_si256((const V*)p); }
XC_AI void st(u32* p, V x) { _mm256_store_si256((V*)p, x); }
XC_AI V cst(u32 x) { return _mm256_set1_epi32((int)x); }
XC_AI V red2(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P2))); }
XC_AI V neg2(V x) { return _mm256_sub_epi32(cst(P2), x); }
XC_AI V add2(V a, V b) { return red2(_mm256_add_epi32(a, b)); }
XC_AI V sub2(V a, V b) { return red2(_mm256_add_epi32(a, neg2(b))); }
XC_AI V norm1(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P))); }
XC_AI V mul64(V a, V b) { return _mm256_mul_epu32(a, b); }
XC_AI V redc2(V e, V o) {
 const V pv= _mm256_set1_epi64x(P), nv= _mm256_set1_epi64x(NPINV);
 V te= _mm256_add_epi64(e, mul64(mul64(e, nv), pv));
 V to= _mm256_add_epi64(o, mul64(mul64(o, nv), pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(te, 32), to, 0xAA);
}
// 8 レーンの x y R^{-1} ([0, 2p))。
XC_AI V mont(V x, V y) { return redc2(mul64(x, y), mul64(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32))); }
XC_AI V lane_wht(V x) {
 V t= _mm256_shuffle_epi32(x, 0xB1);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xAA)));
 t= _mm256_shuffle_epi32(x, 0x4E);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xCC)));
 t= _mm256_permute2x128_si256(x, x, 0x01);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xF0)));
 return x;
}
XC_AI V mpc(int j) {
 auto b= [&](int r) { return __builtin_popcount(r) == j ? -1 : 0; };
 return _mm256_setr_epi32(b(0), b(1), b(2), b(3), b(4), b(5), b(6), b(7));
}
// 塊には rank r の値を場所 r - sb に置き、場所は [0, ns) だけを持つ。
XC_AI void init_block(u32* X, const u32* in, int lo, int sb, int ns) {
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
XC_AI void fwd_pair(u32* X, u32* Y, int lo, int k, int sb, int ns) {
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
XC_AI void inv_pair(u32* X, u32* Y, int lo, int k, int sb, int ns) {
 if(int s= lo - sb; s >= 0 && s < ns) st(X + 8 * s, add2(ld(X + 8 * s), ld(Y + 8 * s)));
 const int a= std::max(lo + 1 - sb, 0), b= std::min(lo + 4 + k - sb, ns - 1);
 for(int s= a; s <= b; ++s) {
  V x= ld(X + 8 * s), y= ld(Y + 8 * s);
  st(X + 8 * s, add2(x, y)), st(Y + 8 * s, sub2(x, y));
 }
 if(int s= lo + 5 + k - sb; s >= 0 && s < ns) st(Y + 8 * s, sub2(ld(X + 8 * s), ld(Y + 8 * s)));
}
// 1 bit 伸ばす (塊 1 つ分)。F = ĝ'_i (R 倍、rank 0..i を場所 0..i)。G[j] は Ĝ_j の塊 (rank 0..i を場所 0..i)、Gh[j] は上の半分の行き先。
// j = 0..J-1 について D = t Ĝ_{j+1} ĝ_i (rank 1..i+1)、Ĝ_j の下を G_j + D、上を G_j - D (rank 0..i+1) にする。
XC_AI void step_block(u32* const* G, u32* const* Gh, const u32* F, int i, int J) {
 alignas(32) u32 Fe[21 * 8], Fo[21 * 8], Ae[21 * 8], Ao[21 * 8];
 for(int b= 0; b <= i; ++b) {
  V y= norm1(ld(F + 8 * b));
  st(Fe + 8 * b, y), st(Fo + 8 * b, _mm256_srli_epi64(y, 32));
 }
 const V z= _mm256_setzero_si256();
 for(int j= 0; j < J; ++j) {
  const u32* A= G[j + 1];
  for(int a= 0; a <= i; ++a) {
   V x= norm1(ld(A + 8 * a));
   st(Ae + 8 * a, x), st(Ao + 8 * a, _mm256_srli_epi64(x, 32));
  }
  u32 *X= G[j], *Y= Gh[j];
  {
   V g= ld(X);
   st(Y, g);  // rank 0 は D が 0
  }
  for(int d= 1; d <= i + 1; ++d) {
   // D_d = sum_{a=0}^{d-1} A_a F_{d-1-a}
   V D;
   if(d <= 12) {
    V e= z, o= z;
    for(int a= 0; a < d; ++a) {
     e= _mm256_add_epi64(e, mul64(ld(Ae + 8 * a), ld(Fe + 8 * (d - 1 - a))));
     o= _mm256_add_epi64(o, mul64(ld(Ao + 8 * a), ld(Fo + 8 * (d - 1 - a))));
    }
    D= red2(redc2(e, o));
   } else {
    V e1= z, o1= z, e2= z, o2= z;
    for(int a= 0; a < 12; ++a) {
     e1= _mm256_add_epi64(e1, mul64(ld(Ae + 8 * a), ld(Fe + 8 * (d - 1 - a))));
     o1= _mm256_add_epi64(o1, mul64(ld(Ao + 8 * a), ld(Fo + 8 * (d - 1 - a))));
    }
    for(int a= 12; a < d; ++a) {
     e2= _mm256_add_epi64(e2, mul64(ld(Ae + 8 * a), ld(Fe + 8 * (d - 1 - a))));
     o2= _mm256_add_epi64(o2, mul64(ld(Ao + 8 * a), ld(Fo + 8 * (d - 1 - a))));
    }
    D= add2(red2(redc2(e1, o1)), red2(redc2(e2, o2)));
   }
   V g= d <= i ? ld(X + 8 * d) : z;
   st(Y + 8 * d, sub2(g, D));
   st(X + 8 * d, add2(g, D));
  }
 }
}
// 塊 q のレーンの逆変換と答えの取り出し。rank r は場所 r。out = Q K R^{-1}。
XC_AI void out_block(const u32* X, int c, int ns, u32* out, V Kc) {
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
 for(int t= 0; t < 2; ++t) _mm256_storeu_si256((V*)(out + 8 * t), norm1(mont(q[t], Kc)));
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
// e_k = f^{(k)}(c0) (k = 0..n)。微分を n 回、それぞれを c0 で評価する。
inline std::vector<u32> taylor(const std::vector<u32>& f, u32 c0, int n) {
 const int M= f.size();
 std::vector<u32> e(n + 1, 0);
 if(M == 0) return e;
 const size_t L= (M + 15) & ~15;
 u32 *d= (u32*)std::aligned_alloc(32, L * 4 + 64), *pc= (u32*)std::aligned_alloc(32, L * 4 + 64), *mr= (u32*)std::aligned_alloc(32, L * 4 + 64);
 std::memset(d, 0, L * 4 + 64);
 for(int m= 0; m < M; ++m) d[m]= f[m];
 {
  u64 x= R1;  // c0^m R
  for(size_t m= 0; m < L; ++m) pc[m]= u32(x), x= x * c0 % P;
  for(size_t m= 0; m < L; ++m) mr[m]= u32(u64(m + 1) * R1 % P);
 }
 for(int k= 0, len= M; k <= n && len > 0; ++k, --len) {
  // e_k = sum_m d_m c0^m
  V acc= _mm256_setzero_si256();
  for(int m= 0; m < len; m+= 8) acc= add2(acc, red2(mont(_mm256_loadu_si256((const V*)(d + m)), ld(pc + m))));
  alignas(32) u32 t[8];
  st(t, acc);
  u64 s= 0;
  for(int r= 0; r < 8; ++r) s+= t[r];
  e[k]= u32(s % P);
  // d_m <- d_{m+1} (m+1)
  for(int m= 0; m + 1 < len; m+= 8) _mm256_storeu_si256((V*)(d + m), norm1(mont(_mm256_loadu_si256((const V*)(d + m + 1)), ld(mr + m))));
  for(int m= len - 1; m < len + 8 && m < (int)L; ++m) d[m]= 0;
 }
 std::free(d), std::free(pc), std::free(mr);
 return e;
}

template <int N> void comp_n(const u32* g, const std::vector<u32>& e, u32* out) {
 // g: g[0] = 0 の素の値。G_j^{(4)} を素朴に作り、i = 4..N-1 を塊で伸ばす。
 constexpr int NB= N - 4;
 u32* gs= huge_alloc(size_t(4) << N);
 {
  const V r2= cst(R2);
  for(size_t s= 0; s < (size_t(1) << N); s+= 8) _mm256_store_si256((V*)(gs + s), norm1(mont(_mm256_loadu_si256((const V*)(g + s)), r2)));
 }
 // Ĝ_j は j = 0..N-4。Ĝ_j の塊は最後に 2^{N-j-4} 個、rank 0..N-j。
 u32* G[N - 3];
 int bsj[N - 3];
 for(int j= 0; j <= N - 4; ++j) bsj[j]= (N - j + 1) * 8, G[j]= huge_alloc((size_t(1) << (N - j - 4)) * bsj[j] * 4);
 {
  // G_j^{(i)} を [0, 4) で素朴に作る
  std::vector<std::vector<u32>> h(N + 1, std::vector<u32>(16, 0));
  for(int j= 0; j <= N; ++j) h[j][0]= e[j];
  for(int i= 0; i < 4; ++i)
   for(int j= 0; j + i < N; ++j)
    for(int s= 0; s < (1 << i); ++s) {
     u64 t= 0;
     for(int a= s;; a= (a - 1) & s) {
      t= (t + u64(h[j + 1][a]) * g[(1 << i) | (s ^ a)]) % P;
      if(!a) break;
     }
     h[j][(1 << i) | s]= u32(t);
    }
  for(int j= 0; j <= N - 4; ++j) init_block(G[j], h[j].data(), 0, 0, 5);
 }
 constexpr int BSF= N * 8;
 u32* F= huge_alloc((size_t(1) << (N > 5 ? N - 5 : 0)) * BSF * 4);
 alignas(32) static u32 cF[(size_t(1) << (N - 5 > KC ? N - 5 - KC : 0)) * BSF];
 for(int i= 4; i < N; ++i) {
  const int nb= i - 4, K= std::min(nb, KC), H= nb - K, ns= i + 1, J= N - i;
  const size_t cnt= size_t(1) << nb, csz= size_t(1) << K;
  const u32* in= gs + (size_t(1) << i);
  for(size_t q0= 0; q0 < cnt; q0+= csz) {
   for(size_t q= q0; q < q0 + csz; ++q) init_block(F + q * BSF, in + 16 * q, __builtin_popcountll(q), 0, ns);
   for(int k= 0; k < K; ++k)
    for(size_t q= q0; q < q0 + csz; ++q)
     if(!(q >> k & 1)) fwd_pair(F + q * BSF, F + (q | size_t(1) << k) * BSF, __builtin_popcountll(q >> (k + 1)), k, 0, ns);
  }
  u32 *gp[N - 3], *gh[N - 3];
  auto step= [&](size_t q, const u32* Fb) {
   for(int j= 0; j <= J; ++j) gp[j]= G[j] + q * bsj[j];
   for(int j= 0; j < J; ++j) gh[j]= G[j] + (q + cnt) * bsj[j];
   step_block(gp, gh, Fb, i, J);
  };
  if(H == 0) {
   for(size_t q= 0; q < cnt; ++q) step(q, F + q * BSF);
  } else {
   const size_t hs= size_t(1) << H;
   for(size_t jc= 0; jc < csz; ++jc) {
    for(size_t h= 0; h < hs; ++h) std::memcpy(cF + h * BSF, F + ((h << K) + jc) * BSF, size_t(ns) * 32);
    for(int kk= 0; kk < H; ++kk)
     for(size_t h= 0; h < hs; ++h)
      if(!(h >> kk & 1)) fwd_pair(cF + h * BSF, cF + (h | size_t(1) << kk) * BSF, __builtin_popcountll(h >> (kk + 1)), kk + K, 0, ns);
    for(size_t h= 0; h < hs; ++h) step((h << K) + jc, cF + h * BSF);
   }
  }
 }
 // Ĝ_0 の逆変換と取り出し
 constexpr int K= NB < KC ? NB : KC, H= NB - K, NS= N + 1, BS= NS * 8;
 u32* G0= G[0];
 const size_t nblk= size_t(1) << NB, csz= size_t(1) << K;
 if constexpr(H > 0) {
  constexpr size_t hs= size_t(1) << H;
  u32* cG= (u32*)std::aligned_alloc(64, hs * BS * 4);
  for(size_t j= 0; j < csz; ++j) {
   for(size_t h= 0; h < hs; ++h) std::memcpy(cG + h * BS, G0 + ((h << K) + j) * BS, BS * 4);
   for(int kk= H; kk--;)
    for(size_t h= 0; h < hs; ++h)
     if(!(h >> kk & 1)) inv_pair(cG + h * BS, cG + (h | size_t(1) << kk) * BS, __builtin_popcountll(h >> (kk + 1)), kk + K, 0, NS);
   for(size_t h= 0; h < hs; ++h) std::memcpy(G0 + ((h << K) + j) * BS, cG + h * BS, BS * 4);
  }
  std::free(cG);
 }
 const V Kc= cst(u32(u64(pw(2, u64(P - 1) * 64 - (N - 1))) * R1 % P));
 for(size_t q0= 0; q0 < nblk; q0+= csz) {
  for(int k= K; k--;)
   for(size_t q= q0; q < q0 + csz; ++q)
    if(!(q >> k & 1)) inv_pair(G0 + q * BS, G0 + (q | size_t(1) << k) * BS, __builtin_popcountll(q >> (k + 1)), k, 0, NS);
  for(size_t q= q0; q < q0 + csz; ++q) out_block(G0 + q * BS, __builtin_popcountll(q), NS, out + 16 * q, Kc);
 }
 out[0]= e[0];
 for(int j= 0; j <= N - 4; ++j) std::free(G[j]);
 std::free(F), std::free(gs);
}

inline std::vector<u32> composite(const std::vector<u32>& f, int n, const std::vector<u32>& b) {
 std::vector<u32> out(size_t(1) << n, 0);
 if(f.empty()) return out;
 std::vector<u32> e= taylor(f, b[0], n);
 std::vector<u32> g(b);
 g[0]= 0;
 if(n < 5) {
  std::vector<std::vector<u32>> h(n + 1, std::vector<u32>(size_t(1) << n, 0));
  for(int j= 0; j <= n; ++j) h[j][0]= e[j];
  for(int i= 0; i < n; ++i)
   for(int j= 0; j + i < n; ++j)
    for(int s= 0; s < (1 << i); ++s) {
     u64 t= 0;
     for(int a= s;; a= (a - 1) & s) {
      t= (t + u64(h[j + 1][a]) * g[(1 << i) | (s ^ a)]) % P;
      if(!a) break;
     }
     h[j][(1 << i) | s]= u32(t);
    }
  return h[0];
 }
 switch(n) {
#define XC_CASE(m) \
 case m: comp_n<m>(g.data(), e, out.data()); break;
  XC_CASE(5) XC_CASE(6) XC_CASE(7) XC_CASE(8) XC_CASE(9) XC_CASE(10) XC_CASE(11) XC_CASE(12) XC_CASE(13) XC_CASE(14) XC_CASE(15) XC_CASE(16) XC_CASE(17) XC_CASE(18) XC_CASE(19) XC_CASE(20)
#undef XC_CASE
 }
 return out;
}
}  // namespace xc1

inline std::vector<xc1::u32> run(int M, int N, const std::vector<xc1::u32>& f, const std::vector<xc1::u32>& b) {
 (void)M;
 return xc1::composite(f, N, b);
}
