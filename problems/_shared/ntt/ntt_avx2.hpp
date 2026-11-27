#pragma once
// 単独の NTT の核の 3 本目 (実験版)。AVX2 の 8 レーン、Montgomery、基数 4、ひねりは率で進める。
// 口と並びは 2 本目 (ntt_scalar_r4.hpp) と同じ。dft の入口で Montgomery に直し、idft の出口で 1/n を掛けながら戻す。
// 値は 2P 未満で持ち、掛け算の入力は 4P 未満まで許す (Montgomery の REDC の結果は 2P 未満)。
// 長さ 32 以上の段は 8 レーン、16 以下の段 (h = len/4 が 4 以下) と最後の基数 2 の段はスカラーで処理する。
// AVX2 の intrinsics か SIMDe のどちらかが要る。SIMDe のときは SIMDE_ENABLE_NATIVE_ALIASES も定義すること。
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/naive.hpp"
#include "_shared/ntt/_kernel_base.hpp"

#if defined(USE_SIMDE)
#include <simde/x86/avx2.h>
#if !defined(SIMDE_ENABLE_NATIVE_ALIASES)
#error "USE_SIMDE のときは SIMDE_ENABLE_NATIVE_ALIASES も定義してください"
#endif
#elif defined(__AVX2__)
#include <immintrin.h>
#else
#error "AVX2 の intrinsics か SIMDe が要ります (-march=x86-64-v3 か -DUSE_SIMDE)"
#endif

namespace ntt_avx2_internal {
using u32= unsigned;

// 小さい段をまとめて回すタイルの長さ (2 の冪)。4 KB から 8 KB くらい。
constexpr int dft_tile= 2048;
using u64= unsigned long long;
using V= __m256i;

// n は奇数。n^{-1} mod 2^32。
constexpr u32 inv_mod2_32(u32 n) {
 u32 x= 1;
 for (int i= 0; i < 5; ++i) x*= 2 - n * x;
 return x;
}
template <unsigned P> constexpr u32 pow2_32_mod() {
 u64 r= 1;
 for (int i= 0; i < 32; ++i) r= r * 2 % P;
 return u32(r);
}

template <unsigned P> struct mc {
 static constexpr u32 p= P, p2= 2 * P;
 static constexpr u32 ni= u32(0) - inv_mod2_32(P);  // -P^{-1} mod 2^32
 static constexpr u32 r= pow2_32_mod<P>();          // 2^32 mod P
 static constexpr u32 r2= u32(u64(r) * r % P);      // 2^64 mod P
};

template <unsigned P> u32 spow(u32 a, u64 e) {  // ふつうの値のべき (a < P)
 u64 r= 1, b= a;
 for (; e; e>>= 1, b= b * b % P)
  if (e & 1) r= r * b % P;
 return u32(r);
}

// Montgomery の掛け算 (スカラー)。a < 4P、b < P で、結果は P 未満。
template <unsigned P> constexpr u32 smul(u32 a, u32 b) {
 constexpr u32 ni= mc<P>::ni;
 u64 x= u64(a) * b;
 u64 z= (x + u64(u32(x) * ni) * P) >> 32;
 return u32(z >= P ? z - P : z);
}
// ふつうの値を Montgomery に直す (a < P でなくてもよい。a < 4P で結果 < P)。
template <unsigned P> u32 smont(u32 a) { return smul<P>(a, mc<P>::r2); }

template <unsigned P> u32 slow(u32 x) { return x >= mc<P>::p2 ? x - mc<P>::p2 : x; }
template <unsigned P> u32 sdiff(u32 x, u32 y) { return x + mc<P>::p2 - y; }

template <unsigned P> V vsplat(u32 x) { return _mm256_set1_epi32(int(x)); }
template <unsigned P> V vload(const u32* p) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p)); }
template <unsigned P> void vstore(u32* p, V x) { _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), x); }
template <unsigned P> __m128i vload128(const u32* p) { return _mm_loadu_si128(reinterpret_cast<const __m128i*>(p)); }
template <unsigned P> __m128i vsplat128(u32 x) { return _mm_set1_epi32(int(x)); }
// 128 bit を 2 本つないで 1 本の 8 レーンにする。
template <unsigned P> V vcombine128(__m128i lo, __m128i hi) { return _mm256_inserti128_si256(_mm256_castsi128_si256(lo), hi, 1); }
template <unsigned P> V vplus(V x, V y) { return _mm256_add_epi32(x, y); }
template <unsigned P> V vdiff(V x, V y) { return _mm256_sub_epi32(_mm256_add_epi32(x, vsplat<P>(mc<P>::p2)), y); }
template <unsigned P> V vlow(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, vsplat<P>(mc<P>::p2))); }

// Montgomery の掛け算 (8 レーン)。x < 4P、w < P。結果は 2P 未満。
// 積 t = x w の下位 32 bit から q = t * ni mod 2^32 を作り、t + q P の上位 32 bit を取る。
// 偶数レーンは t を右に 32 ずらし、奇数レーンは o の上位 32 bit がもともと奇数レーンにあるので混ぜる。
// 奇数レーンは x と w を右に 32 ずらしてから掛ける (ずらさないと偶数レーンの値と混ざる)。
// 32 bit の下位だけの積 (mullo_epi32) は使わない。SIMDe が符号付きで掛けて UBSan に鳴るため。
template <unsigned P> V vmul(V x, V w) {
 const V pv= vsplat<P>(P), niv= vsplat<P>(mc<P>::ni);
 const V xo= _mm256_srli_epi64(x, 32), wo= _mm256_srli_epi64(w, 32);
 const V t= _mm256_mul_epu32(x, w), to= _mm256_mul_epu32(xo, wo);
 const V qe= _mm256_mul_epu32(t, niv), qo= _mm256_mul_epu32(to, niv);
 const V e= _mm256_add_epi64(t, _mm256_mul_epu32(qe, pv));
 const V o= _mm256_add_epi64(to, _mm256_mul_epu32(qo, pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xAA);
}
// w が全レーン同じとき (率や 4 乗根)。右にずらすのが 1 つ減る。
template <unsigned P> V vmuls(V x, V w) {
 const V pv= vsplat<P>(P), niv= vsplat<P>(mc<P>::ni);
 const V xo= _mm256_srli_epi64(x, 32);
 const V t= _mm256_mul_epu32(x, w), to= _mm256_mul_epu32(xo, w);
 const V qe= _mm256_mul_epu32(t, niv), qo= _mm256_mul_epu32(to, niv);
 const V e= _mm256_add_epi64(t, _mm256_mul_epu32(qe, pv));
 const V o= _mm256_add_epi64(to, _mm256_mul_epu32(qo, pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xAA);
}

// mod は素数。mod - 1 の素因数を集め、どの p でも g^((mod-1)/p) != 1 となる最小の g を返す。
template <unsigned P> const u32& primitive_root() {
 static const u32 g= [] {
  u32 n= P - 1;
  std::vector<u32> ps;
  for (u32 p= 2; u64(p) * p <= n; ++p) {
   if (n % p) continue;
   ps.push_back(p);
   while (n % p == 0) n/= p;
  }
  if (n > 1) ps.push_back(n);
  for (u32 g= 2;; ++g) {
   bool ok= true;
   for (u32 p: ps)
    if (spow<P>(g, (P - 1) / p) == 1) {
     ok= false;
     break;
    }
   if (ok) return g;
  }
 }();
 return g;
}

// 1 の原始 4 乗根 (順) とその逆。段ごとに作り直さない。
template <unsigned P> const V& im_vec() {
 static const V v= vsplat<P>(smont<P>(spow<P>(primitive_root<P>(), u64(P - 1) / 4)));
 return v;
}
template <unsigned P> const V& ii_vec() {
 static const V v= vsplat<P>(smont<P>(spow<P>(primitive_root<P>(), u64(P - 1) - u64(P - 1) / 4)));
 return v;
}
// n^{-1} mod P。n = 2^e ごとに残す。
template <unsigned P> u32 inv_n_scalar(int n) {
 static u32 tab[26]= {};
 const int e= __builtin_ctz(unsigned(n));
 if (!tab[e]) tab[e]= spow<P>(unsigned(n), u64(P - 2));
 return tab[e];
}

// 段のひねりの表。長さ len の段では h = len/4 として、tw[j] = ω_len^j、tw[h + j] = ω_len^{2j}、tw[2h + j] = ω_len^{3j}。
// 同じ段のすべてのブロックで使い回す。ベクトルで作る (率を掛けて 8 個ずつ進める)。
// 長さと向きごとに 1 回だけ作って残す (小さい n を何度も変換するときの作り直しを避ける)。
template <unsigned P> struct scratch {
 static inline std::vector<u32> tw[2][26];
};
// inv = false で順変換 (ω_len^j)、true で逆変換 (ω_len^{-j})。
template <unsigned P> const u32* build_tw(int len, int h, bool inv) {
 auto& tab= scratch<P>::tw[inv ? 1 : 0][__builtin_ctz(unsigned(len))];
 if ((int)tab.size() == 3 * h) return tab.data();
 tab.assign(3 * h, 0);
 const u32 g= primitive_root<P>();
 const u32 wl= smont<P>(spow<P>(g, inv ? u64(P - 1) - u64(P - 1) / unsigned(len) : u64(P - 1) / unsigned(len)));
 const u32 c2= smul<P>(wl, wl), c3= smul<P>(c2, wl);
 u32 a1[8], a2[8], a3[8];
 a1[0]= a2[0]= a3[0]= mc<P>::r;
 for (int j= 1; j < 8; ++j) a1[j]= smul<P>(a1[j - 1], wl), a2[j]= smul<P>(a2[j - 1], c2), a3[j]= smul<P>(a3[j - 1], c3);
 V w1= vload<P>(a1), w2= vload<P>(a2), w3= vload<P>(a3);
 const V r1= vsplat<P>(smul<P>(a1[7], wl));
 const V r2= vsplat<P>(smul<P>(a2[7], c2));
 const V r3= vsplat<P>(smul<P>(a3[7], c3));
 u32* p1= tab.data();
 u32* p2= p1 + h;
 u32* p3= p2 + h;
 for (int j= 0; j < h; j+= 8) {
  vstore<P>(p1 + j, w1), vstore<P>(p2 + j, w2), vstore<P>(p3 + j, w3);
  w1= vmuls<P>(w1, r1), w2= vmuls<P>(w2, r2), w3= vmuls<P>(w3, r3);
 }
 return tab.data();
}

// 順変換の 1 段 (len の段、h = len/4 が 8 以上)。
template <unsigned P> void dft_stage_vec(u32* d, int n, int len) {
 const int h= len>> 2, hv= len>> 5;
 const u32* tab= build_tw<P>(len, h, false);
 const u32* t1= tab;
 const u32* t2= tab + h;
 const u32* t3= tab + 2 * h;
 const V& im= im_vec<P>();
 for (int b= 0; b < n; b+= len) {
  u32* q= d + b;
  for (int jv= 0; jv < hv; ++jv) {
   u32* p= q + (jv << 3);
   const int j= jv << 3;
   const V w1= vload<P>(t1 + j), w2= vload<P>(t2 + j), w3= vload<P>(t3 + j);
   const V u0= vload<P>(p), u1= vload<P>(p + h), u2= vload<P>(p + 2 * h), u3= vload<P>(p + 3 * h);
   const V t0= vplus<P>(u0, u2), t2v= vplus<P>(u1, u3);
   const V t1v= vdiff<P>(u0, u2), t3v= vdiff<P>(u1, u3);
   const V l0= vlow<P>(t0), l1= vlow<P>(t1v), l2= vlow<P>(t2v);
   const V pi= vmuls<P>(t3v, im);
   const V s1= vplus<P>(l1, pi), s3= vdiff<P>(l1, pi);
   vstore<P>(p, vlow<P>(vplus<P>(l0, l2)));
   vstore<P>(p + h, vmul<P>(s1, w1));
   vstore<P>(p + 2 * h, vmul<P>(vdiff<P>(l0, l2), w2));
   vstore<P>(p + 3 * h, vmul<P>(s3, w3));
  }
 }
}

// 逆変換の 1 段 (len の段、h = len/4 が 8 以上)。
template <unsigned P> void idft_stage_vec(u32* d, int n, int len) {
 const int h= len>> 2, hv= len>> 5;
 const u32* tab= build_tw<P>(len, h, true);
 const u32* t1= tab;
 const u32* t2= tab + h;
 const u32* t3= tab + 2 * h;
 const V& ii= ii_vec<P>();
 for (int b= 0; b < n; b+= len) {
  u32* q= d + b;
  for (int jv= 0; jv < hv; ++jv) {
   u32* p= q + (jv << 3);
   const int j= jv << 3;
   const V w1= vload<P>(t1 + j), w2= vload<P>(t2 + j), w3= vload<P>(t3 + j);
   const V u0= vload<P>(p);
   const V y1= vmul<P>(vload<P>(p + h), w1);
   const V y2= vmul<P>(vload<P>(p + 2 * h), w2);
   const V y3= vmul<P>(vload<P>(p + 3 * h), w3);
   const V t0= vlow<P>(vplus<P>(u0, y2)), t2v= vlow<P>(vdiff<P>(u0, y2));
   const V t1v= vlow<P>(vplus<P>(y1, y3)), t3v= vmuls<P>(vdiff<P>(y1, y3), ii);
   vstore<P>(p, vlow<P>(vplus<P>(t0, t1v)));
   vstore<P>(p + h, vlow<P>(vplus<P>(t2v, t3v)));
   vstore<P>(p + 2 * h, vlow<P>(vdiff<P>(t0, t1v)));
   vstore<P>(p + 3 * h, vlow<P>(vdiff<P>(t2v, t3v)));
  }
 }
}

// 最後の基数 2 の段 (h = 1)。8 個 (4 対) ずつ。順変換にも逆変換にも使える (1/2 は掛けない)。
template <unsigned P> void dft_stage_r2(u32* d, int n) {
 int i= 0;
 for (; i + 8 <= n; i+= 8) {
  const V v= vload<P>(d + i);
  const V w= _mm256_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 0, 1));  // [a1,a0,a3,a2]
  const V s= vlow<P>(vplus<P>(v, w));                          // 偶数レーン: a+b
  const V t= vlow<P>(vdiff<P>(w, v));                          // 奇数レーン: a-b
  vstore<P>(d + i, _mm256_blend_epi32(s, t, 0xAA));
 }
 for (; i < n; i+= 2) {
  const u32 u= d[i], v= d[i + 1];
  d[i]= slow<P>(u + v), d[i + 1]= slow<P>(sdiff<P>(u, v));
 }
}

// len = 4 の段 (h = 1)。2 ブロック (8 個) ずつ。128 bit レーンの中の 4 個で DFT4 をする。
// s = [u0+u2, u1+u3]、dd = [u0-u2, u1-u3] として、out0 = s0+s1、out2 = s0-s1、out1 = dd0+i dd1、out3 = dd0-i dd1 (ひねりは 1)。
// 値はレーン 0 か全レーンにできるので、最後にレーン 0〜3 へ並べ直す。順変換。
template <unsigned P> void dft_stage_h1(u32* d, int n) {
 const V& im= im_vec<P>();
 for (int b= 0; b < n; b+= 8) {
  const V v= vload<P>(d + b);
  const V sw= _mm256_shuffle_epi32(v, _MM_SHUFFLE(1, 0, 3, 2));  // [e2,e3,e0,e1]
  const V s= vlow<P>(vplus<P>(v, sw));                          // [t0,t1,t0,t1] (t0 = e0+e2、t1 = e1+e3)
  const V dd= vlow<P>(vdiff<P>(v, sw));                         // [t2,t3,-t2,-t3] (t2 = e0-e2、t3 = e1-e3)
  const V p= _mm256_shuffle_epi32(s, _MM_SHUFFLE(2, 3, 0, 1));   // [t1,t0,t1,t0]
  const V q= _mm256_shuffle_epi32(dd, _MM_SHUFFLE(2, 3, 0, 1));  // [t3,t2,t3,t2]
  const V iq= vmuls<P>(q, im);                                  // [i t3, i t2, ..]
  const V a0= vlow<P>(vplus<P>(s, p));                          // 全レーン out0 = t0+t1
  const V a1= _mm256_shuffle_epi32(vlow<P>(vplus<P>(dd, iq)), _MM_SHUFFLE(0, 0, 0, 0));   // out1 = t2+i t3
  const V a2= _mm256_shuffle_epi32(vlow<P>(vdiff<P>(s, p)), _MM_SHUFFLE(0, 0, 0, 0));      // out2 = t0-t1
  const V a3= _mm256_shuffle_epi32(vlow<P>(vdiff<P>(dd, iq)), _MM_SHUFFLE(0, 0, 0, 0));    // out3 = t2-i t3
  vstore<P>(d + b, _mm256_blend_epi32(_mm256_blend_epi32(a0, a1, 0x22), _mm256_blend_epi32(a2, a3, 0x88), 0xCC));
 }
}

// len = 4 の段 (h = 1)。逆変換。
template <unsigned P> void idft_stage_h1(u32* d, int n) {
 const V& ii= ii_vec<P>();
 for (int b= 0; b < n; b+= 8) {
  const V v= vload<P>(d + b);
  const V sw= _mm256_shuffle_epi32(v, _MM_SHUFFLE(1, 0, 3, 2));  // [e2,e3,e0,e1]
  const V s= vlow<P>(vplus<P>(v, sw));                          // [t0,t1,t0,t1]
  const V dd= vlow<P>(vdiff<P>(v, sw));                         // [t2,t3,-t2,-t3]
  const V p= _mm256_shuffle_epi32(s, _MM_SHUFFLE(2, 3, 0, 1));   // [t1,t0,t1,t0]
  const V q= _mm256_shuffle_epi32(dd, _MM_SHUFFLE(2, 3, 0, 1));  // [t3,t2,t3,t2]
  const V iq= vmuls<P>(q, ii);                                  // [ii t3, ii t2, ..]
  const V a0= vlow<P>(vplus<P>(s, p));                          // 全レーン x0 = t0+t1
  const V a1= _mm256_shuffle_epi32(vlow<P>(vplus<P>(dd, iq)), _MM_SHUFFLE(0, 0, 0, 0));   // x1 = t2+ii t3
  const V a2= _mm256_shuffle_epi32(vlow<P>(vdiff<P>(s, p)), _MM_SHUFFLE(0, 0, 0, 0));      // x2 = t0-t1
  const V a3= _mm256_shuffle_epi32(vlow<P>(vdiff<P>(dd, iq)), _MM_SHUFFLE(0, 0, 0, 0));    // x3 = t2-ii t3
  vstore<P>(d + b, _mm256_blend_epi32(_mm256_blend_epi32(a0, a1, 0x22), _mm256_blend_epi32(a2, a3, 0x88), 0xCC));
 }
}

// 小さい段 (h <= 4) のひねり (Montgomery) と、i / i^{-1} のスカラー版。長さと向きごとに 1 回だけ作る。
template <unsigned P> struct small_cache {
 static inline u32 a[2][6][3][4]= {};
 static inline bool built[2][6]= {};
 static inline u32 i4[2]= {0, 0};
 static inline bool built_i= false;
};
template <unsigned P> const u32* small_tw(int len, int h, bool inv) {
 using C= small_cache<P>;
 const int e= __builtin_ctz(unsigned(len));
 if (!C::built[inv ? 1 : 0][e]) {
  const u32 g= primitive_root<P>();
  const u32 wl= smont<P>(spow<P>(g, inv ? u64(P - 1) - u64(P - 1) / unsigned(len) : u64(P - 1) / unsigned(len)));
  const u32 c2= smul<P>(wl, wl), c3= smul<P>(c2, wl);
  u32* t= &C::a[inv ? 1 : 0][e][0][0];
  u32* a1= t;
  u32* a2= t + 4;
  u32* a3= t + 8;
  a1[0]= a2[0]= a3[0]= mc<P>::r;
  for (int j= 1; j < 4; ++j) a1[j]= smul<P>(a1[j - 1], wl), a2[j]= smul<P>(a2[j - 1], c2), a3[j]= smul<P>(a3[j - 1], c3);
  C::built[inv ? 1 : 0][e]= true;
 }
 (void)h;
 return &C::a[inv ? 1 : 0][e][0][0];
}
template <unsigned P> u32 small_i(bool inv_i) {
 using C= small_cache<P>;
 if (!C::built_i) {
  const u32 g= primitive_root<P>();
  C::i4[0]= smont<P>(spow<P>(g, u64(P - 1) / 4));
  C::i4[1]= smont<P>(spow<P>(g, u64(P - 1) - u64(P - 1) / 4));
  C::built_i= true;
 }
 return C::i4[inv_i ? 1 : 0];
}

// h = len/4 が 4 以下の段 (スカラー)。順変換。
template <unsigned P> void dft_stage_small(u32* d, int n, int len) {
 const int h= len>> 2;
 const u32* tw= small_tw<P>(len, h, false);
 const u32* a1= tw;
 const u32* a2= tw + 4;
 const u32* a3= tw + 8;
 const u32 im= small_i<P>(false);
 for (int b= 0; b < n; b+= len) {
  u32* q= d + b;
  for (int j= 0; j < h; ++j) {
   const u32 u0= q[j], u1= q[j + h], u2= q[j + 2 * h], u3= q[j + 3 * h];
   const u32 t0= u0 + u2, t2= u1 + u3;
   const u32 t1= sdiff<P>(u0, u2), t3= sdiff<P>(u1, u3);
   const u32 l0= slow<P>(t0), l1= slow<P>(t1), l2= slow<P>(t2);
   const u32 pi= smul<P>(t3, im);
   const u32 s1= l1 + pi, s3= sdiff<P>(l1, pi);
   q[j]= slow<P>(l0 + l2);
   q[j + h]= smul<P>(s1, a1[j]);
   q[j + 2 * h]= smul<P>(sdiff<P>(l0, l2), a2[j]);
   q[j + 3 * h]= smul<P>(s3, a3[j]);
  }
 }
}

// h = len/4 が 4 以下の段 (スカラー)。逆変換。
template <unsigned P> void idft_stage_small(u32* d, int n, int len) {
 const int h= len>> 2;
 const u32* tw= small_tw<P>(len, h, true);
 const u32* a1= tw;
 const u32* a2= tw + 4;
 const u32* a3= tw + 8;
 const u32 ii= small_i<P>(true);
 for (int b= 0; b < n; b+= len) {
  u32* q= d + b;
  for (int j= 0; j < h; ++j) {
   const u32 u0= q[j];
   const u32 y1= smul<P>(q[j + h], a1[j]), y2= smul<P>(q[j + 2 * h], a2[j]), y3= smul<P>(q[j + 3 * h], a3[j]);
   const u32 t0= slow<P>(u0 + y2), t2= slow<P>(sdiff<P>(u0, y2));
   const u32 t1= slow<P>(y1 + y3), t3= smul<P>(sdiff<P>(y1, y3), ii);
   q[j]= slow<P>(t0 + t1);
   q[j + h]= slow<P>(t2 + t3);
   q[j + 2 * h]= slow<P>(sdiff<P>(t0, t1));
   q[j + 3 * h]= slow<P>(sdiff<P>(t2, t3));
  }
 }
}

// len = 16 の段 (h = 4)。2 ブロック (32 個) ずつ。1 ブロックは 8 レーン 2 本 ([u0|u1] と [u2|u3]) なので、
// 2 ブロック分を 128 bit ごとに組み替えて、u0+u2 などを [b0|b1] の 8 レーンにする。順変換。
// 1 ブロックだけのタイル (n = 16) はスカラーに回す。
template <unsigned P> void dft_stage_h4(u32* d, int n) {
 if (n < 32) {
  dft_stage_small<P>(d, n, 16);
  return;
 }
 const u32* tw= small_tw<P>(16, 4, false);
 const V w1= vcombine128<P>(vload128<P>(tw), vload128<P>(tw));
 const V w2= vcombine128<P>(vload128<P>(tw + 4), vload128<P>(tw + 4));
 const V w3= vcombine128<P>(vload128<P>(tw + 8), vload128<P>(tw + 8));
 const V& im= im_vec<P>();
 for (int b= 0; b + 32 <= n; b+= 32) {
  u32* q= d + b;
  const V x0= vload<P>(q), x1= vload<P>(q + 8), x2= vload<P>(q + 16), x3= vload<P>(q + 24);
  const V s0= vplus<P>(x0, x1), s1= vplus<P>(x2, x3);
  const V d0= vdiff<P>(x0, x1), d1= vdiff<P>(x2, x3);
  const V t0= vlow<P>(_mm256_permute2x128_si256(s0, s1, 0x20));  // u0+u2 (b0 | b1)
  const V t2= vlow<P>(_mm256_permute2x128_si256(s0, s1, 0x31));  // u1+u3
  const V t1= vlow<P>(_mm256_permute2x128_si256(d0, d1, 0x20));  // u0-u2
  const V t3= _mm256_permute2x128_si256(d0, d1, 0x31);           // u1-u3
  const V pi= vmuls<P>(t3, im);
  const V a1= vplus<P>(t1, pi), a3= vdiff<P>(t1, pi);
  const V o0= vlow<P>(vplus<P>(t0, t2));
  const V o1= vmul<P>(a1, w1);
  const V o2= vmul<P>(vdiff<P>(t0, t2), w2);
  const V o3= vmul<P>(a3, w3);
  vstore<P>(q, _mm256_permute2x128_si256(o0, o1, 0x20));
  vstore<P>(q + 8, _mm256_permute2x128_si256(o2, o3, 0x20));
  vstore<P>(q + 16, _mm256_permute2x128_si256(o0, o1, 0x31));
  vstore<P>(q + 24, _mm256_permute2x128_si256(o2, o3, 0x31));
 }
}

// len = 16 の段 (h = 4)。逆変換。u0 には Montgomery の 1 を掛けて、u0 と y1 を 1 本にそろえる。
template <unsigned P> void idft_stage_h4(u32* d, int n) {
 if (n < 32) {
  idft_stage_small<P>(d, n, 16);
  return;
 }
 const u32* tw= small_tw<P>(16, 4, true);
 const V wa= vcombine128<P>(vsplat128<P>(mc<P>::r), vload128<P>(tw));
 const V wb= vcombine128<P>(vload128<P>(tw + 4), vload128<P>(tw + 8));
 const V& ii= ii_vec<P>();
 for (int b= 0; b + 32 <= n; b+= 32) {
  u32* q= d + b;
  const V x0= vload<P>(q), x1= vload<P>(q + 8), x2= vload<P>(q + 16), x3= vload<P>(q + 24);
  const V y0= vmul<P>(x0, wa), y1= vmul<P>(x1, wb);
  const V y2= vmul<P>(x2, wa), y3= vmul<P>(x3, wb);
  const V s0= vplus<P>(y0, y1), s1= vplus<P>(y2, y3);
  const V d0= vdiff<P>(y0, y1), d1= vdiff<P>(y2, y3);
  const V t0= vlow<P>(_mm256_permute2x128_si256(s0, s1, 0x20));  // u0+y2
  const V t1= vlow<P>(_mm256_permute2x128_si256(s0, s1, 0x31));  // y1+y3
  const V t2= vlow<P>(_mm256_permute2x128_si256(d0, d1, 0x20));  // u0-y2
  const V t3= vmuls<P>(_mm256_permute2x128_si256(d0, d1, 0x31), ii);  // (y1-y3) i^{-1}
  const V o0= vlow<P>(vplus<P>(t0, t1));
  const V o1= vlow<P>(vplus<P>(t2, t3));
  const V o2= vlow<P>(vdiff<P>(t0, t1));
  const V o3= vlow<P>(vdiff<P>(t2, t3));
  vstore<P>(q, _mm256_permute2x128_si256(o0, o1, 0x20));
  vstore<P>(q + 8, _mm256_permute2x128_si256(o2, o3, 0x20));
  vstore<P>(q + 16, _mm256_permute2x128_si256(o0, o1, 0x31));
  vstore<P>(q + 24, _mm256_permute2x128_si256(o2, o3, 0x31));
 }
}

// len = 8 の段 (h = 2)。2 ブロック (16 個) ずつ。1 ブロックが 8 レーン 1 本 ([u0|u1|u2|u3]、各 2 レーン) なので、
// 128 bit の入れ替えで u0+u2 と u1+u3 を同じ 128 bit にそろえ、2 ブロック分を 64 bit ずつ組にする。順変換。
// 1 ブロックだけのタイル (n = 8) はスカラーに回す。
template <unsigned P> void dft_stage_h2(u32* d, int n) {
 if (n < 16) {
  dft_stage_small<P>(d, n, 8);
  return;
 }
 const u32* tw= small_tw<P>(8, 2, false);
 const V w1= _mm256_set_epi32(tw[1], tw[0], tw[1], tw[0], tw[1], tw[0], tw[1], tw[0]);
 const V w2= _mm256_set_epi32(tw[5], tw[4], tw[5], tw[4], tw[5], tw[4], tw[5], tw[4]);
 const V w3= _mm256_set_epi32(tw[9], tw[8], tw[9], tw[8], tw[9], tw[8], tw[9], tw[8]);
 const V& im= im_vec<P>();
 for (int b= 0; b + 16 <= n; b+= 16) {
  u32* q= d + b;
  const V x0= vload<P>(q), x1= vload<P>(q + 8);
  const V z0= _mm256_permute2x128_si256(x0, x0, 0x01), z1= _mm256_permute2x128_si256(x1, x1, 0x01);
  const V s0= vplus<P>(x0, z0), s1= vplus<P>(x1, z1);
  const V d0= vdiff<P>(x0, z0), d1= vdiff<P>(x1, z1);
  const V t0= vlow<P>(_mm256_unpacklo_epi64(s0, s1));  // u0+u2 (b0 | b1)
  const V t2= vlow<P>(_mm256_unpackhi_epi64(s0, s1));  // u1+u3
  const V t1= vlow<P>(_mm256_unpacklo_epi64(d0, d1));  // u0-u2
  const V t3= _mm256_unpackhi_epi64(d0, d1);           // u1-u3
  const V pi= vmuls<P>(t3, im);
  const V a1= vplus<P>(t1, pi), a3= vdiff<P>(t1, pi);
  const V o0= vlow<P>(vplus<P>(t0, t2));
  const V o1= vmul<P>(a1, w1);
  const V o2= vmul<P>(vdiff<P>(t0, t2), w2);
  const V o3= vmul<P>(a3, w3);
  // o0 は [b0 | b1 | b0 | b1] (64 bit ごと) なので、b0 は unpacklo、b1 は unpackhi から取る。
  const V u01= _mm256_unpacklo_epi64(o0, o1), u23= _mm256_unpacklo_epi64(o2, o3);
  const V v01= _mm256_unpackhi_epi64(o0, o1), v23= _mm256_unpackhi_epi64(o2, o3);
  vstore<P>(q, _mm256_permute2x128_si256(u01, u23, 0x20));
  vstore<P>(q + 8, _mm256_permute2x128_si256(v01, v23, 0x20));
 }
}

// len = 8 の段 (h = 2)。逆変換。
template <unsigned P> void idft_stage_h2(u32* d, int n) {
 if (n < 16) {
  idft_stage_small<P>(d, n, 8);
  return;
 }
 const u32* tw= small_tw<P>(8, 2, true);
 const V mv= _mm256_set_epi32(tw[9], tw[8], tw[5], tw[4], tw[1], tw[0], int(mc<P>::r), int(mc<P>::r));
 const V& ii= ii_vec<P>();
 for (int b= 0; b + 16 <= n; b+= 16) {
  u32* q= d + b;
  const V x0= vload<P>(q), x1= vload<P>(q + 8);
  const V y0= vmul<P>(x0, mv), y1= vmul<P>(x1, mv);
  const V z0= _mm256_permute2x128_si256(y0, y0, 0x01), z1= _mm256_permute2x128_si256(y1, y1, 0x01);
  const V s0= vplus<P>(y0, z0), s1= vplus<P>(y1, z1);
  const V d0= vdiff<P>(y0, z0), d1= vdiff<P>(y1, z1);
  const V t0= vlow<P>(_mm256_unpacklo_epi64(s0, s1));
  const V t1= vlow<P>(_mm256_unpackhi_epi64(s0, s1));
  const V t2= vlow<P>(_mm256_unpacklo_epi64(d0, d1));
  const V t3= vmuls<P>(_mm256_unpackhi_epi64(d0, d1), ii);
  const V o0= vlow<P>(vplus<P>(t0, t1));
  const V o1= vlow<P>(vplus<P>(t2, t3));
  const V o2= vlow<P>(vdiff<P>(t0, t1));
  const V o3= vlow<P>(vdiff<P>(t2, t3));
  // o0 は [b0 | b1 | b0 | b1] (64 bit ごと) なので、b0 は unpacklo、b1 は unpackhi から取る。
  const V u01= _mm256_unpacklo_epi64(o0, o1), u23= _mm256_unpacklo_epi64(o2, o3);
  const V v01= _mm256_unpackhi_epi64(o0, o1), v23= _mm256_unpackhi_epi64(o2, o3);
  vstore<P>(q, _mm256_permute2x128_si256(u01, u23, 0x20));
  vstore<P>(q + 8, _mm256_permute2x128_si256(v01, v23, 0x20));
 }
}

// タイル (dft_tile 以下) の残りの段をまとめて回す。順変換。
template <unsigned P> void dft_stages_small(u32* d, int n) {
 int len= n;
 for (; len >= 32; len>>= 2) dft_stage_vec<P>(d, n, len);
 if (len == 16) dft_stage_h4<P>(d, n), len= 4;
 if (len == 8) dft_stage_h2<P>(d, n), len= 2;
 if (len == 4) {
  if (n >= 8) dft_stage_h1<P>(d, n);
  else dft_stage_small<P>(d, n, 4);
  len= 1;
 }
 if (len == 2) dft_stage_r2<P>(d, n);
}
// タイル (dft_tile 以下) の段を、小さい方から順に回す。逆変換。
template <unsigned P> void idft_stages_small(u32* d, int n) {
 const bool odd_e= __builtin_ctz(unsigned(n)) & 1;
 if (odd_e) {
  dft_stage_r2<P>(d, n);
  if (n >= 8) idft_stage_h2<P>(d, n);
  for (int len= 32; len <= n; len<<= 2) idft_stage_vec<P>(d, n, len);
 } else {
  if (n >= 8) idft_stage_h1<P>(d, n);
  else idft_stage_small<P>(d, n, 4);
  if (n >= 16) idft_stage_h4<P>(d, n);
  for (int len= 64; len <= n; len<<= 2) idft_stage_vec<P>(d, n, len);
 }
}

// DIF。a[i] に Σ_j a[j] ω^{j k} が入る。k は位置 i の 4 進の桁を逆にした数 (2 の指数が奇数なら最後の基数 2 の段で最上位の桁が最下位に来る)。
template <unsigned P> void to_mont(u32* d, int n) {
 const V r2v= vsplat<P>(mc<P>::r2);
 int i= 0;
 for (; i + 8 <= n; i+= 8) vstore<P>(d + i, vmuls<P>(vload<P>(d + i), r2v));
 for (; i < n; ++i) d[i]= smont<P>(d[i]);
}
template <unsigned P> void from_mont(u32* d, int n) {
 const V onev= vsplat<P>(1);
 int i= 0;
 for (; i + 8 <= n; i+= 8) vstore<P>(d + i, vmuls<P>(vload<P>(d + i), onev));
 for (; i < n; ++i) d[i]= smul<P>(d[i], 1);
}
template <unsigned P> void dft_impl(ZMod<P>* a, int n) {
 if (n <= 1) return;
 assert((n & (n - 1)) == 0 && (P - 1) % unsigned(n) == 0);
 u32* d= reinterpret_cast<u32*>(a);
 to_mont<P>(d, n);
 // 大きい段 (タイルより長い段) は配列全体をなめる。タイルは段の長さに合わせて 4 で割って決める。
 int tile= n;
 while (tile > dft_tile) tile>>= 2;
 for (int len= n; len > tile; len>>= 2) dft_stage_vec<P>(d, n, len);
 // 残りの段はタイルごとに、タイルを L1 に載せたまま回す。
 for (int b= 0; b < n; b+= tile) dft_stages_small<P>(d + b, tile);
 from_mont<P>(d, n);
}

// DIT。dft の逆。段ごとの 1/4 は掛けず、最後に 1/n を掛けながらふつうの値に戻す。
template <unsigned P> void idft_impl(ZMod<P>* a, int n) {
 if (n <= 1) return;
 assert((n & (n - 1)) == 0 && (P - 1) % unsigned(n) == 0);
 u32* d= reinterpret_cast<u32*>(a);
 to_mont<P>(d, n);
 // 小さい段はタイルごとに、大きい段は配列全体をなめる。タイルは段の長さに合わせて 4 で割って決める。
 int tile= n;
 while (tile > dft_tile) tile>>= 2;
 for (int b= 0; b < n; b+= tile) idft_stages_small<P>(d + b, tile);
 for (int len= tile<< 2; len <= n; len<<= 2) idft_stage_vec<P>(d, n, len);
 // 1/n を掛けると、Montgomery のままの値がふつうの値に戻る。
 const V cv= vsplat<P>(inv_n_scalar<P>(n));
 int i= 0;
 for (; i + 8 <= n; i+= 8) vstore<P>(d + i, vmuls<P>(vload<P>(d + i), cv));
 if (i < n) {
  const u32 iv= inv_n_scalar<P>(n);
  for (; i < n; ++i) d[i]= smul<P>(d[i], iv);
 }
}

// a[0..n) は長さ n の変換。係数に戻して 0 を詰め、長さ 2n の変換を a[0..2n) に作る (2 本目と同じ素直な形)。
template <unsigned P> void dft_doubling_impl(ZMod<P>* a, int n) {
 std::vector<ZMod<P>> c(a, a + n);
 idft_impl<P>(c.data(), n);
 std::fill(a, a + 2 * n, ZMod<P>());
 std::copy(c.begin(), c.end(), a);
 dft_impl<P>(a, 2 * n);
}

// 以下、even_dft と odd_dft は 2 本目と同じ並びの式を、ふつうの値のまま (u64 の剰余) で書く。
template <unsigned P> u32 pmul(u32 a, u32 b) { return u32(u64(a) * b % P); }
template <unsigned P> u32 padd(u32 a, u32 b) { return u32((u64(a) + b) % P); }

// 長さ n の変換から、偶数次の部分の長さ n/2 の変換を作る。
template <unsigned P> void even_dft_impl(ZMod<P>* a, int n) {
 const int m= n>> 1;
 const u32 iv2= (P + 1) / 2;
 if (__builtin_ctz(unsigned(n)) & 1) {
  for (int j= 0; j < m; ++j) a[j]= ZMod<P>::raw(pmul<P>(padd<P>(a[2 * j].val(), a[2 * j + 1].val()), iv2));
 } else {
  for (int j= 0; j < m; ++j) {
   const int i= ((j>> 1)<< 2) | (j & 1);
   a[j]= ZMod<P>::raw(pmul<P>(padd<P>(a[i].val(), a[i + 2].val()), iv2));
  }
 }
}

// 長さ n の変換から、奇数次の部分の長さ n/2 の変換を作る。
template <unsigned P> void odd_dft_impl(ZMod<P>* a, int n) {
 const int e= __builtin_ctz(unsigned(n));
 const int m= n>> 1;
 const u32 g= primitive_root<P>();
 const u32 wi= spow<P>(g, u64(P - 1) - u64(P - 1) / unsigned(n));  // ω_n^{-1}
 const u32 iv2= (P + 1) / 2;
 const int D= (e & 1) ? (e>> 1) : (e>> 1) - 1;
 const u64 p4= D ? 1ull << (2 * D) : 1;
 const u32 w4p= spow<P>(g, u64(P - 1) / unsigned(n) * p4);  // ω_n^{+4^D}
 const u32 w4i= spow<P>(wi, p4);                           // ω_n^{-4^D}
 u32 step[16];
 for (int t= 0; t < D; ++t) step[t]= pmul<P>(spow<P>(wi, 5ull << (2 * (D - 1 - t))), w4p);
 if (e & 1) {
  u32 base= iv2;
  for (int j= 0; j < m; ++j) {
   a[j]= ZMod<P>::raw(pmul<P>(padd<P>(a[2 * j].val(), P - a[2 * j + 1].val()), base));
   if (j + 1 < m) base= pmul<P>(base, step[__builtin_ctz(~unsigned(j))>> 1]);
  }
 } else {
  const int mm= m>> 1;
  u32 base= iv2;
  for (int j= 0; j < mm; ++j) {
   a[2 * j]= ZMod<P>::raw(pmul<P>(padd<P>(a[4 * j].val(), P - a[4 * j + 2].val()), base));
   a[2 * j + 1]= ZMod<P>::raw(pmul<P>(padd<P>(a[4 * j + 1].val(), P - a[4 * j + 3].val()), pmul<P>(base, w4i)));
   if (j + 1 < mm) base= pmul<P>(base, step[__builtin_ctz(~unsigned(j))>> 1]);
  }
 }
}


template <class K> struct static_ntt_mod: std::false_type {
 static constexpr unsigned mod= 0;
};
template <unsigned P> struct static_ntt_mod<ZMod<P>>: std::bool_constant<ntt_friendly<P>> {
 static constexpr unsigned mod= P;
};
}  // namespace ntt_avx2_internal

struct NttAvx2 {
 // out[k - lo] = Σ_{i+j=k} a[i] b[j] (lo <= k < hi)。n >= 1、m >= 1、0 <= lo < hi <= n + m - 1。
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  if constexpr (ntt_avx2_internal::static_ntt_mod<K>::value) {
   constexpr unsigned P= ntt_avx2_internal::static_ntt_mod<K>::mod;
   // 巡回の長さは L >= max(hi, n + m - 1 - lo) でよい (折り返した項が [lo, hi) に入らない)。
   const int need= std::max(hi, n + m - 1 - lo);
   int l= 1;
   while (l < need) l<<= 1;
   if (std::min(n, m) >= 60 && (P - 1) % unsigned(l) == 0) {
    std::vector<K> fa(a, a + n), fb;
    fa.resize(l);
    if (a == b && n == m) {
     dft<P>(fa.data(), l);
     for (int i= 0; i < l; ++i) fa[i]*= fa[i];
    } else {
     fb.assign(b, b + m), fb.resize(l);
     dft<P>(fa.data(), l), dft<P>(fb.data(), l);
     for (int i= 0; i < l; ++i) fa[i]*= fb[i];
    }
    idft<P>(fa.data(), l);
    for (int k= lo; k < hi; ++k) out[k - lo]= fa[k];
    return;
   }
  }
  Naive::convolve_range(a, n, b, m, lo, hi, out);
 }

 template <unsigned P> requires ntt_friendly<P> static void dft(ZMod<P>* a, int n) { ntt_avx2_internal::dft_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void idft(ZMod<P>* a, int n) { ntt_avx2_internal::idft_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void dft_doubling(ZMod<P>* a, int n) { ntt_avx2_internal::dft_doubling_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void even_dft(ZMod<P>* a, int n) { ntt_avx2_internal::even_dft_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void odd_dft(ZMod<P>* a, int n) { ntt_avx2_internal::odd_dft_impl<P>(a, n); }
};
