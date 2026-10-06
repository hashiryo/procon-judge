#pragma once
// schoenhage3_r27_w4_k1f に、schoenhage3_r9_w8_k1 と同じ AVX-512 の道 (R_27 の積を 8 本ずつ、変換の XOR を 512 bit で) を足した版。
// 持たない CPU では schoenhage3_r27_w4_k1f と同じ道を通る。
// R_d = F[x]/(x^{2d} + x^d + 1) (d = 3^k、F = F_{2^64}) では x^{3d} = 1 で、x^d が 1 の原始 3 乗根になる。R_D の積を、d 係数ずつ 2δ 個に
// 割って (D = d δ、δ | d) R_d 係数の y = x^d の多項式とみなし、y^{2δ} + y^δ + 1 の根 (x^{d/δ} の冪) で値を取って各点を R_d の中で掛ける。
// twiddle は x の冪を掛けることで、係数をずらして重ねる XOR だけで済む (Schönhage 1977)。一番上を R_729 (D = 3^12、2D = 1062882 >= 2^20)、
// その下を R_27 にして、R_27 の積 (78732 回) は Karatsuba で掛ける。R_27 の積は W 本を lane に並べて SIMD で同時に掛ける。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#include <simde/x86/clmul.h>
#endif
#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>
namespace schoenhage3 {
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
// ---------------- base: R_27 の積を W 本まとめて ----------------
// 出力ごとに足し込む schoolbook。c[0, 2N-1)
template <class P, int N> inline void school(const typename P::V* a, const typename P::V* b, typename P::Acc* c) {
#pragma GCC unroll 16
 for(int k= 0; k < 2 * N - 1; ++k) {
  typename P::Acc s= P::mul(a[k < N ? 0 : k - N + 1], b[k < N ? k : N - 1]);
#pragma GCC unroll 16
  for(int i= (k < N ? 1 : k - N + 2); i <= (k < N ? k : N - 1); ++i) s= ax<P>(s, P::mul(a[i], b[k - i]));
  c[k]= s;
 }
}
// c[0, 2N-1) = a b (Karatsuba)。1 まで割り切る (27 → 14, 13 → 7, 6 → 4, 3 → 2, 1 → 1)。R_27 の積 1 つで葉の積が 663 回。
template <class P, int N> inline void kara(const typename P::V* a, const typename P::V* b, typename P::Acc* c) {
 using V= typename P::V;
 using Acc= typename P::Acc;
 if constexpr(N <= 1) school<P, N>(a, b, c);
 else {
  constexpr int H= (N + 1) / 2, L= N - H;
  V sa[H], sb[H];
  for(int i= 0; i < L; ++i) sa[i]= P::x(a[i], a[H + i]), sb[i]= P::x(b[i], b[H + i]);
  if constexpr(H > L) sa[H - 1]= a[H - 1], sb[H - 1]= b[H - 1];
  Acc m[2 * H - 1];
  kara<P, H>(a, b, c);
  kara<P, L>(a + H, b + H, c + 2 * H);
  kara<P, H>(sa, sb, m);
  // m ^= c[0, 2H-1) ^ c[2H, 2N-1); c[H, 3H-1) ^= m。c[2H-1] は 0。
  for(int i= 0; i < 2 * L - 1; ++i) m[i]= ax<P>(ax<P>(m[i], c[i]), c[2 * H + i]);
  for(int i= 2 * L - 1; i < 2 * H - 1; ++i) m[i]= ax<P>(m[i], c[i]);
  c[2 * H - 1]= m[H - 1];
  for(int i= 0; i < H - 1; ++i) c[H + i]= ax<P>(c[H + i], m[i]);
  for(int i= H; i < 2 * H - 1; ++i) c[H + i]= ax<P>(c[H + i], m[i]);
 }
}
// R_27 = F[x]/(x^54 + x^27 + 1) の積。x^27 で 2 つに割る Karatsuba の足し戻しを、x^54 = x^27 + 1 の畳みと合わせる。
template <class P> inline void base27(const u64* a, const u64* b, u64* c) {
 using V= typename P::V;
 using Acc= typename P::Acc;
 constexpr int W= P::W;
 V aL[27], aH[27], bL[27], bH[27], sa[27], sb[27];
 for(int i= 0; i < 27; ++i) aL[i]= P::ldv(a + i * W), aH[i]= P::ldv(a + (27 + i) * W), bL[i]= P::ldv(b + i * W), bH[i]= P::ldv(b + (27 + i) * W), sa[i]= P::x(aL[i], aH[i]), sb[i]= P::x(bL[i], bH[i]);
 Acc p0[53], p2[53], p1[53];
 kara<P, 27>(aL, bL, p0), kara<P, 27>(aH, bH, p2), kara<P, 27>(sa, sb, p1);
 // S = p0 + p2, T = p1 + p0。c[i] = S[i] + T[i+27] (i < 26)、c[26] = S[26]、c[i] = S[i] + T[i-27] + T[i] (27 <= i <= 52)、c[53] = T[26]
 for(int i= 0; i < 26; ++i) P::stv(c + i * W, P::fin(ax<P>(ax<P>(p0[i], p2[i]), ax<P>(p1[i + 27], p0[i + 27]))));
 P::stv(c + 26 * W, P::fin(ax<P>(p0[26], p2[26])));
 for(int i= 27; i < 53; ++i) P::stv(c + i * W, P::fin(ax<P>(ax<P>(ax<P>(p0[i], p2[i]), ax<P>(p1[i - 27], p0[i - 27])), ax<P>(p1[i], p0[i]))));
 P::stv(c + 53 * W, P::fin(ax<P>(p1[26], p0[26])));
}
// ---------------- R_d 上の FFT ----------------
// R_d の要素は 2d 個の係数で、係数 1 つが W 個の u64 (lane)。dw = d W が下半分 L と上半分 H の長さ。VT は XOR に使う SIMD の幅。
#ifdef __x86_64__
using VT= V4;
#else
using VT= V2;
#endif
inline int pow3(int k) {
 int r= 1;
 while(k--) r*= 3;
 return r;
}
// ずらした要素 x^e A の位置 j (下半分) と dw + j (上半分) の値を、A からの 2 つの読み出し u, v で作る。
// ty 0: (L, H) = (u, v)、ty 1: (L, T) = (u, v)、ty 2: (H, T) = (u, v)。T = L ^ H。x^{2d} = x^d + 1 から、どの区間もこの 3 通りに収まる。
template <int TY, class T> inline void tri(const T& u, const T& v, T& L, T& H, T& X) {
 if constexpr(TY == 0) L= u, H= v, X= u ^ v;
 else if constexpr(TY == 1) L= u, X= v, H= u ^ v;
 else H= u, X= v, L= u ^ v;
}
struct Seg {
 int j0, j1, xo, yo, ty;
};
// x^e A (0 <= e < 3d) の読み出し方を j の区間ごとに (u = A[j + xo]、v = A[j + yo])。e = q d + r、r は u64 単位で r W。
inline int segs(int d, int W, int e, Seg* s) {
 const int dw= d * W, q= e / d, r= (e % d) * W;
 int n= 0;
 if(q == 0) {
  if(r) s[n++]= {0, r, 2 * dw - r, dw - r, 1};
  s[n++]= {r, dw, -r, dw - r, 0};
 } else if(q == 1) {
  if(r) s[n++]= {0, r, dw - r, 2 * dw - r, 2};
  s[n++]= {r, dw, dw - r, -r, 1};
 } else {
  if(r) s[n++]= {0, r, dw - r, 2 * dw - r, 0};
  s[n++]= {r, dw, -r, dw - r, 2};
 }
 return n;
}
// 順変換の butterfly の 1 区間。O_t = A0 + w^t (x^{e1} A1) + w^{2t} (x^{e2} A2) (w = x^d、x^d (L, H) = (H, L + H))。
// 書き込みが何とでも重なりうる (memcpy) ので、ループの中で参照を読み直さないよう、読み書きの位置は先にポインタにしておく。
template <class T, int TB, int TC> inline int fwd_run(const u64* aLp, const u64* aHp, const u64* bu, const u64* bv, const u64* cu, const u64* cv, u64* o0L, u64* o0H, u64* o1L, u64* o1H, u64* o2L, u64* o2H, int j, int j1) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= j1; j+= L) {
  const T aL= ld<T>(aLp + j), aH= ld<T>(aHp + j);
  T bL, bH, t1, cL, cH, t2;
  tri<TB>(ld<T>(bu + j), ld<T>(bv + j), bL, bH, t1);
  tri<TC>(ld<T>(cu + j), ld<T>(cv + j), cL, cH, t2);
  st<T>(o0L + j, aL ^ bL ^ cL), st<T>(o0H + j, aH ^ bH ^ cH);
  st<T>(o1L + j, aL ^ bH ^ t2), st<T>(o1H + j, aH ^ t1 ^ cL);
  st<T>(o2L + j, aL ^ t1 ^ cH), st<T>(o2H + j, aH ^ bL ^ t2);
 }
 return j;
}
template <int TB, int TC> inline void fwd_seg(const u64* A0, const u64* A1, const u64* A2, u64* O0, u64* O1, u64* O2, int j0, int j1, int dw, const Seg& sb, const Seg& sc) {
 const u64 *aH= A0 + dw, *bu= A1 + sb.xo, *bv= A1 + sb.yo, *cu= A2 + sc.xo, *cv= A2 + sc.yo;
 u64 *o0H= O0 + dw, *o1H= O1 + dw, *o2H= O2 + dw;
 const int j= fwd_run<VT, TB, TC>(A0, aH, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j0, j1);
 fwd_run<u64, TB, TC>(A0, aH, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j, j1);
}
using FwdFn= void (*)(const u64*, const u64*, const u64*, u64*, u64*, u64*, int, int, int, const Seg&, const Seg&);
constexpr FwdFn FWD[3][3]= {{fwd_seg<0, 0>, fwd_seg<0, 1>, fwd_seg<0, 2>}, {fwd_seg<1, 0>, fwd_seg<1, 1>, fwd_seg<1, 2>}, {fwd_seg<2, 0>, fwd_seg<2, 1>, fwd_seg<2, 2>}};
// 順変換の butterfly 1 つ。A mod (y^cnt - x^E) の 3 つ組 (A0, A1, A2) から、y^{cnt/3} = x^{E/3 + d t} での値 O_t (t = 0, 1, 2) を作る。
// ずらしは読み出しの添字に吸収する (書き込みは src と別の場所)。
struct Shift2 {
 Seg sb[2], sc[2];
 int nb, nc;
 Shift2(int d, int W, int E): nb(segs(d, W, E / 3, sb)), nc(segs(d, W, 2 * E / 3, sc)) {}
};
inline void fwd_bfly(const u64* A0, const u64* A1, const u64* A2, u64* O0, u64* O1, u64* O2, int dw, const Shift2& s) {
 for(int x= 0; x < s.nb; ++x)
  for(int y= 0; y < s.nc; ++y) {
   const int j0= std::max(s.sb[x].j0, s.sc[y].j0), j1= std::min(s.sb[x].j1, s.sc[y].j1);
   if(j0 < j1) FWD[s.sb[x].ty][s.sc[y].ty](A0, A1, A2, O0, O1, O2, j0, j1, dw, s.sb[x], s.sc[y]);
  }
}
// 1 段を src の block (cnt 要素) から dst へ。
inline void fwd3(const u64* src, u64* dst, int d, int W, int cnt, int E) {
 const size_t m= cnt / 3, dw= (size_t)d * W, sz= 2 * dw;
 const Shift2 s(d, W, E);
 for(size_t i= 0; i < m; ++i) fwd_bfly(src + i * sz, src + (m + i) * sz, src + (2 * m + i) * sz, dst + i * sz, dst + (m + i) * sz, dst + (2 * m + i) * sz, dw, s);
}
template <class T, int TY> inline int shift_run(const u64* pu, const u64* pv, u64* oL, u64* oH, int j, int j1) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= j1; j+= L) {
  const T u= ld<T>(pu + j), v= ld<T>(pv + j);
  if constexpr(TY == 0) st<T>(oL + j, u), st<T>(oH + j, v);
  else if constexpr(TY == 1) st<T>(oL + j, u), st<T>(oH + j, u ^ v);
  else st<T>(oL + j, u ^ v), st<T>(oH + j, u);
 }
 return j;
}
template <int TY> inline void shift_seg(const u64* A, u64* out, int dw, const Seg& s) {
 const u64 *pu= A + s.xo, *pv= A + s.yo;
 u64* oH= out + dw;
 shift_run<u64, TY>(pu, pv, out, oH, shift_run<VT, TY>(pu, pv, out, oH, s.j0, s.j1), s.j1);
}
// out = x^e A
inline void shift(const u64* A, u64* out, int d, int W, int e) {
 Seg s[2];
 const int n= segs(d, W, e, s), dw= d * W;
 for(int x= 0; x < n; ++x) {
  if(s[x].ty == 0) shift_seg<0>(A, out, dw, s[x]);
  else if(s[x].ty == 1) shift_seg<1>(A, out, dw, s[x]);
  else shift_seg<2>(A, out, dw, s[x]);
 }
}
template <class T> inline int inv_run(const u64* O0, const u64* O0H, const u64* O1, const u64* O1H, const u64* O2, const u64* O2H, u64* A0, u64* A0H, u64* tB, u64* tBH, u64* tC, u64* tCH, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T aL= ld<T>(O0 + j), aH= ld<T>(O0H + j), bL= ld<T>(O1 + j), bH= ld<T>(O1H + j), cL= ld<T>(O2 + j), cH= ld<T>(O2H + j), s1= bL ^ bH, s2= cL ^ cH;
  st<T>(A0 + j, aL ^ bL ^ cL), st<T>(A0H + j, aH ^ bH ^ cH);
  st<T>(tB + j, aL ^ s1 ^ cH), st<T>(tBH + j, aH ^ bL ^ s2);
  st<T>(tC + j, aL ^ bH ^ s2), st<T>(tCH + j, aH ^ s1 ^ cL);
 }
 return j;
}
// 逆変換の butterfly 1 つ。O_t から A0 = O0 + O1 + O2 と、ずらす前の B = O0 + w^2 O1 + w O2、C = O0 + w O1 + w^2 O2 を作り (1/3 = 1)、
// B, C を x^{-E/3}, x^{-2E/3} でずらして A1, A2 に置く。A0 は O0 と同じ場所でもよい。
inline void inv_bfly(const u64* O0, const u64* O1, const u64* O2, u64* A0, u64* A1, u64* A2, int d, int W, int E, u64* tmp) {
 const int dw= d * W, e1= E / 3, e2= 2 * E / 3;
 u64 *tB= tmp, *tC= tmp + 2 * dw;
 const u64 *O0H= O0 + dw, *O1H= O1 + dw, *O2H= O2 + dw;
 u64 *A0H= A0 + dw, *tBH= tB + dw, *tCH= tC + dw;
 inv_run<u64>(O0, O0H, O1, O1H, O2, O2H, A0, A0H, tB, tBH, tC, tCH, inv_run<VT>(O0, O0H, O1, O1H, O2, O2H, A0, A0H, tB, tBH, tC, tCH, 0, dw), dw);
 shift(tB, A1, d, W, e1 ? 3 * d - e1 : 0), shift(tC, A2, d, W, e2 ? 3 * d - e2 : 0);
}
inline void inv3(u64* blk, int d, int W, int cnt, int E, u64* tmp) {
 const size_t m= cnt / 3, sz= (size_t)2 * d * W;
 for(size_t i= 0; i < m; ++i) {
  u64 *A0= blk + i * sz, *A1= A0 + m * sz, *A2= A1 + m * sz;
  inv_bfly(A0, A1, A2, A0, A1, A2, d, W, E, tmp);
 }
}
// 2 段を 1 回の読み書きで src の block から dst へ。間の 9 要素は T に置く。
inline void fwd9(const u64* src, u64* dst, int d, int W, int cnt, int E, u64* T) {
 const size_t m= cnt / 9, dw= (size_t)d * W, sz= 2 * dw;
 const Shift2 s1(d, W, E), s2[3]= {Shift2(d, W, E / 3), Shift2(d, W, E / 3 + d), Shift2(d, W, E / 3 + 2 * d)};
 for(size_t i= 0; i < m; ++i) {
  for(size_t j= 0; j < 3; ++j) fwd_bfly(src + (i + j * m) * sz, src + (i + j * m + 3 * m) * sz, src + (i + j * m + 6 * m) * sz, T + j * sz, T + (3 + j) * sz, T + (6 + j) * sz, dw, s1);
  for(size_t t= 0; t < 3; ++t) fwd_bfly(T + 3 * t * sz, T + (3 * t + 1) * sz, T + (3 * t + 2) * sz, dst + (3 * t * m + i) * sz, dst + (3 * t * m + m + i) * sz, dst + (3 * t * m + 2 * m + i) * sz, dw, s2[t]);
 }
}
// fwd9 の逆をその場で。9 要素を T に読んでから書き戻す。
inline void inv9(u64* blk, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 9, sz= (size_t)2 * d * W;
 for(size_t i= 0; i < m; ++i) {
  for(size_t t= 0; t < 3; ++t) inv_bfly(blk + (3 * t * m + i) * sz, blk + (3 * t * m + m + i) * sz, blk + (3 * t * m + 2 * m + i) * sz, T + 3 * t * sz, T + (3 * t + 1) * sz, T + (3 * t + 2) * sz, d, W, E / 3 + d * (int)t, tmp);
  for(size_t j= 0; j < 3; ++j) inv_bfly(T + j * sz, T + (3 + j) * sz, T + (6 + j) * sz, blk + (i + j * m) * sz, blk + (i + j * m + 3 * m) * sz, blk + (i + j * m + 6 * m) * sz, d, W, E, tmp);
 }
}
// 3 段を 1 回の読み書きで src の block から dst へ (27 要素を T に置く)。1 段目は src から T へ、2 段目は T の中でその場で
// (ずらした 2 つを tmp に写してから合わせる)、3 段目は T から dst へ。unit i の入力は src[i + m (j3 + 3 j2 + 9 j1)]、
// T の並びは (t1, j2, j3) → (t1, t2, j3)、出力は dst[(9 t1 + 3 t2 + t3) m + i]。
inline void fwd_bfly_ip(u64* A0, u64* A1, u64* A2, int d, int W, int E, u64* tmp) {
 const int dw= d * W;
 u64 *tB= tmp, *tC= tmp + 2 * dw;
 shift(A1, tB, d, W, E / 3), shift(A2, tC, d, W, 2 * E / 3);
 fwd_run<u64, 0, 0>(A0, A0 + dw, tB, tB + dw, tC, tC + dw, A0, A0 + dw, A1, A1 + dw, A2, A2 + dw, fwd_run<VT, 0, 0>(A0, A0 + dw, tB, tB + dw, tC, tC + dw, A0, A0 + dw, A1, A1 + dw, A2, A2 + dw, 0, dw), dw);
}
// 2 段目と 3 段目の twiddle。E2[t1] = E/3 + d t1 (2 段目)、s3[3 t1 + t2] は E2[t1]/3 + d t2 (3 段目)。
struct Tw27 {
 int E2[3];
 Shift2 s3[9];
 Tw27(int d, int W, int E): E2{E / 3, E / 3 + d, E / 3 + 2 * d}, s3{Shift2(d, W, E / 3 / 3), Shift2(d, W, E / 3 / 3 + d), Shift2(d, W, E / 3 / 3 + 2 * d), Shift2(d, W, (E / 3 + d) / 3), Shift2(d, W, (E / 3 + d) / 3 + d), Shift2(d, W, (E / 3 + d) / 3 + 2 * d), Shift2(d, W, (E / 3 + 2 * d) / 3), Shift2(d, W, (E / 3 + 2 * d) / 3 + d), Shift2(d, W, (E / 3 + 2 * d) / 3 + 2 * d)} {}
};
// unit i の 2 段目と 3 段目 (T の中でその場でと、T から dst へ)。
inline void fwd27_23(u64* T, u64* dst, size_t i, size_t m, int d, int W, const Tw27& w, u64* tmp) {
 const size_t sz= (size_t)2 * d * W;
 auto Tp= [&](int a, int b, int c) { return T + (size_t)(9 * a + 3 * b + c) * sz; };
 for(int t1= 0; t1 < 3; ++t1)
  for(int j3= 0; j3 < 3; ++j3) fwd_bfly_ip(Tp(t1, 0, j3), Tp(t1, 1, j3), Tp(t1, 2, j3), d, W, w.E2[t1], tmp);
 for(int t1= 0; t1 < 3; ++t1)
  for(int t2= 0; t2 < 3; ++t2) {
   const size_t o= (size_t)(9 * t1 + 3 * t2) * m + i;
   fwd_bfly(Tp(t1, t2, 0), Tp(t1, t2, 1), Tp(t1, t2, 2), dst + o * sz, dst + (o + m) * sz, dst + (o + 2 * m) * sz, d * W, w.s3[3 * t1 + t2]);
  }
}
inline void fwd27(const u64* src, u64* dst, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 27, dw= (size_t)d * W, sz= 2 * dw;
 const Shift2 s1(d, W, E);
 const Tw27 w(d, W, E);
 for(size_t i= 0; i < m; ++i) {
  for(int j= 0; j < 9; ++j) {
   const size_t b= i + m * j;
   fwd_bfly(src + b * sz, src + (b + 9 * m) * sz, src + (b + 18 * m) * sz, T + j * sz, T + (9 + j) * sz, T + (18 + j) * sz, dw, s1);
  }
  fwd27_23(T, dst, i, m, d, W, w, tmp);
 }
}
// fwd27 の逆をその場で。27 要素を T に (t1, t2, t3) の並びで読み、3 段目、2 段目、1 段目の順に戻して書き戻す。
inline void inv27(u64* blk, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 27, sz= (size_t)2 * d * W;
 auto Tp= [&](int a, int b, int c) { return T + (size_t)(9 * a + 3 * b + c) * sz; };
 for(size_t i= 0; i < m; ++i) {
  for(int t1= 0; t1 < 3; ++t1)
   for(int t2= 0; t2 < 3; ++t2) {
    const size_t o= (size_t)(9 * t1 + 3 * t2) * m + i;
    inv_bfly(blk + o * sz, blk + (o + m) * sz, blk + (o + 2 * m) * sz, Tp(t1, t2, 0), Tp(t1, t2, 1), Tp(t1, t2, 2), d, W, (E / 3 + d * t1) / 3 + d * t2, tmp);
   }
  for(int t1= 0; t1 < 3; ++t1)
   for(int j3= 0; j3 < 3; ++j3) inv_bfly(Tp(t1, 0, j3), Tp(t1, 1, j3), Tp(t1, 2, j3), Tp(t1, 0, j3), Tp(t1, 1, j3), Tp(t1, 2, j3), d, W, E / 3 + d * t1, tmp);
  for(int j2= 0; j2 < 3; ++j2)
   for(int j3= 0; j3 < 3; ++j3) {
    const size_t b= i + m * (j3 + 3 * j2);
    inv_bfly(Tp(0, j2, j3), Tp(1, j2, j3), Tp(2, j2, j3), blk + b * sz, blk + (b + 9 * m) * sz, blk + (b + 18 * m) * sz, d, W, E, tmp);
   }
 }
}
// 深さ優先に 3 段ずつ進める (残りは 2 段か 1 段)。1 回ごとに x と y を入れ替える。逆変換はその場で。
inline void fwd(u64* x, u64* y, int d, int W, int cnt, int E, u64* T) {
 if(cnt == 1) return;
 if(cnt == 3) return fwd3(x, y, d, W, cnt, E);
 if(cnt == 9) return fwd9(x, y, d, W, cnt, E, T);
 fwd27(x, y, d, W, cnt, E, T, T + (size_t)27 * 2 * d * W);
 const size_t m= cnt / 27, sz= (size_t)2 * d * W;
 for(int t= 0; t < 27; ++t) fwd(y + t * m * sz, x + t * m * sz, d, W, m, ((E / 3 + d * (t / 9)) / 3 + d * (t / 3 % 3)) / 3 + d * (t % 3), T);
}
inline int passes(int cnt) { return cnt == 1 ? 0 : cnt <= 9 ? 1 : 1 + passes(cnt / 27); }
inline void inv(u64* blk, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 if(cnt == 1) return;
 if(cnt == 3) return inv3(blk, d, W, cnt, E, tmp);
 if(cnt == 9) return inv9(blk, d, W, cnt, E, T, tmp);
 const size_t m= cnt / 27, sz= (size_t)2 * d * W;
 for(int t= 0; t < 27; ++t) inv(blk + t * m * sz, d, W, m, ((E / 3 + d * (t / 9)) / 3 + d * (t / 3 % 3)) / 3 + d * (t % 3), T, tmp);
 inv27(blk, d, W, cnt, E, T, tmp);
}
// A = lo + y^δ hi を mod (y^δ - x^d) (P) と mod (y^δ - x^{2d}) (Q) に分ける。x^d (lo, 0) = (0, lo)、x^{2d} (h, 0) = (h, h) から
// P_i = (lo_i, hi_i)、Q_i = (lo_i + hi_i, hi_i)。src は d 係数ずつの chunk で、n 個目より先は 0 とみなす。
template <class T> inline int split_run(const u64* lo, const u64* hi, u64* P, u64* Q, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T l= ld<T>(lo + j), h= ld<T>(hi + j);
  st<T>(P + j, l), st<T>(P + dw + j, h), st<T>(Q + j, l ^ h), st<T>(Q + dw + j, h);
 }
 return j;
}
inline void split(const u64* src, size_t n, u64* dst, int d, int W, int dl, u64* tmp) {
 const int dw= d * W, sz= 2 * dw;
 auto chunk= [&](size_t t, u64* buf) -> const u64* {
  const size_t b= t * dw;
  if(b + dw <= n) return src + b;
  std::fill(buf, buf + dw, 0);
  if(b < n) std::copy(src + b, src + n, buf);
  return buf;
 };
 for(int i= 0; i < dl; ++i) {
  const u64 *lo= chunk(i, tmp), *hi= chunk(dl + i, tmp + dw);
  u64 *P= dst + (size_t)i * sz, *Q= dst + (size_t)(dl + i) * sz;
  split_run<u64>(lo, hi, P, Q, split_run<VT>(lo, hi, P, Q, 0, dw), dw);
 }
}
// split の逆と、c_t x^{dt} の重ね合わせと、mod x^{2D} + x^D + 1 を 1 度に。hi = P + Q、lo = P + x^d hi (x^d + x^{2d} = 1)。
// c_t = lo_t (t < δ)、hi_{t-δ} (t >= δ) を chunk t に c_t.L、chunk t+1 に c_t.H と足し、はみ出す chunk 2δ は chunk 0 と chunk δ に畳む。
// i の小さい順に進めると、どの chunk も最初に触るときが代入になるので、出力を 0 で埋めずに済む (S0..S3 が代入か XOR か)。
template <class T, bool S0, bool S1, bool S2, bool S3> inline int merge_run(const u64* P, const u64* Q, u64* c0, u64* c1, u64* c2, u64* c3, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T pL= ld<T>(P + j), pH= ld<T>(P + dw + j), hL= pL ^ ld<T>(Q + j), hH= pH ^ ld<T>(Q + dw + j), v0= pL ^ hH, v1= pH ^ hL ^ hH;
  st<T>(c0 + j, S0 ? v0 : ld<T>(c0 + j) ^ v0), st<T>(c1 + j, S1 ? v1 : ld<T>(c1 + j) ^ v1), st<T>(c2 + j, S2 ? hL : ld<T>(c2 + j) ^ hL), st<T>(c3 + j, S3 ? hH : ld<T>(c3 + j) ^ hH);
 }
 return j;
}
// 最後の i: chunk δ-1 に lo.L、chunk δ に lo.H + hi.H (はみ出しの分)、chunk 2δ-1 に hi.L、chunk 0 に hi.H を足す。
template <class T> inline int merge_last(const u64* P, const u64* Q, u64* c0, u64* cd, u64* c2, u64* cz, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T pL= ld<T>(P + j), pH= ld<T>(P + dw + j), hL= pL ^ ld<T>(Q + j), hH= pH ^ ld<T>(Q + dw + j);
  st<T>(c0 + j, ld<T>(c0 + j) ^ pL ^ hH), st<T>(cd + j, ld<T>(cd + j) ^ pH ^ hL), st<T>(c2 + j, ld<T>(c2 + j) ^ hL), st<T>(cz + j, ld<T>(cz + j) ^ hH);
 }
 return j;
}
template <bool S0, bool S1, bool S2, bool S3> inline void merge_one(const u64* P, const u64* Q, u64* c0, u64* c1, u64* c2, u64* c3, int dw) {
 merge_run<u64, S0, S1, S2, S3>(P, Q, c0, c1, c2, c3, merge_run<VT, S0, S1, S2, S3>(P, Q, c0, c1, c2, c3, 0, dw), dw);
}
inline void merge(const u64* A, u64* c, int d, int W, int dl) {
 const int dw= d * W, sz= 2 * dw;
 const size_t Dw= (size_t)dw * dl;
 for(int i= 0; i < dl; ++i) {
  const u64 *P= A + (size_t)i * sz, *Q= A + (size_t)(dl + i) * sz;
  u64 *c0= c + (size_t)i * dw, *c1= c0 + dw, *c2= c + Dw + (size_t)i * dw, *c3= c2 + dw;
  if(i + 1 == dl) merge_last<u64>(P, Q, c0, c1, c2, c, merge_last<VT>(P, Q, c0, c1, c2, c, 0, dw), dw);
  else if(i == 0) merge_one<true, true, true, true>(P, Q, c0, c1, c2, c3, dw);
  else merge_one<false, true, false, true>(P, Q, c0, c1, c2, c3, dw);
 }
}
// ---------------- Schönhage の再帰 ----------------
constexpr int BK= 3;  // R_27 で base
// R_D (D = 3^k) を R_d (d = 3^kd) の 2δ 個の積に割る (δ = 3^(k - kd))。再帰が必ず k = BK で止まるよう kd >= BK にする (3^12 → 3^6 → 3^3)。
inline int kd_of(int k) { return std::max((k + 1) / 2, BK); }
inline size_t ws_size(int k, int W) {
 if(k <= BK) return 0;
 const int kd= kd_of(k), d= pow3(kd), D= pow3(k);
 return ((size_t)12 * D + 66 * d) * W + ws_size(kd, W);
}
// a (n 個) を変換して x に置く (y と T と tmp は作業用)。読み書きの回数が奇数なら y から始め、終わりが x に来るようにする。
inline void transform(const u64* a, size_t n, u64* x, u64* y, int d, int W, int dl, u64* T, u64* tmp) {
 const size_t sz= (size_t)2 * d * W;
 u64 *s= passes(dl) & 1 ? y : x, *o= passes(dl) & 1 ? x : y;
 split(a, n, s, d, W, dl, tmp);
 fwd(s, o, d, W, dl, d, T), fwd(s + dl * sz, o + dl * sz, d, W, dl, 2 * d, T);
}
// c = a b in R_D (D = 3^k)、W 本ずつ lane-sliced。c は a と同じ場所でもよい。
template <class P> inline void mul_R(int k, const u64* a, const u64* b, u64* c, u64* ws) {
 constexpr int W= P::W;
 if(k <= BK) return base27<P>(a, b, c);
 const int kd= kd_of(k), d= pow3(kd), dl= pow3(k - kd), sz= 2 * d * W;
 const size_t tot= (size_t)2 * dl * sz, D2= (size_t)2 * d * dl * W;
 u64 *A= ws, *Bv= ws + tot, *S= Bv + tot, *T= S + tot, *tmp= T + 29 * sz, *sub= tmp + 2 * sz;
 transform(a, D2, A, S, d, W, dl, T, tmp), transform(b, D2, Bv, S, d, W, dl, T, tmp);
 for(int i= 0; i < 2 * dl; ++i) mul_R<P>(kd, A + (size_t)i * sz, Bv + (size_t)i * sz, A + (size_t)i * sz, sub);
 inv(A, d, W, dl, d, T, tmp), inv(A + (size_t)dl * sz, d, W, dl, 2 * d, T, tmp);
 merge(A, c, d, W, dl);
}
// 一番上の最初の 2 段を、入力の chunk から直接読む形で (split を書かない)。入力の上半分が 0 (n, m <= D) なら P_i = Q_i = (a_i, 0) なので、
// ring element を下半分 L と上半分 H の 2 つのポインタで渡し、H は 0 の列を指す。区間ごとの読み出しは L か H の片方に収まる。
template <int TB, int TC> inline void fwd_seg_lh(const u64* A0L, const u64* A0H, const u64* A1L, const u64* A1H, const u64* A2L, const u64* A2H, u64* O0, u64* O1, u64* O2, int j0, int j1, int dw, const Seg& sb, const Seg& sc) {
 auto at= [&](const u64* L, const u64* H, int o) { return o + j0 >= dw ? H + (o - dw) : L + o; };
 const u64 *bu= at(A1L, A1H, sb.xo), *bv= at(A1L, A1H, sb.yo), *cu= at(A2L, A2H, sc.xo), *cv= at(A2L, A2H, sc.yo);
 u64 *o0H= O0 + dw, *o1H= O1 + dw, *o2H= O2 + dw;
 const int j= fwd_run<VT, TB, TC>(A0L, A0H, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j0, j1);
 fwd_run<u64, TB, TC>(A0L, A0H, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j, j1);
}
using FwdLHFn= void (*)(const u64*, const u64*, const u64*, const u64*, const u64*, const u64*, u64*, u64*, u64*, int, int, int, const Seg&, const Seg&);
constexpr FwdLHFn FWDLH[3][3]= {{fwd_seg_lh<0, 0>, fwd_seg_lh<0, 1>, fwd_seg_lh<0, 2>}, {fwd_seg_lh<1, 0>, fwd_seg_lh<1, 1>, fwd_seg_lh<1, 2>}, {fwd_seg_lh<2, 0>, fwd_seg_lh<2, 1>, fwd_seg_lh<2, 2>}};
// src の chunk k (d 個) を L に、0 の列 Z を H にした ring element を 3 つ取り、butterfly を 1 つ。n 個目より先は 0 (chunk をまたぐものは pc に写す)。
struct Chunks {
 const u64* src;
 size_t n;
 int d;
 const u64* Z;
 u64* pc;
 const u64* get(size_t k) const {
  const size_t b= k * d;
  if(b + d <= n) return src + b;
  if(b >= n) return Z;
  return pc;
 }
};
// 一番上の最初の 3 段 (radix-27 の 1 回目) を、入力の chunk から直接読む形で。1 段目の読み出しだけが fwd27 と違う。一番上が 27 要素以上のときだけ使う。
inline void fwd27_first(const Chunks& in, u64* dst, int d, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 27, dw= d, sz= 2 * dw;
 const Shift2 s1(d, 1, E);
 const Tw27 w(d, 1, E);
 for(size_t i= 0; i < m; ++i) {
  for(int j= 0; j < 9; ++j) {
   const u64 *L0= in.get(i + m * j), *L1= in.get(i + m * (9 + j)), *L2= in.get(i + m * (18 + j));
   u64 *O0= T + j * sz, *O1= T + (9 + j) * sz, *O2= T + (18 + j) * sz;
   for(int x= 0; x < s1.nb; ++x)
    for(int y= 0; y < s1.nc; ++y) {
     const int j0= std::max(s1.sb[x].j0, s1.sc[y].j0), j1= std::min(s1.sb[x].j1, s1.sc[y].j1);
     if(j0 < j1) FWDLH[s1.sb[x].ty][s1.sc[y].ty](L0, in.Z, L1, in.Z, L2, in.Z, O0, O1, O2, j0, j1, dw, s1.sb[x], s1.sc[y]);
    }
  }
  fwd27_23(T, dst, i, m, d, 1, w, tmp);
 }
}
// a (n 個、n <= D) を変換して x に置く。最初の 3 段は入力から直接読み、残りの段の回数の偶奇で最初の書き先を x か y に決める。
inline void transform_first(const u64* a, size_t n, u64* x, u64* y, int d, int dl, u64* T, u64* Z, u64* pc) {
 const size_t sz= (size_t)2 * d, m= dl / 27;
 if(n % d) std::fill(pc, pc + d, 0), std::copy(a + n / d * d, a + n, pc);
 const Chunks in{a, n, d, Z, pc};
 u64 *s= passes(m) & 1 ? y : x, *o= passes(m) & 1 ? x : y;
 for(int h= 0; h < 2; ++h) {
  const int E= (h + 1) * d;
  u64 *sh= s + h * dl * sz, *oh= o + h * dl * sz;
  fwd27_first(in, sh, d, dl, E, T, T + 27 * sz);
  for(int t= 0; t < 27; ++t) fwd(sh + t * m * sz, oh + t * m * sz, d, 1, m, ((E / 3 + d * (t / 9)) / 3 + d * (t / 3 % 3)) / 3 + d * (t % 3), T);
 }
}
// 一番上: 2 つの入力をそれぞれ最後まで変換し、各点の積を W 本ずつ lane-sliced に並べ替えて掛け、全体を逆変換する。
template <class P> inline void mul_top(int k, const u64* a, size_t n, const u64* b, size_t m, u64* c) {
 constexpr int W= P::W;
 const int kd= kd_of(k), d= pow3(kd), dl= pow3(k - kd);
 const size_t sz= (size_t)2 * d, tot= 2 * dl * sz;
 std::unique_ptr<u64[]> buf(new u64[3 * tot + 31 * sz + 2 * sz * W + ws_size(kd, W)]);  // 0 で埋めない (どこも書いてから読む)
 u64 *A= buf.get(), *Bv= A + tot, *S= Bv + tot, *T= S + tot, *tmp= T + 29 * sz, *LA= tmp + 2 * sz, *LB= LA + sz * W, *sub= LB + sz * W;
 const size_t D= (size_t)d * dl;
 if(n <= D && m <= D && dl >= 27) {
  u64 *Z= LA, *pc= LB;  // LA, LB はまだ使わないので、0 の列と chunk の写しに借りる
  std::fill(Z, Z + d, 0);
  transform_first(a, n, A, S, d, dl, T, Z, pc), transform_first(b, m, Bv, S, d, dl, T, Z, pc);
 } else transform(a, n, A, S, d, 1, dl, T, tmp), transform(b, m, Bv, S, d, 1, dl, T, tmp);
 for(int g= 0; g < 2 * dl; g+= W) {
  const int w= std::min(W, 2 * dl - g);
  u64 *ea[W], *eb[W];
  for(int l= 0; l < w; ++l) ea[l]= A + (g + l) * sz, eb[l]= Bv + (g + l) * sz;
  to_lanes<W>(ea, w, LA, sz), to_lanes<W>(eb, w, LB, sz);
  mul_R<P>(kd, LA, LB, LA, sub);
  from_lanes<W>(LA, ea, w, sz);
 }
 inv(A, d, 1, dl, d, T, tmp), inv(A + dl * sz, d, 1, dl, 2 * d, T, tmp);
 merge(A, c, d, 1, dl);
}
template <class P> inline std::vector<u64> convolve(const std::vector<u64>& a, const std::vector<u64>& b) {
 const int n= a.size(), m= b.size();
 if(!n || !m) return {};
 const int N= n + m - 1;
 if(N <= 2 * pow3(BK + 1)) return naive(a, b);
 int k= BK + 2;
 while(2 * pow3(k) < N) ++k;
 std::vector<u64> c(2 * pow3(k));
 mul_top<P>(k, a.data(), n, b.data(), m, c.data());
 c.resize(N);
 return c;
}
}  // namespace schoenhage3
#ifdef __x86_64__
// ここから AVX-512 の版 (実行時に CPU が対応していれば使う)。GCC は #pragma GCC target、clang は #pragma clang attribute で領域を開く。
#ifdef __clang__
#pragma clang attribute push(__attribute__((target("avx512f,avx512bw,avx512dq,avx512vl,vpclmulqdq"))), apply_to = function)
#else
#pragma GCC push_options
#pragma GCC target("avx512f,avx512bw,avx512dq,avx512vl,vpclmulqdq")
#endif
namespace schoenhage3z {
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
// 8 本。512 bit の clmul (imm 0x00 で lane 0, 2, 4, 6、0x11 で lane 1, 3, 5, 7)。
struct P8 {
 static constexpr int W= 8;
 using V= __m512i;
 struct Acc {
  V e, o;
 };
 static V ldv(const u64* p) { return _mm512_loadu_si512((const void*)p); }
 static void stv(u64* p, const V& v) { _mm512_storeu_si512((void*)p, v); }
 static V x(const V& a, const V& b) { return _mm512_xor_si512(a, b); }
 static V bc(u64 c) { return _mm512_set1_epi64((long long)c); }
 static Acc mul(const V& a, const V& b) { return {_mm512_clmulepi64_epi128(a, b, 0x00), _mm512_clmulepi64_epi128(a, b, 0x11)}; }
 static V fin(const Acc& c) {
  const __m512i R= _mm512_broadcast_i32x4(_mm_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0));
  const __m512i lo= _mm512_unpacklo_epi64(c.e, c.o), h= _mm512_unpackhi_epi64(c.e, c.o), d= _mm512_xor_si512(h, _mm512_slli_epi64(h, 1));
  return _mm512_xor_si512(_mm512_xor_si512(lo, _mm512_shuffle_epi8(R, _mm512_srli_epi64(h, 60))), _mm512_xor_si512(d, _mm512_slli_epi64(d, 3)));
 }
};
// 8 x 8 の u64 の転置 (r[l] の k 番目 -> r[k] の l 番目)。
inline void tr8(__m512i* r) {
 const __m512i t0= _mm512_unpacklo_epi64(r[0], r[1]), t1= _mm512_unpackhi_epi64(r[0], r[1]), t2= _mm512_unpacklo_epi64(r[2], r[3]), t3= _mm512_unpackhi_epi64(r[2], r[3]);
 const __m512i t4= _mm512_unpacklo_epi64(r[4], r[5]), t5= _mm512_unpackhi_epi64(r[4], r[5]), t6= _mm512_unpacklo_epi64(r[6], r[7]), t7= _mm512_unpackhi_epi64(r[6], r[7]);
 const __m512i u0= _mm512_shuffle_i32x4(t0, t2, 0x88), u1= _mm512_shuffle_i32x4(t1, t3, 0x88), u2= _mm512_shuffle_i32x4(t0, t2, 0xdd), u3= _mm512_shuffle_i32x4(t1, t3, 0xdd);
 const __m512i u4= _mm512_shuffle_i32x4(t4, t6, 0x88), u5= _mm512_shuffle_i32x4(t5, t7, 0x88), u6= _mm512_shuffle_i32x4(t4, t6, 0xdd), u7= _mm512_shuffle_i32x4(t5, t7, 0xdd);
 r[0]= _mm512_shuffle_i32x4(u0, u4, 0x88), r[1]= _mm512_shuffle_i32x4(u1, u5, 0x88), r[2]= _mm512_shuffle_i32x4(u2, u6, 0x88), r[3]= _mm512_shuffle_i32x4(u3, u7, 0x88);
 r[4]= _mm512_shuffle_i32x4(u0, u4, 0xdd), r[5]= _mm512_shuffle_i32x4(u1, u5, 0xdd), r[6]= _mm512_shuffle_i32x4(u2, u6, 0xdd), r[7]= _mm512_shuffle_i32x4(u3, u7, 0xdd);
}
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
 } else if constexpr(W == 8) {
  if(w == 8)
   for(; t + 8 <= n; t+= 8) {
    __m512i r[8];
    for(int l= 0; l < 8; ++l) r[l]= _mm512_loadu_si512((const void*)(e[l] + t));
    tr8(r);
    for(int k= 0; k < 8; ++k) _mm512_storeu_si512((void*)(out + 8 * (t + k)), r[k]);
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
 } else if constexpr(W == 8) {
  if(w == 8)
   for(; t + 8 <= n; t+= 8) {
    __m512i r[8];
    for(int k= 0; k < 8; ++k) r[k]= _mm512_loadu_si512((const void*)(in + 8 * (t + k)));
    tr8(r);
    for(int l= 0; l < 8; ++l) _mm512_storeu_si512((void*)(e[l] + t), r[l]);
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
// ---------------- base: R_27 の積を W 本まとめて ----------------
// 出力ごとに足し込む schoolbook。c[0, 2N-1)
template <class P, int N> inline void school(const typename P::V* a, const typename P::V* b, typename P::Acc* c) {
#pragma GCC unroll 16
 for(int k= 0; k < 2 * N - 1; ++k) {
  typename P::Acc s= P::mul(a[k < N ? 0 : k - N + 1], b[k < N ? k : N - 1]);
#pragma GCC unroll 16
  for(int i= (k < N ? 1 : k - N + 2); i <= (k < N ? k : N - 1); ++i) s= ax<P>(s, P::mul(a[i], b[k - i]));
  c[k]= s;
 }
}
// c[0, 2N-1) = a b (Karatsuba)。1 まで割り切る (27 → 14, 13 → 7, 6 → 4, 3 → 2, 1 → 1)。R_27 の積 1 つで葉の積が 663 回。
template <class P, int N> inline void kara(const typename P::V* a, const typename P::V* b, typename P::Acc* c) {
 using V= typename P::V;
 using Acc= typename P::Acc;
 if constexpr(N <= 1) school<P, N>(a, b, c);
 else {
  constexpr int H= (N + 1) / 2, L= N - H;
  V sa[H], sb[H];
  for(int i= 0; i < L; ++i) sa[i]= P::x(a[i], a[H + i]), sb[i]= P::x(b[i], b[H + i]);
  if constexpr(H > L) sa[H - 1]= a[H - 1], sb[H - 1]= b[H - 1];
  Acc m[2 * H - 1];
  kara<P, H>(a, b, c);
  kara<P, L>(a + H, b + H, c + 2 * H);
  kara<P, H>(sa, sb, m);
  // m ^= c[0, 2H-1) ^ c[2H, 2N-1); c[H, 3H-1) ^= m。c[2H-1] は 0。
  for(int i= 0; i < 2 * L - 1; ++i) m[i]= ax<P>(ax<P>(m[i], c[i]), c[2 * H + i]);
  for(int i= 2 * L - 1; i < 2 * H - 1; ++i) m[i]= ax<P>(m[i], c[i]);
  c[2 * H - 1]= m[H - 1];
  for(int i= 0; i < H - 1; ++i) c[H + i]= ax<P>(c[H + i], m[i]);
  for(int i= H; i < 2 * H - 1; ++i) c[H + i]= ax<P>(c[H + i], m[i]);
 }
}
// R_27 = F[x]/(x^54 + x^27 + 1) の積。x^27 で 2 つに割る Karatsuba の足し戻しを、x^54 = x^27 + 1 の畳みと合わせる。
template <class P> inline void base27(const u64* a, const u64* b, u64* c) {
 using V= typename P::V;
 using Acc= typename P::Acc;
 constexpr int W= P::W;
 V aL[27], aH[27], bL[27], bH[27], sa[27], sb[27];
 for(int i= 0; i < 27; ++i) aL[i]= P::ldv(a + i * W), aH[i]= P::ldv(a + (27 + i) * W), bL[i]= P::ldv(b + i * W), bH[i]= P::ldv(b + (27 + i) * W), sa[i]= P::x(aL[i], aH[i]), sb[i]= P::x(bL[i], bH[i]);
 Acc p0[53], p2[53], p1[53];
 kara<P, 27>(aL, bL, p0), kara<P, 27>(aH, bH, p2), kara<P, 27>(sa, sb, p1);
 // S = p0 + p2, T = p1 + p0。c[i] = S[i] + T[i+27] (i < 26)、c[26] = S[26]、c[i] = S[i] + T[i-27] + T[i] (27 <= i <= 52)、c[53] = T[26]
 for(int i= 0; i < 26; ++i) P::stv(c + i * W, P::fin(ax<P>(ax<P>(p0[i], p2[i]), ax<P>(p1[i + 27], p0[i + 27]))));
 P::stv(c + 26 * W, P::fin(ax<P>(p0[26], p2[26])));
 for(int i= 27; i < 53; ++i) P::stv(c + i * W, P::fin(ax<P>(ax<P>(ax<P>(p0[i], p2[i]), ax<P>(p1[i - 27], p0[i - 27])), ax<P>(p1[i], p0[i]))));
 P::stv(c + 53 * W, P::fin(ax<P>(p1[26], p0[26])));
}
// ---------------- R_d 上の FFT ----------------
// R_d の要素は 2d 個の係数で、係数 1 つが W 個の u64 (lane)。dw = d W が下半分 L と上半分 H の長さ。VT は XOR に使う SIMD の幅。
typedef u64 V8 __attribute__((vector_size(64)));
using VT= V8;
inline int pow3(int k) {
 int r= 1;
 while(k--) r*= 3;
 return r;
}
// ずらした要素 x^e A の位置 j (下半分) と dw + j (上半分) の値を、A からの 2 つの読み出し u, v で作る。
// ty 0: (L, H) = (u, v)、ty 1: (L, T) = (u, v)、ty 2: (H, T) = (u, v)。T = L ^ H。x^{2d} = x^d + 1 から、どの区間もこの 3 通りに収まる。
template <int TY, class T> inline void tri(const T& u, const T& v, T& L, T& H, T& X) {
 if constexpr(TY == 0) L= u, H= v, X= u ^ v;
 else if constexpr(TY == 1) L= u, X= v, H= u ^ v;
 else H= u, X= v, L= u ^ v;
}
struct Seg {
 int j0, j1, xo, yo, ty;
};
// x^e A (0 <= e < 3d) の読み出し方を j の区間ごとに (u = A[j + xo]、v = A[j + yo])。e = q d + r、r は u64 単位で r W。
inline int segs(int d, int W, int e, Seg* s) {
 const int dw= d * W, q= e / d, r= (e % d) * W;
 int n= 0;
 if(q == 0) {
  if(r) s[n++]= {0, r, 2 * dw - r, dw - r, 1};
  s[n++]= {r, dw, -r, dw - r, 0};
 } else if(q == 1) {
  if(r) s[n++]= {0, r, dw - r, 2 * dw - r, 2};
  s[n++]= {r, dw, dw - r, -r, 1};
 } else {
  if(r) s[n++]= {0, r, dw - r, 2 * dw - r, 0};
  s[n++]= {r, dw, -r, dw - r, 2};
 }
 return n;
}
// 順変換の butterfly の 1 区間。O_t = A0 + w^t (x^{e1} A1) + w^{2t} (x^{e2} A2) (w = x^d、x^d (L, H) = (H, L + H))。
// 書き込みが何とでも重なりうる (memcpy) ので、ループの中で参照を読み直さないよう、読み書きの位置は先にポインタにしておく。
template <class T, int TB, int TC> inline int fwd_run(const u64* aLp, const u64* aHp, const u64* bu, const u64* bv, const u64* cu, const u64* cv, u64* o0L, u64* o0H, u64* o1L, u64* o1H, u64* o2L, u64* o2H, int j, int j1) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= j1; j+= L) {
  const T aL= ld<T>(aLp + j), aH= ld<T>(aHp + j);
  T bL, bH, t1, cL, cH, t2;
  tri<TB>(ld<T>(bu + j), ld<T>(bv + j), bL, bH, t1);
  tri<TC>(ld<T>(cu + j), ld<T>(cv + j), cL, cH, t2);
  st<T>(o0L + j, aL ^ bL ^ cL), st<T>(o0H + j, aH ^ bH ^ cH);
  st<T>(o1L + j, aL ^ bH ^ t2), st<T>(o1H + j, aH ^ t1 ^ cL);
  st<T>(o2L + j, aL ^ t1 ^ cH), st<T>(o2H + j, aH ^ bL ^ t2);
 }
 return j;
}
template <int TB, int TC> inline void fwd_seg(const u64* A0, const u64* A1, const u64* A2, u64* O0, u64* O1, u64* O2, int j0, int j1, int dw, const Seg& sb, const Seg& sc) {
 const u64 *aH= A0 + dw, *bu= A1 + sb.xo, *bv= A1 + sb.yo, *cu= A2 + sc.xo, *cv= A2 + sc.yo;
 u64 *o0H= O0 + dw, *o1H= O1 + dw, *o2H= O2 + dw;
 const int j= fwd_run<V4, TB, TC>(A0, aH, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, fwd_run<VT, TB, TC>(A0, aH, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j0, j1), j1);
 fwd_run<u64, TB, TC>(A0, aH, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j, j1);
}
using FwdFn= void (*)(const u64*, const u64*, const u64*, u64*, u64*, u64*, int, int, int, const Seg&, const Seg&);
constexpr FwdFn FWD[3][3]= {{fwd_seg<0, 0>, fwd_seg<0, 1>, fwd_seg<0, 2>}, {fwd_seg<1, 0>, fwd_seg<1, 1>, fwd_seg<1, 2>}, {fwd_seg<2, 0>, fwd_seg<2, 1>, fwd_seg<2, 2>}};
// 順変換の butterfly 1 つ。A mod (y^cnt - x^E) の 3 つ組 (A0, A1, A2) から、y^{cnt/3} = x^{E/3 + d t} での値 O_t (t = 0, 1, 2) を作る。
// ずらしは読み出しの添字に吸収する (書き込みは src と別の場所)。
struct Shift2 {
 Seg sb[2], sc[2];
 int nb, nc;
 Shift2(int d, int W, int E): nb(segs(d, W, E / 3, sb)), nc(segs(d, W, 2 * E / 3, sc)) {}
};
inline void fwd_bfly(const u64* A0, const u64* A1, const u64* A2, u64* O0, u64* O1, u64* O2, int dw, const Shift2& s) {
 for(int x= 0; x < s.nb; ++x)
  for(int y= 0; y < s.nc; ++y) {
   const int j0= std::max(s.sb[x].j0, s.sc[y].j0), j1= std::min(s.sb[x].j1, s.sc[y].j1);
   if(j0 < j1) FWD[s.sb[x].ty][s.sc[y].ty](A0, A1, A2, O0, O1, O2, j0, j1, dw, s.sb[x], s.sc[y]);
  }
}
// 1 段を src の block (cnt 要素) から dst へ。
inline void fwd3(const u64* src, u64* dst, int d, int W, int cnt, int E) {
 const size_t m= cnt / 3, dw= (size_t)d * W, sz= 2 * dw;
 const Shift2 s(d, W, E);
 for(size_t i= 0; i < m; ++i) fwd_bfly(src + i * sz, src + (m + i) * sz, src + (2 * m + i) * sz, dst + i * sz, dst + (m + i) * sz, dst + (2 * m + i) * sz, dw, s);
}
template <class T, int TY> inline int shift_run(const u64* pu, const u64* pv, u64* oL, u64* oH, int j, int j1) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= j1; j+= L) {
  const T u= ld<T>(pu + j), v= ld<T>(pv + j);
  if constexpr(TY == 0) st<T>(oL + j, u), st<T>(oH + j, v);
  else if constexpr(TY == 1) st<T>(oL + j, u), st<T>(oH + j, u ^ v);
  else st<T>(oL + j, u ^ v), st<T>(oH + j, u);
 }
 return j;
}
template <int TY> inline void shift_seg(const u64* A, u64* out, int dw, const Seg& s) {
 const u64 *pu= A + s.xo, *pv= A + s.yo;
 u64* oH= out + dw;
 shift_run<u64, TY>(pu, pv, out, oH, shift_run<V4, TY>(pu, pv, out, oH, shift_run<VT, TY>(pu, pv, out, oH, s.j0, s.j1), s.j1), s.j1);
}
// out = x^e A
inline void shift(const u64* A, u64* out, int d, int W, int e) {
 Seg s[2];
 const int n= segs(d, W, e, s), dw= d * W;
 for(int x= 0; x < n; ++x) {
  if(s[x].ty == 0) shift_seg<0>(A, out, dw, s[x]);
  else if(s[x].ty == 1) shift_seg<1>(A, out, dw, s[x]);
  else shift_seg<2>(A, out, dw, s[x]);
 }
}
template <class T> inline int inv_run(const u64* O0, const u64* O0H, const u64* O1, const u64* O1H, const u64* O2, const u64* O2H, u64* A0, u64* A0H, u64* tB, u64* tBH, u64* tC, u64* tCH, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T aL= ld<T>(O0 + j), aH= ld<T>(O0H + j), bL= ld<T>(O1 + j), bH= ld<T>(O1H + j), cL= ld<T>(O2 + j), cH= ld<T>(O2H + j), s1= bL ^ bH, s2= cL ^ cH;
  st<T>(A0 + j, aL ^ bL ^ cL), st<T>(A0H + j, aH ^ bH ^ cH);
  st<T>(tB + j, aL ^ s1 ^ cH), st<T>(tBH + j, aH ^ bL ^ s2);
  st<T>(tC + j, aL ^ bH ^ s2), st<T>(tCH + j, aH ^ s1 ^ cL);
 }
 return j;
}
// 逆変換の butterfly 1 つ。O_t から A0 = O0 + O1 + O2 と、ずらす前の B = O0 + w^2 O1 + w O2、C = O0 + w O1 + w^2 O2 を作り (1/3 = 1)、
// B, C を x^{-E/3}, x^{-2E/3} でずらして A1, A2 に置く。A0 は O0 と同じ場所でもよい。
inline void inv_bfly(const u64* O0, const u64* O1, const u64* O2, u64* A0, u64* A1, u64* A2, int d, int W, int E, u64* tmp) {
 const int dw= d * W, e1= E / 3, e2= 2 * E / 3;
 u64 *tB= tmp, *tC= tmp + 2 * dw;
 const u64 *O0H= O0 + dw, *O1H= O1 + dw, *O2H= O2 + dw;
 u64 *A0H= A0 + dw, *tBH= tB + dw, *tCH= tC + dw;
 inv_run<u64>(O0, O0H, O1, O1H, O2, O2H, A0, A0H, tB, tBH, tC, tCH, inv_run<V4>(O0, O0H, O1, O1H, O2, O2H, A0, A0H, tB, tBH, tC, tCH, inv_run<VT>(O0, O0H, O1, O1H, O2, O2H, A0, A0H, tB, tBH, tC, tCH, 0, dw), dw), dw);
 shift(tB, A1, d, W, e1 ? 3 * d - e1 : 0), shift(tC, A2, d, W, e2 ? 3 * d - e2 : 0);
}
inline void inv3(u64* blk, int d, int W, int cnt, int E, u64* tmp) {
 const size_t m= cnt / 3, sz= (size_t)2 * d * W;
 for(size_t i= 0; i < m; ++i) {
  u64 *A0= blk + i * sz, *A1= A0 + m * sz, *A2= A1 + m * sz;
  inv_bfly(A0, A1, A2, A0, A1, A2, d, W, E, tmp);
 }
}
// 2 段を 1 回の読み書きで src の block から dst へ。間の 9 要素は T に置く。
inline void fwd9(const u64* src, u64* dst, int d, int W, int cnt, int E, u64* T) {
 const size_t m= cnt / 9, dw= (size_t)d * W, sz= 2 * dw;
 const Shift2 s1(d, W, E), s2[3]= {Shift2(d, W, E / 3), Shift2(d, W, E / 3 + d), Shift2(d, W, E / 3 + 2 * d)};
 for(size_t i= 0; i < m; ++i) {
  for(size_t j= 0; j < 3; ++j) fwd_bfly(src + (i + j * m) * sz, src + (i + j * m + 3 * m) * sz, src + (i + j * m + 6 * m) * sz, T + j * sz, T + (3 + j) * sz, T + (6 + j) * sz, dw, s1);
  for(size_t t= 0; t < 3; ++t) fwd_bfly(T + 3 * t * sz, T + (3 * t + 1) * sz, T + (3 * t + 2) * sz, dst + (3 * t * m + i) * sz, dst + (3 * t * m + m + i) * sz, dst + (3 * t * m + 2 * m + i) * sz, dw, s2[t]);
 }
}
// fwd9 の逆をその場で。9 要素を T に読んでから書き戻す。
inline void inv9(u64* blk, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 9, sz= (size_t)2 * d * W;
 for(size_t i= 0; i < m; ++i) {
  for(size_t t= 0; t < 3; ++t) inv_bfly(blk + (3 * t * m + i) * sz, blk + (3 * t * m + m + i) * sz, blk + (3 * t * m + 2 * m + i) * sz, T + 3 * t * sz, T + (3 * t + 1) * sz, T + (3 * t + 2) * sz, d, W, E / 3 + d * (int)t, tmp);
  for(size_t j= 0; j < 3; ++j) inv_bfly(T + j * sz, T + (3 + j) * sz, T + (6 + j) * sz, blk + (i + j * m) * sz, blk + (i + j * m + 3 * m) * sz, blk + (i + j * m + 6 * m) * sz, d, W, E, tmp);
 }
}
// 3 段を 1 回の読み書きで src の block から dst へ (27 要素を T に置く)。1 段目は src から T へ、2 段目は T の中でその場で
// (ずらした 2 つを tmp に写してから合わせる)、3 段目は T から dst へ。unit i の入力は src[i + m (j3 + 3 j2 + 9 j1)]、
// T の並びは (t1, j2, j3) → (t1, t2, j3)、出力は dst[(9 t1 + 3 t2 + t3) m + i]。
inline void fwd_bfly_ip(u64* A0, u64* A1, u64* A2, int d, int W, int E, u64* tmp) {
 const int dw= d * W;
 u64 *tB= tmp, *tC= tmp + 2 * dw;
 shift(A1, tB, d, W, E / 3), shift(A2, tC, d, W, 2 * E / 3);
 fwd_run<u64, 0, 0>(A0, A0 + dw, tB, tB + dw, tC, tC + dw, A0, A0 + dw, A1, A1 + dw, A2, A2 + dw, fwd_run<V4, 0, 0>(A0, A0 + dw, tB, tB + dw, tC, tC + dw, A0, A0 + dw, A1, A1 + dw, A2, A2 + dw, fwd_run<VT, 0, 0>(A0, A0 + dw, tB, tB + dw, tC, tC + dw, A0, A0 + dw, A1, A1 + dw, A2, A2 + dw, 0, dw), dw), dw);
}
// 2 段目と 3 段目の twiddle。E2[t1] = E/3 + d t1 (2 段目)、s3[3 t1 + t2] は E2[t1]/3 + d t2 (3 段目)。
struct Tw27 {
 int E2[3];
 Shift2 s3[9];
 Tw27(int d, int W, int E): E2{E / 3, E / 3 + d, E / 3 + 2 * d}, s3{Shift2(d, W, E / 3 / 3), Shift2(d, W, E / 3 / 3 + d), Shift2(d, W, E / 3 / 3 + 2 * d), Shift2(d, W, (E / 3 + d) / 3), Shift2(d, W, (E / 3 + d) / 3 + d), Shift2(d, W, (E / 3 + d) / 3 + 2 * d), Shift2(d, W, (E / 3 + 2 * d) / 3), Shift2(d, W, (E / 3 + 2 * d) / 3 + d), Shift2(d, W, (E / 3 + 2 * d) / 3 + 2 * d)} {}
};
// unit i の 2 段目と 3 段目 (T の中でその場でと、T から dst へ)。
inline void fwd27_23(u64* T, u64* dst, size_t i, size_t m, int d, int W, const Tw27& w, u64* tmp) {
 const size_t sz= (size_t)2 * d * W;
 auto Tp= [&](int a, int b, int c) { return T + (size_t)(9 * a + 3 * b + c) * sz; };
 for(int t1= 0; t1 < 3; ++t1)
  for(int j3= 0; j3 < 3; ++j3) fwd_bfly_ip(Tp(t1, 0, j3), Tp(t1, 1, j3), Tp(t1, 2, j3), d, W, w.E2[t1], tmp);
 for(int t1= 0; t1 < 3; ++t1)
  for(int t2= 0; t2 < 3; ++t2) {
   const size_t o= (size_t)(9 * t1 + 3 * t2) * m + i;
   fwd_bfly(Tp(t1, t2, 0), Tp(t1, t2, 1), Tp(t1, t2, 2), dst + o * sz, dst + (o + m) * sz, dst + (o + 2 * m) * sz, d * W, w.s3[3 * t1 + t2]);
  }
}
inline void fwd27(const u64* src, u64* dst, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 27, dw= (size_t)d * W, sz= 2 * dw;
 const Shift2 s1(d, W, E);
 const Tw27 w(d, W, E);
 for(size_t i= 0; i < m; ++i) {
  for(int j= 0; j < 9; ++j) {
   const size_t b= i + m * j;
   fwd_bfly(src + b * sz, src + (b + 9 * m) * sz, src + (b + 18 * m) * sz, T + j * sz, T + (9 + j) * sz, T + (18 + j) * sz, dw, s1);
  }
  fwd27_23(T, dst, i, m, d, W, w, tmp);
 }
}
// fwd27 の逆をその場で。27 要素を T に (t1, t2, t3) の並びで読み、3 段目、2 段目、1 段目の順に戻して書き戻す。
inline void inv27(u64* blk, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 27, sz= (size_t)2 * d * W;
 auto Tp= [&](int a, int b, int c) { return T + (size_t)(9 * a + 3 * b + c) * sz; };
 for(size_t i= 0; i < m; ++i) {
  for(int t1= 0; t1 < 3; ++t1)
   for(int t2= 0; t2 < 3; ++t2) {
    const size_t o= (size_t)(9 * t1 + 3 * t2) * m + i;
    inv_bfly(blk + o * sz, blk + (o + m) * sz, blk + (o + 2 * m) * sz, Tp(t1, t2, 0), Tp(t1, t2, 1), Tp(t1, t2, 2), d, W, (E / 3 + d * t1) / 3 + d * t2, tmp);
   }
  for(int t1= 0; t1 < 3; ++t1)
   for(int j3= 0; j3 < 3; ++j3) inv_bfly(Tp(t1, 0, j3), Tp(t1, 1, j3), Tp(t1, 2, j3), Tp(t1, 0, j3), Tp(t1, 1, j3), Tp(t1, 2, j3), d, W, E / 3 + d * t1, tmp);
  for(int j2= 0; j2 < 3; ++j2)
   for(int j3= 0; j3 < 3; ++j3) {
    const size_t b= i + m * (j3 + 3 * j2);
    inv_bfly(Tp(0, j2, j3), Tp(1, j2, j3), Tp(2, j2, j3), blk + b * sz, blk + (b + 9 * m) * sz, blk + (b + 18 * m) * sz, d, W, E, tmp);
   }
 }
}
// 深さ優先に 3 段ずつ進める (残りは 2 段か 1 段)。1 回ごとに x と y を入れ替える。逆変換はその場で。
inline void fwd(u64* x, u64* y, int d, int W, int cnt, int E, u64* T) {
 if(cnt == 1) return;
 if(cnt == 3) return fwd3(x, y, d, W, cnt, E);
 if(cnt == 9) return fwd9(x, y, d, W, cnt, E, T);
 fwd27(x, y, d, W, cnt, E, T, T + (size_t)27 * 2 * d * W);
 const size_t m= cnt / 27, sz= (size_t)2 * d * W;
 for(int t= 0; t < 27; ++t) fwd(y + t * m * sz, x + t * m * sz, d, W, m, ((E / 3 + d * (t / 9)) / 3 + d * (t / 3 % 3)) / 3 + d * (t % 3), T);
}
inline int passes(int cnt) { return cnt == 1 ? 0 : cnt <= 9 ? 1 : 1 + passes(cnt / 27); }
inline void inv(u64* blk, int d, int W, int cnt, int E, u64* T, u64* tmp) {
 if(cnt == 1) return;
 if(cnt == 3) return inv3(blk, d, W, cnt, E, tmp);
 if(cnt == 9) return inv9(blk, d, W, cnt, E, T, tmp);
 const size_t m= cnt / 27, sz= (size_t)2 * d * W;
 for(int t= 0; t < 27; ++t) inv(blk + t * m * sz, d, W, m, ((E / 3 + d * (t / 9)) / 3 + d * (t / 3 % 3)) / 3 + d * (t % 3), T, tmp);
 inv27(blk, d, W, cnt, E, T, tmp);
}
// A = lo + y^δ hi を mod (y^δ - x^d) (P) と mod (y^δ - x^{2d}) (Q) に分ける。x^d (lo, 0) = (0, lo)、x^{2d} (h, 0) = (h, h) から
// P_i = (lo_i, hi_i)、Q_i = (lo_i + hi_i, hi_i)。src は d 係数ずつの chunk で、n 個目より先は 0 とみなす。
template <class T> inline int split_run(const u64* lo, const u64* hi, u64* P, u64* Q, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T l= ld<T>(lo + j), h= ld<T>(hi + j);
  st<T>(P + j, l), st<T>(P + dw + j, h), st<T>(Q + j, l ^ h), st<T>(Q + dw + j, h);
 }
 return j;
}
inline void split(const u64* src, size_t n, u64* dst, int d, int W, int dl, u64* tmp) {
 const int dw= d * W, sz= 2 * dw;
 auto chunk= [&](size_t t, u64* buf) -> const u64* {
  const size_t b= t * dw;
  if(b + dw <= n) return src + b;
  std::fill(buf, buf + dw, 0);
  if(b < n) std::copy(src + b, src + n, buf);
  return buf;
 };
 for(int i= 0; i < dl; ++i) {
  const u64 *lo= chunk(i, tmp), *hi= chunk(dl + i, tmp + dw);
  u64 *P= dst + (size_t)i * sz, *Q= dst + (size_t)(dl + i) * sz;
  split_run<u64>(lo, hi, P, Q, split_run<V4>(lo, hi, P, Q, split_run<VT>(lo, hi, P, Q, 0, dw), dw), dw);
 }
}
// split の逆と、c_t x^{dt} の重ね合わせと、mod x^{2D} + x^D + 1 を 1 度に。hi = P + Q、lo = P + x^d hi (x^d + x^{2d} = 1)。
// c_t = lo_t (t < δ)、hi_{t-δ} (t >= δ) を chunk t に c_t.L、chunk t+1 に c_t.H と足し、はみ出す chunk 2δ は chunk 0 と chunk δ に畳む。
// i の小さい順に進めると、どの chunk も最初に触るときが代入になるので、出力を 0 で埋めずに済む (S0..S3 が代入か XOR か)。
template <class T, bool S0, bool S1, bool S2, bool S3> inline int merge_run(const u64* P, const u64* Q, u64* c0, u64* c1, u64* c2, u64* c3, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T pL= ld<T>(P + j), pH= ld<T>(P + dw + j), hL= pL ^ ld<T>(Q + j), hH= pH ^ ld<T>(Q + dw + j), v0= pL ^ hH, v1= pH ^ hL ^ hH;
  st<T>(c0 + j, S0 ? v0 : ld<T>(c0 + j) ^ v0), st<T>(c1 + j, S1 ? v1 : ld<T>(c1 + j) ^ v1), st<T>(c2 + j, S2 ? hL : ld<T>(c2 + j) ^ hL), st<T>(c3 + j, S3 ? hH : ld<T>(c3 + j) ^ hH);
 }
 return j;
}
// 最後の i: chunk δ-1 に lo.L、chunk δ に lo.H + hi.H (はみ出しの分)、chunk 2δ-1 に hi.L、chunk 0 に hi.H を足す。
template <class T> inline int merge_last(const u64* P, const u64* Q, u64* c0, u64* cd, u64* c2, u64* cz, int j, int dw) {
 constexpr int L= sizeof(T) / 8;
 for(; j + L <= dw; j+= L) {
  const T pL= ld<T>(P + j), pH= ld<T>(P + dw + j), hL= pL ^ ld<T>(Q + j), hH= pH ^ ld<T>(Q + dw + j);
  st<T>(c0 + j, ld<T>(c0 + j) ^ pL ^ hH), st<T>(cd + j, ld<T>(cd + j) ^ pH ^ hL), st<T>(c2 + j, ld<T>(c2 + j) ^ hL), st<T>(cz + j, ld<T>(cz + j) ^ hH);
 }
 return j;
}
template <bool S0, bool S1, bool S2, bool S3> inline void merge_one(const u64* P, const u64* Q, u64* c0, u64* c1, u64* c2, u64* c3, int dw) {
 merge_run<u64, S0, S1, S2, S3>(P, Q, c0, c1, c2, c3, merge_run<V4, S0, S1, S2, S3>(P, Q, c0, c1, c2, c3, merge_run<VT, S0, S1, S2, S3>(P, Q, c0, c1, c2, c3, 0, dw), dw), dw);
}
inline void merge(const u64* A, u64* c, int d, int W, int dl) {
 const int dw= d * W, sz= 2 * dw;
 const size_t Dw= (size_t)dw * dl;
 for(int i= 0; i < dl; ++i) {
  const u64 *P= A + (size_t)i * sz, *Q= A + (size_t)(dl + i) * sz;
  u64 *c0= c + (size_t)i * dw, *c1= c0 + dw, *c2= c + Dw + (size_t)i * dw, *c3= c2 + dw;
  if(i + 1 == dl) merge_last<u64>(P, Q, c0, c1, c2, c, merge_last<V4>(P, Q, c0, c1, c2, c, merge_last<VT>(P, Q, c0, c1, c2, c, 0, dw), dw), dw);
  else if(i == 0) merge_one<true, true, true, true>(P, Q, c0, c1, c2, c3, dw);
  else merge_one<false, true, false, true>(P, Q, c0, c1, c2, c3, dw);
 }
}
// ---------------- Schönhage の再帰 ----------------
constexpr int BK= 3;  // R_27 で base
// R_D (D = 3^k) を R_d (d = 3^kd) の 2δ 個の積に割る (δ = 3^(k - kd))。再帰が必ず k = BK で止まるよう kd >= BK にする (3^12 → 3^6 → 3^3)。
inline int kd_of(int k) { return std::max((k + 1) / 2, BK); }
inline size_t ws_size(int k, int W) {
 if(k <= BK) return 0;
 const int kd= kd_of(k), d= pow3(kd), D= pow3(k);
 return ((size_t)12 * D + 66 * d) * W + ws_size(kd, W);
}
// a (n 個) を変換して x に置く (y と T と tmp は作業用)。読み書きの回数が奇数なら y から始め、終わりが x に来るようにする。
inline void transform(const u64* a, size_t n, u64* x, u64* y, int d, int W, int dl, u64* T, u64* tmp) {
 const size_t sz= (size_t)2 * d * W;
 u64 *s= passes(dl) & 1 ? y : x, *o= passes(dl) & 1 ? x : y;
 split(a, n, s, d, W, dl, tmp);
 fwd(s, o, d, W, dl, d, T), fwd(s + dl * sz, o + dl * sz, d, W, dl, 2 * d, T);
}
// c = a b in R_D (D = 3^k)、W 本ずつ lane-sliced。c は a と同じ場所でもよい。
template <class P> inline void mul_R(int k, const u64* a, const u64* b, u64* c, u64* ws) {
 constexpr int W= P::W;
 if(k <= BK) return base27<P>(a, b, c);
 const int kd= kd_of(k), d= pow3(kd), dl= pow3(k - kd), sz= 2 * d * W;
 const size_t tot= (size_t)2 * dl * sz, D2= (size_t)2 * d * dl * W;
 u64 *A= ws, *Bv= ws + tot, *S= Bv + tot, *T= S + tot, *tmp= T + 29 * sz, *sub= tmp + 2 * sz;
 transform(a, D2, A, S, d, W, dl, T, tmp), transform(b, D2, Bv, S, d, W, dl, T, tmp);
 for(int i= 0; i < 2 * dl; ++i) mul_R<P>(kd, A + (size_t)i * sz, Bv + (size_t)i * sz, A + (size_t)i * sz, sub);
 inv(A, d, W, dl, d, T, tmp), inv(A + (size_t)dl * sz, d, W, dl, 2 * d, T, tmp);
 merge(A, c, d, W, dl);
}
// 一番上の最初の 2 段を、入力の chunk から直接読む形で (split を書かない)。入力の上半分が 0 (n, m <= D) なら P_i = Q_i = (a_i, 0) なので、
// ring element を下半分 L と上半分 H の 2 つのポインタで渡し、H は 0 の列を指す。区間ごとの読み出しは L か H の片方に収まる。
template <int TB, int TC> inline void fwd_seg_lh(const u64* A0L, const u64* A0H, const u64* A1L, const u64* A1H, const u64* A2L, const u64* A2H, u64* O0, u64* O1, u64* O2, int j0, int j1, int dw, const Seg& sb, const Seg& sc) {
 auto at= [&](const u64* L, const u64* H, int o) { return o + j0 >= dw ? H + (o - dw) : L + o; };
 const u64 *bu= at(A1L, A1H, sb.xo), *bv= at(A1L, A1H, sb.yo), *cu= at(A2L, A2H, sc.xo), *cv= at(A2L, A2H, sc.yo);
 u64 *o0H= O0 + dw, *o1H= O1 + dw, *o2H= O2 + dw;
 const int j= fwd_run<V4, TB, TC>(A0L, A0H, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, fwd_run<VT, TB, TC>(A0L, A0H, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j0, j1), j1);
 fwd_run<u64, TB, TC>(A0L, A0H, bu, bv, cu, cv, O0, o0H, O1, o1H, O2, o2H, j, j1);
}
using FwdLHFn= void (*)(const u64*, const u64*, const u64*, const u64*, const u64*, const u64*, u64*, u64*, u64*, int, int, int, const Seg&, const Seg&);
constexpr FwdLHFn FWDLH[3][3]= {{fwd_seg_lh<0, 0>, fwd_seg_lh<0, 1>, fwd_seg_lh<0, 2>}, {fwd_seg_lh<1, 0>, fwd_seg_lh<1, 1>, fwd_seg_lh<1, 2>}, {fwd_seg_lh<2, 0>, fwd_seg_lh<2, 1>, fwd_seg_lh<2, 2>}};
// src の chunk k (d 個) を L に、0 の列 Z を H にした ring element を 3 つ取り、butterfly を 1 つ。n 個目より先は 0 (chunk をまたぐものは pc に写す)。
struct Chunks {
 const u64* src;
 size_t n;
 int d;
 const u64* Z;
 u64* pc;
 const u64* get(size_t k) const {
  const size_t b= k * d;
  if(b + d <= n) return src + b;
  if(b >= n) return Z;
  return pc;
 }
};
// 一番上の最初の 3 段 (radix-27 の 1 回目) を、入力の chunk から直接読む形で。1 段目の読み出しだけが fwd27 と違う。一番上が 27 要素以上のときだけ使う。
inline void fwd27_first(const Chunks& in, u64* dst, int d, int cnt, int E, u64* T, u64* tmp) {
 const size_t m= cnt / 27, dw= d, sz= 2 * dw;
 const Shift2 s1(d, 1, E);
 const Tw27 w(d, 1, E);
 for(size_t i= 0; i < m; ++i) {
  for(int j= 0; j < 9; ++j) {
   const u64 *L0= in.get(i + m * j), *L1= in.get(i + m * (9 + j)), *L2= in.get(i + m * (18 + j));
   u64 *O0= T + j * sz, *O1= T + (9 + j) * sz, *O2= T + (18 + j) * sz;
   for(int x= 0; x < s1.nb; ++x)
    for(int y= 0; y < s1.nc; ++y) {
     const int j0= std::max(s1.sb[x].j0, s1.sc[y].j0), j1= std::min(s1.sb[x].j1, s1.sc[y].j1);
     if(j0 < j1) FWDLH[s1.sb[x].ty][s1.sc[y].ty](L0, in.Z, L1, in.Z, L2, in.Z, O0, O1, O2, j0, j1, dw, s1.sb[x], s1.sc[y]);
    }
  }
  fwd27_23(T, dst, i, m, d, 1, w, tmp);
 }
}
// a (n 個、n <= D) を変換して x に置く。最初の 3 段は入力から直接読み、残りの段の回数の偶奇で最初の書き先を x か y に決める。
inline void transform_first(const u64* a, size_t n, u64* x, u64* y, int d, int dl, u64* T, u64* Z, u64* pc) {
 const size_t sz= (size_t)2 * d, m= dl / 27;
 if(n % d) std::fill(pc, pc + d, 0), std::copy(a + n / d * d, a + n, pc);
 const Chunks in{a, n, d, Z, pc};
 u64 *s= passes(m) & 1 ? y : x, *o= passes(m) & 1 ? x : y;
 for(int h= 0; h < 2; ++h) {
  const int E= (h + 1) * d;
  u64 *sh= s + h * dl * sz, *oh= o + h * dl * sz;
  fwd27_first(in, sh, d, dl, E, T, T + 27 * sz);
  for(int t= 0; t < 27; ++t) fwd(sh + t * m * sz, oh + t * m * sz, d, 1, m, ((E / 3 + d * (t / 9)) / 3 + d * (t / 3 % 3)) / 3 + d * (t % 3), T);
 }
}
// 一番上: 2 つの入力をそれぞれ最後まで変換し、各点の積を W 本ずつ lane-sliced に並べ替えて掛け、全体を逆変換する。
template <class P> inline void mul_top(int k, const u64* a, size_t n, const u64* b, size_t m, u64* c) {
 constexpr int W= P::W;
 const int kd= kd_of(k), d= pow3(kd), dl= pow3(k - kd);
 const size_t sz= (size_t)2 * d, tot= 2 * dl * sz;
 std::unique_ptr<u64[]> buf(new u64[3 * tot + 31 * sz + 2 * sz * W + ws_size(kd, W)]);  // 0 で埋めない (どこも書いてから読む)
 u64 *A= buf.get(), *Bv= A + tot, *S= Bv + tot, *T= S + tot, *tmp= T + 29 * sz, *LA= tmp + 2 * sz, *LB= LA + sz * W, *sub= LB + sz * W;
 const size_t D= (size_t)d * dl;
 if(n <= D && m <= D && dl >= 27) {
  u64 *Z= LA, *pc= LB;  // LA, LB はまだ使わないので、0 の列と chunk の写しに借りる
  std::fill(Z, Z + d, 0);
  transform_first(a, n, A, S, d, dl, T, Z, pc), transform_first(b, m, Bv, S, d, dl, T, Z, pc);
 } else transform(a, n, A, S, d, 1, dl, T, tmp), transform(b, m, Bv, S, d, 1, dl, T, tmp);
 for(int g= 0; g < 2 * dl; g+= W) {
  const int w= std::min(W, 2 * dl - g);
  u64 *ea[W], *eb[W];
  for(int l= 0; l < w; ++l) ea[l]= A + (g + l) * sz, eb[l]= Bv + (g + l) * sz;
  to_lanes<W>(ea, w, LA, sz), to_lanes<W>(eb, w, LB, sz);
  mul_R<P>(kd, LA, LB, LA, sub);
  from_lanes<W>(LA, ea, w, sz);
 }
 inv(A, d, 1, dl, d, T, tmp), inv(A + dl * sz, d, 1, dl, 2 * d, T, tmp);
 merge(A, c, d, 1, dl);
}
template <class P> inline std::vector<u64> convolve(const std::vector<u64>& a, const std::vector<u64>& b) {
 const int n= a.size(), m= b.size();
 if(!n || !m) return {};
 const int N= n + m - 1;
 if(N <= 2 * pow3(BK + 1)) return naive(a, b);
 int k= BK + 2;
 while(2 * pow3(k) < N) ++k;
 std::vector<u64> c(2 * pow3(k));
 mul_top<P>(k, a.data(), n, b.data(), m, c.data());
 c.resize(N);
 return c;
}
}  // namespace schoenhage3z
#ifdef __clang__
#pragma clang attribute pop
#else
#pragma GCC pop_options
#endif
#endif
// AVX-512 (F, BW, DQ, VL) と VPCLMULQDQ があれば 8 本ずつの版、無ければ 4 本ずつの版。arm は 2 本ずつ。
inline std::vector<u64> run(int, int, const std::vector<u64>& a, const std::vector<u64>& b) {
#ifdef __x86_64__
 if(__builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512bw") && __builtin_cpu_supports("avx512dq") && __builtin_cpu_supports("avx512vl") && __builtin_cpu_supports("vpclmulqdq")) return schoenhage3z::convolve<schoenhage3z::P8>(a, b);
 if(__builtin_cpu_supports("vpclmulqdq")) return schoenhage3::convolve<schoenhage3::P4<1>>(a, b);
 return schoenhage3::convolve<schoenhage3::P4<0>>(a, b);
#else
 return schoenhage3::convolve<schoenhage3::P2>(a, b);
#endif
}
