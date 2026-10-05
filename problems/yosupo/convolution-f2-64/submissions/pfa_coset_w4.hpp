#pragma once
// pfa_coset_w2 と同じ次元で、DFT を x64 で 4 本ずつ (VPCLMULQDQ があれば 256 bit の clmul)、arm は 2 本ずつ取る版。
// F_{2^64}^* の位数は 2^64 - 1 = 3 5 17 257 641 65537 6700417 なので、1 の冪根で DFT が取れる長さは 3, 5, 17 の積 (最大 255) と、
// 大きい素因数を含むものだけになる。257 以上の素数の長さの DFT は重いので、3, 5, 17 の積の長さの次元を 3 つ並べて 2^20 に届かせる。
// 各次元は中国剰余で添字を並べ替える prime-factor algorithm で、twiddle が無い。内側の次元は線形 (2B - 1 <= L)、一番外は巡回で、
// c mod (x^M - 1) (M = ∏B_内 · L_外 >= n + m - 1) を求める。DFT_5 は F_4 を経て分け (積 6 回)、DFT_17 は Rader で長さ 16 の巡回畳み込みに
// 直して Karatsuba 2 段で掛ける。DFT は W 本の線を lane に並べて SIMD で同時に取る。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#endif
#include <algorithm>
#include <array>
#include <cstring>
#include <vector>
namespace pfa {
using u64= unsigned long long;
using u8= unsigned char;
typedef u64 V2 __attribute__((vector_size(16)));
typedef u64 V4 __attribute__((vector_size(32)));
// 読み書きは memcpy で (x64 で 32 byte 境界を仮定させないため)。
template <class T> inline T ld(const u64* p) {
 T v;
 std::memcpy(&v, p, sizeof(T));
 return v;
}
template <class T> inline void st(u64* p, const T& v) { std::memcpy(p, &v, sizeof(T)); }
inline __m128i clm(u64 a, u64 b) { return _mm_clmulepi64_si128(_mm_cvtsi64_si128((long long)a), _mm_cvtsi64_si128((long long)b), 0); }
// 128 bit の積を x^64 + x^4 + x^3 + x + 1 で畳む (Library の GF2p64 の mul と同じ)。
inline u64 red(const __m128i& v) {
 static constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 const u64 h= v[1], d= h ^ (h << 1);
 return u64(v[0]) ^ RED[h >> 60] ^ d ^ (d << 3);
}
// W 本の値を並べた vector (lane-sliced)。Acc は畳む前の積で、e が偶数番目の lane の 128 bit、o が奇数番目の lane の 128 bit。
struct P2 {
 static constexpr int W= 2;
 using V= __m128i;
 struct Acc {
  V e, o;
 };
 static V ldv(const u64* p) { return _mm_loadu_si128((const __m128i*)p); }
 static void stv(u64* p, const V& v) { _mm_storeu_si128((__m128i*)p, v); }
 static V x(const V& a, const V& b) { return _mm_xor_si128(a, b); }
 static V bc(u64 c) { return _mm_set1_epi64x((long long)c); }
 static Acc mul(const V& a, const V& b) { return {_mm_clmulepi64_si128(a, b, 0x00), _mm_clmulepi64_si128(a, b, 0x11)}; }
 static V fin(const Acc& c) {
  const __m128i R= _mm_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
  const __m128i lo= _mm_unpacklo_epi64(c.e, c.o), h= _mm_unpackhi_epi64(c.e, c.o), d= _mm_xor_si128(h, _mm_slli_epi64(h, 1));
  return _mm_xor_si128(_mm_xor_si128(lo, _mm_shuffle_epi8(R, _mm_srli_epi64(h, 60))), _mm_xor_si128(d, _mm_slli_epi64(d, 3)));
 }
};
// 4 本。VP なら 256 bit の clmul (imm 0x00 で lane 0, 2、0x11 で lane 1, 3)、でなければ 128 bit の clmul を 2 回ずつ。
template <bool VP> struct P4 {
 static constexpr int W= 4;
 using V= __m256i;
 struct Acc {
  V e, o;
 };
 static V ldv(const u64* p) { return _mm256_loadu_si256((const __m256i*)p); }
 static void stv(u64* p, const V& v) { _mm256_storeu_si256((__m256i*)p, v); }
 static V x(const V& a, const V& b) { return _mm256_xor_si256(a, b); }
 static V bc(u64 c) { return _mm256_set1_epi64x((long long)c); }
 static Acc mul(const V& a, const V& b) {
  if constexpr(VP) return {_mm256_clmulepi64_epi128(a, b, 0x00), _mm256_clmulepi64_epi128(a, b, 0x11)};
  else {
   const __m128i al= _mm256_castsi256_si128(a), ah= _mm256_extracti128_si256(a, 1), bl= _mm256_castsi256_si128(b), bh= _mm256_extracti128_si256(b, 1);
   return {_mm256_setr_m128i(_mm_clmulepi64_si128(al, bl, 0x00), _mm_clmulepi64_si128(ah, bh, 0x00)), _mm256_setr_m128i(_mm_clmulepi64_si128(al, bl, 0x11), _mm_clmulepi64_si128(ah, bh, 0x11))};
  }
 }
 static V fin(const Acc& c) {
  const __m256i R= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
  const __m256i lo= _mm256_unpacklo_epi64(c.e, c.o), h= _mm256_unpackhi_epi64(c.e, c.o), d= _mm256_xor_si256(h, _mm256_slli_epi64(h, 1));
  return _mm256_xor_si256(_mm256_xor_si256(lo, _mm256_shuffle_epi8(R, _mm256_srli_epi64(h, 60))), _mm256_xor_si256(d, _mm256_slli_epi64(d, 3)));
 }
};
template <class P> inline typename P::Acc ax(const typename P::Acc& a, const typename P::Acc& b) { return {P::x(a.e, b.e), P::x(a.o, b.o)}; }
// W 本の行 (各 n 個の u64) を lane-sliced (out[t W + l] = e_l[t]) に並べ替える。w 本目以降は 0。from_lanes はその逆。
template <int W> inline void to_lanes(u64* const* e, int w, u64* out, int n) {
 int t= 0;
 if constexpr(W == 2) {
  if(w == 2)
   for(; t + 2 <= n; t+= 2) {
    const V2 x= ld<V2>(e[0] + t), y= ld<V2>(e[1] + t);
    st<V2>(out + 2 * t, V2{x[0], y[0]}), st<V2>(out + 2 * t + 2, V2{x[1], y[1]});
   }
 } else if constexpr(W == 4) {
  if(w == 4)
   for(; t + 4 <= n; t+= 4) {
    const V4 x0= ld<V4>(e[0] + t), x1= ld<V4>(e[1] + t), x2= ld<V4>(e[2] + t), x3= ld<V4>(e[3] + t);
    st<V4>(out + 4 * t, V4{x0[0], x1[0], x2[0], x3[0]}), st<V4>(out + 4 * t + 4, V4{x0[1], x1[1], x2[1], x3[1]});
    st<V4>(out + 4 * t + 8, V4{x0[2], x1[2], x2[2], x3[2]}), st<V4>(out + 4 * t + 12, V4{x0[3], x1[3], x2[3], x3[3]});
   }
 }
 for(; t < n; ++t)
  for(int l= 0; l < W; ++l) out[(size_t)t * W + l]= l < w ? e[l][t] : 0;
}
template <int W> inline void from_lanes(const u64* in, u64* const* e, int w, int n) {
 int t= 0;
 if constexpr(W == 2) {
  if(w == 2)
   for(; t + 2 <= n; t+= 2) {
    const V2 x= ld<V2>(in + 2 * t), y= ld<V2>(in + 2 * t + 2);
    st<V2>(e[0] + t, V2{x[0], y[0]}), st<V2>(e[1] + t, V2{x[1], y[1]});
   }
 } else if constexpr(W == 4) {
  if(w == 4)
   for(; t + 4 <= n; t+= 4) {
    const V4 x0= ld<V4>(in + 4 * t), x1= ld<V4>(in + 4 * t + 4), x2= ld<V4>(in + 4 * t + 8), x3= ld<V4>(in + 4 * t + 12);
    st<V4>(e[0] + t, V4{x0[0], x1[0], x2[0], x3[0]}), st<V4>(e[1] + t, V4{x0[1], x1[1], x2[1], x3[1]});
    st<V4>(e[2] + t, V4{x0[2], x1[2], x2[2], x3[2]}), st<V4>(e[3] + t, V4{x0[3], x1[3], x2[3], x3[3]});
   }
 }
 for(; t < n; ++t)
  for(int l= 0; l < w; ++l) e[l][t]= in[(size_t)t * W + l];
}
// 小さいときの素朴な積 (畳むのは出力ごとに 1 回)。
inline std::vector<u64> naive(const std::vector<u64>& a, const std::vector<u64>& b) {
 const int n= a.size(), m= b.size(), N= n + m - 1;
 std::vector<u64> c(N);
 for(int k= 0; k < N; ++k) {
  __m128i t= _mm_setzero_si128();
  for(int i= std::max(0, k - m + 1); i <= std::min(k, n - 1); ++i) t= _mm_xor_si128(t, clm(a[i], b[k - i]));
  c[k]= red(t);
 }
 return c;
}
inline u64 mulf(u64 a, u64 b) { return red(clm(a, b)); }
inline u64 powf(u64 a, u64 e) {
 u64 r= 1;
 for(; e; e>>= 1, a= mulf(a, a))
  if(e & 1) r= mulf(r, a);
 return r;
}
inline u64 invf(u64 a) { return powf(a, ~0ull - 1); }
// ---------------- 長さ 3, 5, 17 の DFT ----------------
// 2 はこの既約多項式で F_{2^64}^* の生成元なので、1 の原始 p 乗根は 2^((2^64-1)/p)。逆変換は根を逆数にする (1/p = 1)。
struct Consts {
 u64 w3[2], f5[2][3], k17[2][36];
 Consts() {
  const u64 r3= powf(2, ~0ull / 3), r5= powf(2, ~0ull / 5), r17= powf(2, ~0ull / 17);
  for(int dir= 0; dir < 2; ++dir) {
   w3[dir]= dir ? mulf(r3, r3) : r3;
   // DFT_5: β = r5^{±1}、ω = β + β^4 (1 の原始 3 乗根)。f5 = {ω, β, β^2}
   const u64 b= dir ? powf(r5, 4) : r5;
   f5[dir][0]= r5 ^ powf(r5, 4), f5[dir][1]= b, f5[dir][2]= mulf(b, b);
   // DFT_17: K_m = w^(3^-m) (3^-1 = 6 mod 17) を、Karatsuba の葉 9 個 × 4 の並びに展開しておく。
   const u64 w17= dir ? powf(r17, 16) : r17;
   u64 K[16];
   for(int m= 0, gi= 1; m < 16; ++m, gi= gi * 6 % 17) K[m]= powf(w17, gi);
   u64* o= k17[dir];
   for(int h= 0; h < 3; ++h) {
    u64 X[8];
    for(int i= 0; i < 8; ++i) X[i]= h == 0 ? K[i] : h == 1 ? K[8 + i] : K[i] ^ K[8 + i];
    for(int q= 0; q < 3; ++q)
     for(int i= 0; i < 4; ++i) *o++= q == 0 ? X[i] : q == 1 ? X[4 + i] : X[i] ^ X[4 + i];
   }
  }
 }
};
inline const Consts& consts() {
 static const Consts c;
 return c;
}
template <class P> struct Ker {
 using V= typename P::V;
 using Acc= typename P::Acc;
 static V mulr(const V& a, const V& b) { return P::fin(P::mul(a, b)); }
 static void school4(const V* a, const V* k, Acc* c) {
#pragma GCC unroll 8
  for(int s= 0; s < 7; ++s) {
   const int i0= s < 4 ? 0 : s - 3, i1= s < 4 ? s : 3;
   Acc t= P::mul(a[i0], k[s - i0]);
#pragma GCC unroll 4
   for(int i= i0 + 1; i <= i1; ++i) t= ax<P>(t, P::mul(a[i], k[s - i]));
   c[s]= t;
  }
 }
 // 8 x 8 の積 (15 項) を 3 つの葉 (L0, L1, L01) から
 static void comb8(const Acc* L0, const Acc* L1, const Acc* L01, Acc* R) {
#pragma GCC unroll 8
  for(int i= 0; i < 7; ++i) R[i]= L0[i], R[8 + i]= L1[i];
  R[7]= ax<P>(ax<P>(L01[3], L0[3]), L1[3]);
#pragma GCC unroll 8
  for(int i= 0; i < 7; ++i)
   if(i != 3) R[4 + i]= ax<P>(R[4 + i], ax<P>(ax<P>(L01[i], L0[i]), L1[i]));
 }
 // DFT_3: (a0, a1, a2) -> (a0 + a1 + a2, a0 + a2 + w s, a0 + a1 + w s)、s = a1 + a2
 static void dft3(u64* p0, u64* p1, u64* p2, const V& w) {
  const V a0= P::ldv(p0), a1= P::ldv(p1), a2= P::ldv(p2), s= P::x(a1, a2), t= mulr(s, w);
  P::stv(p0, P::x(a0, s)), P::stv(p1, P::x(P::x(a0, a2), t)), P::stv(p2, P::x(P::x(a0, a1), t));
 }
 // DFT_5: r = a mod Φ_5 を F_4 上の 2 つの 2 次式 x^2 + ω x + 1、x^2 + ω^2 x + 1 で割り、それぞれの根 (β, β^4)、(β^2, β^3) で値を取る。
 static void dft5(u64* b, size_t s, const V* f) {
  const V a0= P::ldv(b), a1= P::ldv(b + s), a2= P::ldv(b + 2 * s), a3= P::ldv(b + 3 * s), a4= P::ldv(b + 4 * s);
  const V r0= P::x(a0, a4), r1= P::x(a1, a4), r2= P::x(a2, a4), r3= P::x(a3, a4), r23= P::x(r2, r3);
  const V m1= mulr(r3, f[0]), m2= mulr(r23, f[0]), u0= P::x(P::x(r0, r2), m1), u1= P::x(r1, m2), v0= P::x(u0, r3), v1= P::x(u1, r23);
  const V X1= P::x(u0, mulr(u1, f[1])), X2= P::x(v0, mulr(v1, f[2]));
  P::stv(b, P::x(P::x(P::x(a0, a1), P::x(a2, a3)), a4));
  P::stv(b + s, X1), P::stv(b + 4 * s, P::x(X1, mulr(u1, f[0])));
  P::stv(b + 2 * s, X2), P::stv(b + 3 * s, P::x(P::x(X2, v1), mulr(v1, f[0])));
 }
 // DFT_17 (Rader、原始根 3): A_q = a_{3^q}、Y_r = Σ A_q K_{r-q}、X_{3^{-r}} = a_0 + Y_r。積は畳まずに足し、Y_r ごとに 1 回畳む。
 static void dft17(u64* b, size_t s, const V* K) {
  static constexpr int GP[16]= {1, 3, 9, 10, 13, 5, 15, 11, 16, 14, 8, 7, 4, 12, 2, 6};  // 3^q mod 17
  static constexpr int GN[16]= {1, 6, 2, 12, 4, 7, 8, 14, 16, 11, 15, 5, 13, 10, 9, 3};  // 3^(-r) mod 17
  V a[17];
#pragma GCC unroll 17
  for(int i= 0; i < 17; ++i) a[i]= P::ldv(b + i * s);
  V X0= a[0];
#pragma GCC unroll 17
  for(int i= 1; i < 17; ++i) X0= P::x(X0, a[i]);
  V D[36];
#pragma GCC unroll 3
  for(int h= 0; h < 3; ++h) {
   V X[8];
#pragma GCC unroll 8
   for(int i= 0; i < 8; ++i) X[i]= h == 0 ? a[GP[i]] : h == 1 ? a[GP[8 + i]] : P::x(a[GP[i]], a[GP[8 + i]]);
#pragma GCC unroll 3
   for(int q= 0; q < 3; ++q)
#pragma GCC unroll 4
    for(int i= 0; i < 4; ++i) D[h * 12 + q * 4 + i]= q == 0 ? X[i] : q == 1 ? X[4 + i] : P::x(X[i], X[4 + i]);
  }
  Acc Lf[9][7], R[3][15];
#pragma GCC unroll 9
  for(int l= 0; l < 9; ++l) school4(D + 4 * l, K + 4 * l, Lf[l]);
#pragma GCC unroll 3
  for(int h= 0; h < 3; ++h) comb8(Lf[3 * h], Lf[3 * h + 1], Lf[3 * h + 2], R[h]);
  // Y = R0 + R1 + x^8 (R01 + R0 + R1) mod x^16 - 1
  Acc Y[16];
#pragma GCC unroll 15
  for(int i= 0; i < 15; ++i) Y[i]= ax<P>(R[0][i], R[1][i]);
  Y[15]= ax<P>(ax<P>(R[2][7], R[0][7]), R[1][7]);
#pragma GCC unroll 15
  for(int i= 0; i < 15; ++i)
   if(i != 7) Y[(i + 8) & 15]= ax<P>(Y[(i + 8) & 15], ax<P>(ax<P>(R[2][i], R[0][i]), R[1][i]));
  P::stv(b, X0);
#pragma GCC unroll 16
  for(int r= 0; r < 16; ++r) P::stv(b + GN[r] * s, P::x(a[0], P::fin(Y[r])));
 }
};
// ---------------- 多次元の配置 ----------------
// 長さ L' (3, 5, 17 の積) の部分は、中国剰余で添字 n を (n mod p) の組に置き換えて並べる (位置 = Σ (n mod p) st_p)。
// こうすると長さ L' の巡回畳み込みが各 p の巡回畳み込みのテンソル積になる。普通の次元は L' = L。剰余類の次元は L = 2L'、B = L' で、
// μ_{L'} と θ μ_{L'} (θ = 2) の 2 つで値を取る。θ 側は係数に θ^n を掛けてから長さ L' の DFT を取り、逆では θ^{-n} を掛け戻して
// mod (x^{L'} - 1) と mod (x^{L'} - θ^{L'}) から c = u + x^{L'} v を組む (v = (r0 + r1) / (1 + θ^{L'})、u = r0 + v)。
struct Dim {
 int L, Lp, B, np, pr[3], st[3];
 bool coset;
 std::vector<int> pos, inv;
 std::vector<u64> tw, itw;
 u64 kappa;
 Dim(int L_, int B_, bool c): L(L_), Lp(c ? L_ / 2 : L_), B(B_), np(0), coset(c), kappa(0) {
  for(int p: {3, 5, 17})
   if(Lp % p == 0) pr[np++]= p;
  for(int i= np - 1, s= 1; i >= 0; --i) st[i]= s, s*= pr[i];
  pos.resize(Lp), inv.resize(Lp);
  for(int n= 0; n < Lp; ++n) {
   int q= 0;
   for(int i= 0; i < np; ++i) q+= n % pr[i] * st[i];
   pos[n]= q, inv[q]= n;
  }
  if(coset) {
   tw.resize(Lp), itw.resize(Lp);
   const u64 th= 2, ith= invf(th);
   for(int n= 0; n < Lp; ++n) tw[n]= n ? mulf(tw[n - 1], th) : 1, itw[n]= n ? mulf(itw[n - 1], ith) : 1;
   kappa= invf(1 ^ powf(th, Lp));
  }
 }
};
// 配列は次元 0 が連続 (W の倍数に切り上げた pitch)、その上に次元 1, 2, ... の行。次元 0 の DFT は W 行ずつ lane-sliced に並べ替えてから、
// 次元 1 以上の DFT は次元 0 の方向に W 本ずつ取る。
template <class P> struct Conv {
 using V= typename P::V;
 static constexpr int W= P::W;
 std::vector<Dim> dims;
 int pitch;
 size_t rows, total;
 V kw3[2], f5[2][3], k17[2][36];
 Conv(const std::vector<std::array<int, 3>>& ds) {
  for(auto [L, B, c]: ds) dims.emplace_back(L, B, c != 0);
  pitch= (dims[0].L + W - 1) / W * W, rows= 1;
  for(size_t j= 1; j < dims.size(); ++j) rows*= dims[j].L;
  total= (rows + W) * pitch;
  const Consts& c= consts();
  for(int d= 0; d < 2; ++d) {
   kw3[d]= P::bc(c.w3[d]);
   for(int i= 0; i < 3; ++i) f5[d][i]= P::bc(c.f5[d][i]);
   for(int i= 0; i < 36; ++i) k17[d][i]= P::bc(c.k17[d][i]);
  }
 }
 void line(int p, u64* base, size_t s, int dir) {
  if(p == 3) Ker<P>::dft3(base, base + s, base + 2 * s, kw3[dir]);
  else if(p == 5) Ker<P>::dft5(base, s, f5[dir]);
  else Ker<P>::dft17(base, s, k17[dir]);
 }
 // 次元 d の全部の線に DFT。base(q) は位置 q の先頭、G はその次元の位置 1 つあたりの u64 数、lanes は 1 つの位置にある u64 数。
 template <class F> void dim_dft(const Dim& d, F base, size_t G, size_t lanes, int dir) {
  for(int i= 0; i < d.np; ++i) {
   const int p= d.pr[i], s= d.st[i];
   for(int c= 0; c < (d.coset ? 2 : 1); ++c)
    for(int q= 0; q < d.Lp; ++q)
     if(q / s % p == 0) {
      u64* bq= base(c * d.Lp + q);
      for(size_t lo= 0; lo < lanes; lo+= W) line(p, bq + lo, (size_t)s * G, dir);
     }
  }
 }
 // 剰余類の次元の逆変換の仕上げ (θ^{-n} を掛け戻して u, v を組む)。
 template <class F> void coset_fix(const Dim& d, F base, size_t lanes) {
  const V ka= P::bc(d.kappa);
  for(int q= 0; q < d.Lp; ++q) {
   u64 *b0= base(q), *b1= base(d.Lp + q);
   const V it= P::bc(d.itw[d.inv[q]]);
   for(size_t lo= 0; lo < lanes; lo+= W) {
    const V r0= P::ldv(b0 + lo), r1= P::fin(P::mul(P::ldv(b1 + lo), it)), v= P::fin(P::mul(P::x(r0, r1), ka));
    P::stv(b0 + lo, P::x(r0, v)), P::stv(b1 + lo, v);
   }
  }
 }
 void transform(u64* A, int dir, u64* lb) {
  const Dim& d0= dims[0];
  for(size_t r= 0; r < rows; r+= W) {
   u64* e[W];
   for(int l= 0; l < W; ++l) e[l]= A + (r + l) * pitch;
   to_lanes<W>(e, W, lb, d0.L);
   dim_dft(d0, [&](int q) { return lb + (size_t)q * W; }, W, W, dir);
   if(dir && d0.coset) coset_fix(d0, [&](int q) { return lb + (size_t)q * W; }, W);
   from_lanes<W>(lb, e, W, d0.L);
  }
  size_t G= pitch;
  for(size_t j= 1; j < dims.size(); ++j) {
   const Dim& dj= dims[j];
   const size_t hiN= rows / (G / pitch) / dj.L;
   for(size_t hi= 0; hi < hiN; ++hi) {
    u64* blk= A + hi * dj.L * G;
    dim_dft(dj, [&](int q) { return blk + (size_t)q * G; }, G, G, dir);
    if(dir && dj.coset) coset_fix(dj, [&](int q) { return blk + (size_t)q * G; }, G);
   }
   G*= dj.L;
  }
 }
 // 入力の係数を置く。剰余類の次元では、θ 側に θ^n を掛けた写しも置く。
 void place(const std::vector<u64>& src, u64* dst) {
  const size_t D= dims.size();
  for(size_t i= 0; i < src.size(); ++i) {
   size_t x= i, offs[8], mul= 1;
   int dg[4], cnt= 1;
   for(size_t j= 0; j < D; ++j) dg[j]= x % dims[j].B, x/= dims[j].B;
   u64 vals[8];
   offs[0]= 0, vals[0]= src[i];
   for(size_t j= 0; j < D; ++j) {
    const Dim& d= dims[j];
    const size_t q= d.pos[dg[j]];
    for(int t= 0; t < cnt; ++t) offs[t]+= q * mul;
    if(d.coset) {
     for(int t= 0; t < cnt; ++t) offs[cnt + t]= offs[t] + (size_t)d.Lp * mul, vals[cnt + t]= mulf(vals[t], d.tw[dg[j]]);
     cnt*= 2;
    }
    mul*= j ? d.L : pitch;
   }
   for(int t= 0; t < cnt; ++t) dst[offs[t]]= vals[t];
  }
 }
 std::vector<u64> run(const std::vector<u64>& a, const std::vector<u64>& b, int N) {
  std::vector<u64> A(total), Bv(total), lb((size_t)pitch * W);
  place(a, A.data()), place(b, Bv.data());
  transform(A.data(), 0, lb.data()), transform(Bv.data(), 0, lb.data());
  for(size_t i= 0; i < total; i+= W) P::stv(&A[i], P::fin(P::mul(P::ldv(&A[i]), P::ldv(&Bv[i]))));
  transform(A.data(), 1, lb.data());
  size_t M= 1;
  for(size_t j= 0; j + 1 < dims.size(); ++j) M*= dims[j].B;
  M*= dims.back().L;
  std::vector<u64> c(M);
  // 位置 -> 各次元の添字 n_j (剰余類の次元は θ 側を n + L') -> 出力の添字 n_0 + B_0 (n_1 + B_1 (...)) mod M
  auto nat= [](const Dim& d, int q) { return q < d.Lp ? d.inv[q] : d.inv[q - d.Lp] + d.Lp; };
  std::vector<size_t> n0(dims[0].L);
  for(int t= 0; t < dims[0].L; ++t) n0[t]= nat(dims[0], t);
  for(size_t r= 0; r < rows; ++r) {
   size_t x= r, base= 0, mul= dims[0].B;
   for(size_t j= 1; j < dims.size(); ++j) {
    const int q= x % dims[j].L;
    x/= dims[j].L;
    base+= (size_t)nat(dims[j], q) * mul;
    mul*= dims[j].B;
   }
   const u64* row= &A[r * pitch];
   for(int t= 0; t < dims[0].L; ++t) c[(base + n0[t]) % M]^= row[t];
  }
  c.resize(N);
  return c;
 }
};
// 次元を選ぶ: 内側を 0 から 2 個 (普通か剰余類 2 つ) と、外側 (巡回) を 1 個。点の数に、次元ごとの手間の見積もりを掛けて最小に。
// 2^20 では、普通の次元だけなら 85 x 255 x 255 (5.5M 点)、剰余類の次元を使うと 34 x 510 x 255 (4.4M 点)。
inline std::vector<std::array<int, 3>> choose(int N, bool coset) {
 static constexpr int D[]= {3, 5, 15, 17, 51, 85, 255};
 auto cost= [](int L) { return (L % 3 ? 0 : 0.4) + (L % 5 ? 0 : 1.3) + (L % 17 ? 0 : 6.0); };
 struct Opt {
  int L, B, c;
  double cpp;
 };
 std::vector<Opt> inner;
 for(int L: D) {
  inner.push_back({L, (L + 1) / 2, 0, cost(L)});
  if(coset) inner.push_back({2 * L, L, 1, cost(L) + 0.8});
 }
 double best= 1e300;
 std::vector<std::array<int, 3>> res;
 const int ni= inner.size();
 for(int k= 0; k <= 2; ++k)
  for(int x= 0; x < (k >= 1 ? ni : 1); ++x)
   for(int y= 0; y < (k >= 2 ? ni : 1); ++y) {
    double Bp= 1, pts= 1, cs= 0;
    std::vector<Opt> in;
    if(k >= 1) in.push_back(inner[x]);
    if(k >= 2) in.push_back(inner[y]);
    for(auto& o: in) Bp*= o.B, pts*= o.L, cs+= o.cpp;
    for(int Lo: D)
     if(Bp * Lo >= N) {
      const double c= pts * Lo * (cs + cost(Lo));
      if(c < best) {
       best= c, res.clear();
       for(auto& o: in) res.push_back({o.L, o.B, o.c});
       res.push_back({Lo, Lo, 0});
      }
      break;
     }
   }
 return res;
}
template <class P> inline std::vector<u64> convolve(const std::vector<u64>& a, const std::vector<u64>& b) {
 const int n= a.size(), m= b.size();
 if(!n || !m) return {};
 const int N= n + m - 1;
 if(N <= 32) return naive(a, b);
 return Conv<P>(choose(N, true)).run(a, b, N);
}
}  // namespace pfa
// x64 は 4 本 (VPCLMULQDQ があれば 256 bit の clmul)。arm は SIMDe の 256 bit の clmul が PMULL にならないので 2 本。
inline std::vector<u64> run(int, int, const std::vector<u64>& a, const std::vector<u64>& b) {
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return pfa::convolve<pfa::P4<1>>(a, b);
 return pfa::convolve<pfa::P4<0>>(a, b);
#else
 return pfa::convolve<pfa::P2>(a, b);
#endif
}
