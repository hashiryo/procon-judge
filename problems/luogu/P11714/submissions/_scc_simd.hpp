#pragma once
// 主旋律の 3^n の和を AVX2 で求める形 (scc_simd が読む)。g[S] = 2^{e(S)} - sum_{∅≠T⊊S} g[T] 2^{e(S\T) + c(T→S\T)} を S の小さい順に
// 求め、強連結な辺の部分集合の数 f は f = -log(1 - g) (g = 1 - exp(-f)、集合冪級数の log) で NeoLibrary の sps::log から出す。
// 添字の下の 3 bit をレーンに置き (塊 = 上の bit が同じ 8 個)、上の bit は T の上の部分 T_h と U = S \ T の上の部分 U_h の組で回す。
// 肩 e(U) + c(T→U) は、T = (T_h, T_l)、U = (U_h, U_l) で
//   [e(U_h) + c(T_h→U_h)] + c(T_h→U_l) + [c(T_l→U_h) + x(U_h, U_l)] + [e(U_l) + c(T_l→U_l)]
// に分かれる (x は U_h と U_l の間の辺の数、両向き)。レーンを S_l、U_l = d とすると T_l = S_l ^ d なので、
//   Gd[T_h][d][S_l] = [d ⊆ S_l] g[T_h, S_l ^ d] 2^{c(T_h→d) + e(d) + c(S_l^d → d)}、Cd[U_h][d][S_l] = 2^{c(S_l^d → U_h) + x(U_h, d)}
// を先に作れば、組 (T_h, U_h) の寄与は 2^{e(U_h) + c(T_h→U_h)} sum_d Gd[T_h][d] Cd[U_h][d] (レーンごと) になる。値は Montgomery の
// 表現で [0, p) に持ち、8 項の積を u64 にためて 1 回で還元する。U_h = ∅ (同じ塊の中の T_l ⊊ S_l) は、塊ごとにレーンの小さい順に
// 1 つずつ引く。比べ終えたら消す。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/log.hpp"
namespace scc_simd {
using u32= std::uint32_t;
using u64= std::uint64_t;
using V= __m256i;
constexpr u32 P= 1000000007;
constexpr u32 inv32(u32 n) {
 u32 x= n;
 for(int i= 0; i < 5; ++i) x*= 2u - n * x;
 return x;
}
constexpr u32 NP= 0u - inv32(P);
constexpr u32 R1= u32((u64(1) << 32) % P), R2= u32(u64(R1) * R1 % P);
inline u32 mmul(u32 a, u32 b) {  // a b R^{-1} を [0, p) に
 const u64 t= u64(a) * b;
 const u32 m= u32(t) * NP;
 const u64 r= (t + u64(m) * P) >> 32;
 return u32(r >= P ? r - P : r);
}
[[gnu::always_inline]] inline V mul64(V a, V b) { return _mm256_mul_epu32(a, b); }
// e は偶数のレーン、o は奇数のレーンの u64 の和 (2^63 未満)。それぞれ x R^{-1} に還元して [0, 2p) で 8 レーンに並べる。
[[gnu::always_inline]] inline V redc2(V e, V o) {
 const V np= _mm256_set1_epi64x(NP), p= _mm256_set1_epi64x(P);
 const V te= _mm256_add_epi64(e, mul64(mul64(e, np), p)), to= _mm256_add_epi64(o, mul64(mul64(o, np), p));
 return _mm256_blend_epi32(_mm256_srli_epi64(te, 32), to, 0xAA);
}
[[gnu::always_inline]] inline V norm1(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(P))); }  // [0, 2p) -> [0, p)
// g を求める (g[∅] = 0)。n >= 3。
inline std::vector<u32> twisted_g(int n, const std::vector<std::pair<int, int>>& es) {
 const int N= 1 << n, H= n - 3, NH= 1 << H;
 std::vector<u32> out(n, 0);
 for(auto [a, b]: es) out[a]|= 1u << b;
 auto cto= [&](u32 X, u32 Y) {  // c(X→Y)
  int r= 0;
  for(u32 t= X; t; t&= t - 1) r+= __builtin_popcount(out[__builtin_ctz(t)] & Y);
  return r;
 };
 std::vector<int> e(N, 0);
 for(int s= 1; s < N; ++s) {
  const int v= __builtin_ctz(s), r= s & (s - 1);
  e[s]= e[r] + __builtin_popcount(out[v] & r) + cto(u32(r), 1u << v);
 }
 std::vector<u32> pw(n * n + 1);
 pw[0]= R1;
 for(size_t i= 1; i < pw.size(); ++i) pw[i]= u32(u64(pw[i - 1]) * 2 % P);
 // 表: Bt[T_h][d] = c(T_h→d)、Ct[t][U_h] = c(t→U_h)、E[t][d] = e(d) + c(t→d)
 std::vector<int> Bt(size_t(NH) * 8), Ct(size_t(8) * NH);
 for(int th= 0; th < NH; ++th)
  for(int d= 0; d < 8; ++d) Bt[size_t(th) * 8 + d]= cto(u32(th) << 3, u32(d));
 for(int t= 0; t < 8; ++t)
  for(int uh= 0; uh < NH; ++uh) Ct[size_t(t) * NH + uh]= cto(u32(t), u32(uh) << 3);
 int E[8][8];
 for(int t= 0; t < 8; ++t)
  for(int d= 0; d < 8; ++d) E[t][d]= (t & d) ? 0 : e[d] + cto(u32(t), u32(d));
 // Cd[U_h][d] (8 レーン)
 V* Cd= static_cast<V*>(std::aligned_alloc(32, size_t(NH) * 8 * sizeof(V)));  // vector<__m256i> は gcc が属性を落とす警告を出すので
 for(int uh= 0; uh < NH; ++uh)
  for(int d= 0; d < 8; ++d) {
   alignas(32) u32 lane[8];
   const int x= Bt[size_t(uh) * 8 + d] + Ct[size_t(d) * NH + uh];
   for(int sl= 0; sl < 8; ++sl) lane[sl]= (d & ~sl) ? 0 : pw[Ct[size_t(sl ^ d) * NH + uh] + x];
   Cd[size_t(uh) * 8 + d]= _mm256_load_si256((const V*)lane);
  }
 V* Gd= static_cast<V*>(std::aligned_alloc(32, size_t(NH) * 8 * sizeof(V)));
 std::vector<u32> g(N, 0);
 std::vector<int> cH(NH, 0);
 const V P2= _mm256_set1_epi32(2 * P);
 for(int sh= 0; sh < NH; ++sh) {
  alignas(32) u32 acc[8];
  for(int sl= 0; sl < 8; ++sl) acc[sl]= pw[e[(sh << 3) | sl]];
  V a= _mm256_load_si256((const V*)acc);
  // cH[T_h] = c(T_h→S_h) を T_h の小さい順に
  for(int th= (0 - sh) & sh; th; th= (th - sh) & sh) cH[th]= cH[th & (th - 1)] + __builtin_popcount(out[__builtin_ctz(th) + 3] & (u32(sh) << 3));
  cH[0]= 0;
  if(sh) {
   for(int th= (sh - 1) & sh;; th= (th - 1) & sh) {
    const int uh= sh ^ th;
    const int A= e[uh << 3] + cH[th] - e[th << 3];
    const V* gp= &Gd[size_t(th) * 8];
    const V* cp= &Cd[size_t(uh) * 8];
    V se= _mm256_setzero_si256(), so= se;
    for(int d= 0; d < 8; ++d) {
     const V x= gp[d], y= cp[d];
     se= _mm256_add_epi64(se, mul64(x, y)), so= _mm256_add_epi64(so, mul64(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
    }
    V r= norm1(redc2(se, so));
    const V w= _mm256_set1_epi32(int(pw[A]));
    r= redc2(mul64(r, w), mul64(_mm256_srli_epi64(r, 32), w));  // [0, 2p)
    a= _mm256_sub_epi32(_mm256_add_epi32(a, P2), r);             // [0, 3p)
    a= _mm256_min_epu32(a, _mm256_sub_epi32(a, P2));
    a= _mm256_min_epu32(a, _mm256_sub_epi32(a, _mm256_set1_epi32(P)));
    if(!th) break;
   }
  }
  _mm256_store_si256((V*)acc, a);
  // 同じ塊の中 (U_h = ∅、T_l ⊊ S_l、T ≠ ∅)
  u32* gs= &g[size_t(sh) << 3];
  for(int sl= 0; sl < 8; ++sl) {
   u64 v= acc[sl] >= P ? acc[sl] - P : acc[sl];
   for(int tl= (sl - 1) & sl;; tl= (tl - 1) & sl) {
    if(sh || tl) {
     const int ul= sl ^ tl;
     v+= mmul(P - gs[tl], pw[Bt[size_t(sh) * 8 + ul] + E[tl][ul]]);
    }
    if(!tl) break;
   }
   gs[sl]= u32(v % P);
   if(sl == 0 && sh == 0) gs[0]= 0;  // g[∅] = 0
  }
  // Gd[S_h][d]
  for(int d= 0; d < 8; ++d) {
   alignas(32) u32 lane[8];
   for(int sl= 0; sl < 8; ++sl) lane[sl]= (d & ~sl) ? 0 : mmul(gs[sl ^ d], pw[Bt[size_t(sh) * 8 + d] + E[sl ^ d][d]]);
   Gd[size_t(sh) * 8 + d]= _mm256_load_si256((const V*)lane);
  }
 }
 std::free(Cd), std::free(Gd);
 for(auto& x: g) x= mmul(x, 1);  // Montgomery の表現から戻す
 return g;
}
}  // namespace scc_simd
struct Solver {
 using Z= ZMod<1000000007>;
 int n;
 vector<pair<int, int>> es;
 u32 ans= 0;
 Solver(int n_, const vector<pair<int, int>>& es_): n(n_), es(es_) {}
 void run() {
  if(n < 3) {  // 小さいときは素朴に (1 頂点は 1、2 頂点は両向きの辺が揃うときだけ 1)
   if(n == 1) ans= 1;
   else ans= es.size() == 2 ? 1 : 0;
   return;
  }
  const std::vector<u32> g= scc_simd::twisted_g(n, es);
  std::vector<Z> h(g.size());
  h[0]= Z(1);
  for(size_t i= 1; i < g.size(); ++i) h[i]= Z() - Z::raw(g[i]);
  const std::vector<Z> f= sps::log(h);
  ans= (Z() - f.back()).val();
 }
 u32 answer() const { return ans; }
};
