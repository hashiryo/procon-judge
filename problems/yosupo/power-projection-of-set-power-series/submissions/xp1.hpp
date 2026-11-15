#pragma once
// 集合冪級数の power projection。ans_m = Σ_S w[S] [S] a^m (m = 0..M-1)。合成の手順 (polynomial-composite の xc1) の転置。
//   c0 = a[∅]、g = a - c0、V_k = Σ_S w[S] (g^k / k!)[S] とすると ans_m = Σ_k (m)_k c0^{m-k} V_k ((m)_k は下降階乗)。
//   V = Φ^T w で、Φ は e ↦ Σ_k e_k g^k / k!。Φ は「Ĝ_j^{(4)} を素朴に作る → 1 bit ずつ Ĝ_j^{(i+1)}[y ±] = Ĝ_j^{(i)} ± t ĝ_i Ĝ_{j+1}^{(i)}
//   → Ĝ_0^{(N)} を逆変換して取り出す」なので、転置は「w を順変換 (2/2^N 倍) → 上の bit から
//   Ĝ_j^T(i) = (下 + 上)|_{rank ≤ i} + corr(ĝ_i, 下 - 上 (j-1 の分)) → 4 bit の分を素朴に戻す」になる。corr は t ĝ を掛ける写像の転置。
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

namespace xp1 {
using u32= std::uint32_t;
using u64= std::uint64_t;
using V= __m256i;
#define XP_AI __attribute__((always_inline)) inline

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

XP_AI V ld(const u32* p) { return _mm256_load_si256((const V*)p); }
XP_AI void st(u32* p, V x) { _mm256_store_si256((V*)p, x); }
XP_AI V cst(u32 x) { return _mm256_set1_epi32((int)x); }
XP_AI V red2(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P2))); }
XP_AI V neg2(V x) { return _mm256_sub_epi32(cst(P2), x); }
XP_AI V add2(V a, V b) { return red2(_mm256_add_epi32(a, b)); }
XP_AI V sub2(V a, V b) { return red2(_mm256_add_epi32(a, neg2(b))); }
XP_AI V norm1(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, cst(P))); }
XP_AI V mul64(V a, V b) { return _mm256_mul_epu32(a, b); }
XP_AI V redc2(V e, V o) {
 const V pv= _mm256_set1_epi64x(P), nv= _mm256_set1_epi64x(NPINV);
 V te= _mm256_add_epi64(e, mul64(mul64(e, nv), pv));
 V to= _mm256_add_epi64(o, mul64(mul64(o, nv), pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(te, 32), to, 0xAA);
}
XP_AI V mont(V x, V y) { return redc2(mul64(x, y), mul64(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32))); }
XP_AI V lane_wht(V x) {
 V t= _mm256_shuffle_epi32(x, 0xB1);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xAA)));
 t= _mm256_shuffle_epi32(x, 0x4E);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xCC)));
 t= _mm256_permute2x128_si256(x, x, 0x01);
 x= red2(_mm256_add_epi32(t, _mm256_blend_epi32(x, neg2(x), 0xF0)));
 return x;
}
XP_AI V mpc(int j) {
 auto b= [&](int r) { return __builtin_popcount(r) == j ? -1 : 0; };
 return _mm256_setr_epi32(b(0), b(1), b(2), b(3), b(4), b(5), b(6), b(7));
}
XP_AI void init_block(u32* X, const u32* in, int lo, int sb, int ns) {
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
XP_AI void fwd_pair(u32* X, u32* Y, int lo, int k, int sb, int ns) {
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
// 転置の 1 段 (塊 1 つ分)。F = ĝ'_i (R 倍、rank 0..i)。Zl[j], Zh[j] (j < J) は段 i+1 の双対の下と上 (rank 0..i+1)。
// Y[j] (j = 0..J) に段 i の双対 (rank 0..i) を書く。Y[j] は Zl[j] と同じ場所でよい。
XP_AI void step_block(u32* const* Zl, u32* const* Zh, u32* const* Y, const u32* F, int i, int J) {
 alignas(32) u32 Fe[21 * 8], Fo[21 * 8], De[2][22 * 8], Do[2][22 * 8], S[22 * 8];
 for(int b= 0; b <= i; ++b) {
  V y= norm1(ld(F + 8 * b));
  st(Fe + 8 * b, y), st(Fo + 8 * b, _mm256_srli_epi64(y, 32));
 }
 const V z= _mm256_setzero_si256();
 for(int j= 0; j <= J; ++j) {
  const int cur= j & 1, prv= cur ^ 1;
  // 和 (下 + 上) と、次の j のための差 (下 - 上) を、Y[j] (Zl[j] と同じ場所) を書く前に取る
  if(j < J) {
   for(int r= 0; r <= i; ++r) st(S + 8 * r, add2(ld(Zl[j] + 8 * r), ld(Zh[j] + 8 * r)));
   for(int d= 1; d <= i + 1; ++d) {
    V x= norm1(sub2(ld(Zl[j] + 8 * d), ld(Zh[j] + 8 * d)));
    st(De[cur] + 8 * d, x), st(Do[cur] + 8 * d, _mm256_srli_epi64(x, 32));
   }
  }
  const u32 *de= De[prv], *dO= Do[prv];
  for(int a= 0; a <= i; ++a) {
   V y= j < J ? ld(S + 8 * a) : z;
   if(j >= 1) {
    // corr_a = sum_{d=a+1}^{i+1} F_{d-1-a} D_d (D は j-1 の差)
    const int nt= i + 1 - a;
    V c;
    if(nt <= 12) {
     V e= z, o= z;
     for(int d= a + 1; d <= i + 1; ++d) {
      e= _mm256_add_epi64(e, mul64(ld(Fe + 8 * (d - 1 - a)), ld(de + 8 * d)));
      o= _mm256_add_epi64(o, mul64(ld(Fo + 8 * (d - 1 - a)), ld(dO + 8 * d)));
     }
     c= red2(redc2(e, o));
    } else {
     V e1= z, o1= z, e2= z, o2= z;
     for(int d= a + 1; d <= a + 12; ++d) {
      e1= _mm256_add_epi64(e1, mul64(ld(Fe + 8 * (d - 1 - a)), ld(de + 8 * d)));
      o1= _mm256_add_epi64(o1, mul64(ld(Fo + 8 * (d - 1 - a)), ld(dO + 8 * d)));
     }
     for(int d= a + 13; d <= i + 1; ++d) {
      e2= _mm256_add_epi64(e2, mul64(ld(Fe + 8 * (d - 1 - a)), ld(de + 8 * d)));
      o2= _mm256_add_epi64(o2, mul64(ld(Fo + 8 * (d - 1 - a)), ld(dO + 8 * d)));
     }
     c= add2(red2(redc2(e1, o1)), red2(redc2(e2, o2)));
    }
    y= add2(y, c);
   }
   st(Y[j] + 8 * a, y);
  }
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
// c_m = g^m / m! (m = 0..n) を素朴に (最小の要素を含む塊を選ぶ)。
inline std::vector<std::vector<u32>> egf_powers(int n, const u32* g) {
 const int L= 1 << n;
 std::vector<std::vector<u32>> c(n + 1, std::vector<u32>(L, 0));
 c[0][0]= 1;
 for(int m= 1; m <= n; ++m)
  for(int s= 1; s < L; ++s) {
   const int lo= s & -s, rest= s ^ lo;
   u64 t= 0;
   for(int a= rest;; a= (a - 1) & rest) {  // 塊 = lo ∪ a
    t= (t + u64(g[lo | a]) * c[m - 1][rest ^ a]) % P;
    if(!a) break;
   }
   c[m][s]= u32(t);
  }
 return c;
}

template <int N> std::vector<u32> vk_n(const u32* g, const u32* w) {
 // 段 N の双対 = w の順変換 (rank 0..N を場所 0..N)。2/2^N は最後に掛ける。
 constexpr int NB= N - 4, NS= N + 1;
 u32* gs= huge_alloc(size_t(4) << N);
 {
  const V r2= cst(R2);
  for(size_t s= 0; s < (size_t(1) << N); s+= 8) _mm256_store_si256((V*)(gs + s), norm1(mont(_mm256_loadu_si256((const V*)(g + s)), r2)));
 }
 // 双対の配列。j の配列は段 i (j ≤ N - i) で 2^{i-4} 個の塊、rank 0..i。最大は段 N - j。
 u32* D[N - 3];
 int bsj[N - 3];
 for(int j= 0; j <= N - 4; ++j) bsj[j]= (N - j + 1) * 8, D[j]= huge_alloc((size_t(1) << (N - j - 4)) * bsj[j] * 4);
 {
  constexpr int K= NB < KC ? NB : KC, H= NB - K, BS= NS * 8;
  const size_t nblk= size_t(1) << NB, csz= size_t(1) << K;
  u32* X= D[0];
  for(size_t q0= 0; q0 < nblk; q0+= csz) {
   for(size_t q= q0; q < q0 + csz; ++q) init_block(X + q * BS, w + 16 * q, __builtin_popcountll(q), 0, NS);
   for(int k= 0; k < K; ++k)
    for(size_t q= q0; q < q0 + csz; ++q)
     if(!(q >> k & 1)) fwd_pair(X + q * BS, X + (q | size_t(1) << k) * BS, __builtin_popcountll(q >> (k + 1)), k, 0, NS);
  }
  if constexpr(H > 0) {
   constexpr size_t hs= size_t(1) << H;
   u32* c= (u32*)std::aligned_alloc(64, hs * BS * 4);
   for(size_t j= 0; j < csz; ++j) {
    for(size_t h= 0; h < hs; ++h) std::memcpy(c + h * BS, X + ((h << K) + j) * BS, BS * 4);
    for(int kk= 0; kk < H; ++kk)
     for(size_t h= 0; h < hs; ++h)
      if(!(h >> kk & 1)) fwd_pair(c + h * BS, c + (h | size_t(1) << kk) * BS, __builtin_popcountll(h >> (kk + 1)), kk + K, 0, NS);
    for(size_t h= 0; h < hs; ++h) std::memcpy(X + ((h << K) + j) * BS, c + h * BS, BS * 4);
   }
   std::free(c);
  }
 }
 constexpr int BSF= N * 8;
 u32* F= huge_alloc((size_t(1) << (N > 5 ? N - 5 : 0)) * BSF * 4);
 u32* cF= (u32*)std::aligned_alloc(32, (size_t(1) << (N - 5 > KC ? N - 5 - KC : 0)) * BSF * 4);
 for(int i= N - 1; i >= 4; --i) {
  const int nb= i - 4, K= std::min(nb, KC), H= nb - K, ns= i + 1, J= N - i - 1;
  const size_t cnt= size_t(1) << nb, csz= size_t(1) << K;
  const u32* in= gs + (size_t(1) << i);
  for(size_t q0= 0; q0 < cnt; q0+= csz) {
   for(size_t q= q0; q < q0 + csz; ++q) init_block(F + q * BSF, in + 16 * q, __builtin_popcountll(q), 0, ns);
   for(int k= 0; k < K; ++k)
    for(size_t q= q0; q < q0 + csz; ++q)
     if(!(q >> k & 1)) fwd_pair(F + q * BSF, F + (q | size_t(1) << k) * BSF, __builtin_popcountll(q >> (k + 1)), k, 0, ns);
  }
  // 段 i+1 の双対は j = 0..J (J + 1 本)、段 i の双対は j = 0..J+1。
  u32 *zl[N - 3], *zh[N - 3], *yy[N - 3];
  auto step= [&](size_t q, const u32* Fb) {
   for(int j= 0; j <= J; ++j) zl[j]= D[j] + q * bsj[j], zh[j]= D[j] + (q + cnt) * bsj[j];
   for(int j= 0; j <= J + 1; ++j) yy[j]= D[j] + q * bsj[j];
   step_block(zl, zh, yy, Fb, i, J + 1);
  };
  if(H == 0) {
   for(size_t q= 0; q < cnt; ++q) step(q, F + q * BSF);
  } else if constexpr(N - 5 > KC) {  // N - 5 <= KC なら列の段は無い (gcc がこの枝の cF の添字を誤って警告するので外す)
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
 // 段 4 の双対 (各 j に塊 1 つ、rank 0..4) を 4 bit の素朴な形に戻し、c_m と内積を取る。
 std::vector<std::vector<u32>> c= egf_powers(4, g);
 std::vector<u32> vk(N + 1, 0);
 const V M0= mpc(0), M1= mpc(1), M2= mpc(2), M3= mpc(3);
 for(int j= 0; j <= N - 4; ++j) {
  alignas(32) u32 v[5][8], gt[16];
  for(int r= 0; r <= 4; ++r) st(v[r], norm1(lane_wht(ld(D[j] + 8 * r))));
  (void)M0, (void)M1, (void)M2, (void)M3;
  for(int s= 0; s < 16; ++s) gt[s]= v[__builtin_popcount(s)][s >> 1];
  for(int m= 0; m <= 4 && j + m <= N; ++m) {
   u64 t= 0;
   for(int s= 0; s < 16; ++s) t= (t + u64(gt[s]) * c[m][s]) % P;
   vk[j + m]= u32((vk[j + m] + t) % P);
  }
 }
 const u64 sc= pw(2, u64(P - 1) * 64 - (N - 1));
 for(auto& x: vk) x= u32(x * sc % P);
 for(int j= 0; j <= N - 4; ++j) std::free(D[j]);
 std::free(F), std::free(gs), std::free(cF);
 return vk;
}

inline std::vector<u32> power_projection(int n, int M, const std::vector<u32>& a, const std::vector<u32>& w) {
 std::vector<u32> g(a);
 const u32 c0= a[0];
 g[0]= 0;
 std::vector<u32> vk;
 if(n < 5) {
  auto c= egf_powers(n, g.data());
  vk.assign(n + 1, 0);
  for(int k= 0; k <= n; ++k) {
   u64 t= 0;
   for(int s= 0; s < (1 << n); ++s) t= (t + u64(w[s]) * c[k][s]) % P;
   vk[k]= u32(t);
  }
 } else {
  switch(n) {
#define XP_CASE(m) \
 case m: vk= vk_n<m>(g.data(), w.data()); break;
   XP_CASE(5) XP_CASE(6) XP_CASE(7) XP_CASE(8) XP_CASE(9) XP_CASE(10) XP_CASE(11) XP_CASE(12) XP_CASE(13) XP_CASE(14) XP_CASE(15) XP_CASE(16) XP_CASE(17) XP_CASE(18) XP_CASE(19) XP_CASE(20)
#undef XP_CASE
  }
 }
 std::vector<u32> ans(M, 0);
 if(c0 == 0) {
  u64 f= 1;
  for(int m= 0; m < M && m <= n; ++m) {
   if(m) f= f * m % P;
   ans[m]= u32(f * vk[m] % P);
  }
  return ans;
 }
 // ans_m = c0^m Σ_k U_k (m)_k、U_k = V_k c0^{-k}。(m)_k の Horner (h = U_k + (m - k) h、k = n..0) を m について 8 個ずつ。
 // h、U、(m - k) は R 倍で持ち、最後に c0^m (素の値) を Montgomery で掛けて R を外す。
 const u64 ic= pw(c0, P - 2);
 std::vector<u32> U(n + 1), kR(n + 1);
 {
  u64 x= 1;
  for(int k= 0; k <= n; ++k) U[k]= u32(u64(vk[k]) * x % P * R1 % P), x= x * ic % P, kR[k]= u32(u64(k) * R1 % P);
 }
 u64 cp= 1;  // c0^m
 for(int m0= 0; m0 < M; m0+= 8) {
  alignas(32) u32 mm[8], cpw[8];
  for(int t= 0; t < 8; ++t) mm[t]= u32(u64(m0 + t) * R1 % P), cpw[t]= u32(cp), cp= cp * c0 % P;
  const V mR= ld(mm);
  V h= _mm256_setzero_si256();
  for(int k= n; k >= 0; --k) h= add2(red2(mont(norm1(h), norm1(sub2(mR, cst(kR[k]))))), cst(U[k]));
  alignas(32) u32 res[8];
  st(res, norm1(mont(norm1(h), ld(cpw))));
  for(int t= 0; t < 8 && m0 + t < M; ++t) ans[m0 + t]= res[t];
 }
 return ans;
}
}  // namespace xp1

inline std::vector<xp1::u32> run(int N, int M, const std::vector<xp1::u32>& a, const std::vector<xp1::u32>& w) { return xp1::power_projection(N, M, a, w); }
