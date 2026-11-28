#pragma once
// 単独の NTT の核の 13 本目 (実験版)。5 本目 (splat) の段を、ひねりを区画ごとに持つ形 (DIT) にしたものです。
// 段の内側で表を読まないので、蝶 1 つあたりの命令が 5 本目より少なくなります。
//
// - 順変換の段 (ブロック長 4h、区画の番号 k): ひねりは (rlog[k], rlog[2k], rlog[2k+1]) で、j によらず
//   区画ごとに 1 つです。蝶は c,d に x を掛けてから組み替え、(b+d) に y、(b-d) に z を掛けます。
// - rlog[i] = ω_nv^{br(i)} (br は log2(nv)-1 bit の反転) の 1 つの表を、段の長さによらず使います。
// - 位置 v は周波数 bitrev(v) に対応するので、葉の重みは w = η^{bitrev(v)} です (η は nv 乗根)。
// - nv の 2 の指数が奇数なら、基数 2 の段をいちばん外側に置きます。
// 数値は 5 本目と同じで、段と葉の並べ方だけが違います。Python の試作で巡回畳み込みと一致することを
// 確かめてから写しました (2026-11-28)。
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/naive.hpp"
#include "_shared/ntt/_kernel_base.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

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

namespace ntt_dit_internal {
using u32= unsigned;
using u64= unsigned long long;
using V= __m256i;
using ntt_avx2_internal::inv_n_scalar;
using ntt_avx2_internal::mc;
using ntt_avx2_internal::primitive_root;
using ntt_avx2_internal::spow;

// タイルのベクトル数 (要素数では 8 倍)。
constexpr int tile_vec= 256;

template <unsigned P> V vsplat(u32 x) { return _mm256_set1_epi32(int(x)); }
template <unsigned P> V vload(const u32* p) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p)); }
template <unsigned P> void vstore(u32* p, V x) { _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), x); }
template <unsigned P> V vplus(V x, V y) { return _mm256_add_epi32(x, y); }
template <unsigned P> V vdiff(V x, V y) { return _mm256_sub_epi32(_mm256_add_epi32(x, vsplat<P>(mc<P>::p2)), y); }
template <unsigned P> V vlow(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, vsplat<P>(mc<P>::p2))); }
template <unsigned P> V vcanon(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, vsplat<P>(P))); }

#if defined(__AVX2__) && !defined(USE_SIMDE)
template <unsigned P> V vpmulld(V a, V b) { return _mm256_mullo_epi32(a, b); }
#else
template <unsigned P> V vpmulld(V a, V b) {
 const V e= _mm256_mul_epu32(a, b);
 const V o= _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
 return _mm256_blend_epi32(e, _mm256_slli_epi64(o, 32), 0xAA);
}
#endif

// Shoup の掛け算 (w と wi は全レーン同じ)。x < 2^32、w < P、wi = floor(w 2^32 / P)。結果は 2P 未満。
template <unsigned P> V vshoups(V x, V w, V wi) {
 const V pv= vsplat<P>(P);
 const V qe= _mm256_mul_epu32(x, wi);
 const V qo= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), wi);
 const V q= _mm256_blend_epi32(_mm256_srli_epi64(qe, 32), qo, 0xAA);
 return _mm256_sub_epi32(vpmulld<P>(x, w), vpmulld<P>(q, pv));
}

template <unsigned P> u32 shoup_q(u32 w) { return u32((u64(w) << 32) / P); }

template <unsigned P> u32 im_normal() {
 static const u32 v= spow<P>(primitive_root<P>(), u64(P - 1) / 4);
 return v;
}

struct tbuf {
 u32* p= nullptr;
 int n= 0;
 void assign(int n_) {
  if (n == n_) return;
  std::free(p);
  void* q= nullptr;
  if (posix_memalign(&q, 32, size_t(n_) * 4)) std::abort();
  p= static_cast<u32*>(q), n= n_;
 }
};

// 全体で 1 つの根の表。rlog[i] = ω_nv^{br(i)} (br は log2(nv)-1 bit の反転) とその商。
// 段の長さによらず、区画の番号 k で引く。count = nv/2 個ずつ、w と商で 2 * count 個。
template <unsigned P> struct root_tables {
 static inline tbuf t[2][26];
};
template <unsigned P> const u32* root_table(int nv, bool inv) {
 const int e= __builtin_ctz(unsigned(nv));
 const int count= nv >> 1;
 auto& tb= root_tables<P>::t[inv ? 1 : 0][e];
 if (tb.n != 2 * count) {
  tb.assign(2 * count);
  const u32 g= primitive_root<P>();
  const u32 om= spow<P>(g, inv ? u64(P - 1) - u64(P - 1) / unsigned(nv) : u64(P - 1) / unsigned(nv));
  u32* w= tb.p;
  u32* q= tb.p + count;
  // w[i] = om^{br(i)} を、i を 1 進めるごとの比で作る。br の幅は e-1。
  // 末尾に 1 が s 個続く i からの指数の差は 2^(width-1-s) - 2^(width-s) * (2^s - 1)。
  // om の位数は nv = 2^(width+1) なので、mod nv で還元する (mod 2^width では符号が変わる)。
  const int width= e - 1;
  std::vector<u32> step(width);
  for (int s= 0; s < width; ++s) {
   const long long delta= (3LL << (width - 1 - s)) - (1LL << width);
   const u32 exph= u32(((delta % (long long)nv) + nv) % nv);
   step[s]= spow<P>(om, exph);
  }
  u32 cur= 1;
  w[0]= 1;
  for (int i= 0; i < count - 1; ++i) {
   int s= __builtin_ctz(~unsigned(i));
   if (s >= width) s= width - 1;
   cur= u32(u64(cur) * step[s] % P);
   w[i + 1]= cur;
  }
  for (int i= 0; i < count; ++i) q[i]= shoup_q<P>(w[i]);
 }
 return tb.p;
}

// 葉の重み w = η^{bitrev(v)} の表 (nv 個) と商。
template <unsigned P> struct leaf_tables {
 static inline tbuf t[26];
};
template <unsigned P> const u32* leaf_table_bits(int nv) {
 const int e= __builtin_ctz(unsigned(nv));
 auto& tb= leaf_tables<P>::t[e];
 if (tb.n != 2 * nv) {
  tb.assign(2 * nv);
  const u32 g= primitive_root<P>();
  const u32 eta= spow<P>(g, u64(P - 1) / unsigned(nv));
  u32* w= tb.p;
  u32* q= tb.p + nv;
  // bitrev の幅は e。末尾に 1 が s 個続く i からの比は η^{3 * 2^(e-1-s)} (mod nv)。
  std::vector<u32> step(e);
  for (int s= 0; s < e; ++s) {
   const long long delta= (3LL << (e - 1 - s)) - (1LL << e);
   step[s]= spow<P>(eta, u32(((delta % (long long)nv) + nv) % nv));
  }
  u32 cur= 1;
  w[0]= 1;
  for (int i= 0; i < nv - 1; ++i) {
   int s= __builtin_ctz(~unsigned(i));
   if (s >= e) s= e - 1;
   cur= u32(u64(cur) * step[s] % P);
   w[i + 1]= cur;
  }
  for (int i= 0; i < nv; ++i) q[i]= shoup_q<P>(w[i]);
 }
 return tb.p;
}

// 段 (ブロック長 4h、区画の番号 k) の順変換。ひねりは区画ごとに 1 つ。
template <unsigned P> void stage_dit(u32* d, int h, int k, const u32* tb, int count) {
 const u32 *w= tb, *q= tb + count;
 const V X= vsplat<P>(w[k]), Y= vsplat<P>(w[2 * k]), Z= vsplat<P>(w[2 * k + 1]);
 const V Xq= vsplat<P>(q[k]), Yq= vsplat<P>(q[2 * k]), Zq= vsplat<P>(q[2 * k + 1]);
 for (int j= 0; j < h; ++j) {
  u32* p= d + 8 * j;
  const V u0= vload<P>(p), u1= vload<P>(p + 8 * h), u2= vload<P>(p + 16 * h), u3= vload<P>(p + 24 * h);
  const V c= vshoups<P>(u2, X, Xq), dd= vshoups<P>(u3, X, Xq);
  const V ac= vplus<P>(u0, c), amc= vdiff<P>(u0, c);
  const V bd0= vplus<P>(u1, dd), bmd0= vdiff<P>(u1, dd);
  const V bd= vshoups<P>(bd0, Y, Yq), bmd= vshoups<P>(bmd0, Z, Zq);
  const V lac= vlow<P>(ac), lamc= vlow<P>(amc);
  vstore<P>(p, vlow<P>(vplus<P>(lac, bd)));
  vstore<P>(p + 8 * h, vlow<P>(vdiff<P>(lac, bd)));
  vstore<P>(p + 16 * h, vlow<P>(vplus<P>(lamc, bmd)));
  vstore<P>(p + 24 * h, vlow<P>(vdiff<P>(lamc, bmd)));
 }
}

// 段 (ブロック長 4h、区画の番号 k) の逆変換。表は共役のもの。
template <unsigned P> void stage_dit_inv(u32* d, int h, int k, const u32* tb, int count) {
 const u32 *w= tb, *q= tb + count;
 const V X= vsplat<P>(w[k]), Y= vsplat<P>(w[2 * k]), Z= vsplat<P>(w[2 * k + 1]);
 const V Xq= vsplat<P>(q[k]), Yq= vsplat<P>(q[2 * k]), Zq= vsplat<P>(q[2 * k + 1]);
 for (int j= 0; j < h; ++j) {
  u32* p= d + 8 * j;
  const V u0= vload<P>(p), u1= vload<P>(p + 8 * h), u2= vload<P>(p + 16 * h), u3= vload<P>(p + 24 * h);
  const V ab= vplus<P>(u0, u1), cd= vplus<P>(u2, u3);
  const V amb= vshoups<P>(vdiff<P>(u0, u1), Y, Yq);
  const V cmd= vshoups<P>(vdiff<P>(u2, u3), Z, Zq);
  const V lab= vlow<P>(ab), lcd= vlow<P>(cd);
  vstore<P>(p, vlow<P>(vplus<P>(lab, lcd)));
  vstore<P>(p + 8 * h, vlow<P>(vplus<P>(amb, cmd)));
  vstore<P>(p + 16 * h, vshoups<P>(vdiff<P>(lab, lcd), X, Xq));
  vstore<P>(p + 24 * h, vshoups<P>(vdiff<P>(amb, cmd), X, Xq));
 }
}

// 基数 2 の段 (nv の 2 の指数が奇数のとき、いちばん外側)。ひねりは 1。
template <unsigned P> void stage_top(u32* d, int nv) {
 const int h= nv >> 1;
 for (int i= 0; i < h; ++i) {
  const V u0= vload<P>(d + 8 * i), u1= vload<P>(d + 8 * (i + h));
  vstore<P>(d + 8 * i, vlow<P>(vplus<P>(u0, u1)));
  vstore<P>(d + 8 * (i + h), vlow<P>(vdiff<P>(u0, u1)));
 }
}

// タイル (NV ベクトル) の順変換の段の列。L = NV, NV/4, …, 4。区画の番号 k は 4k + t で下る。
template <unsigned P, int L> struct tile_fwd {
 static void run(u32* d, int k, const u32* tb, int count) {
  stage_dit<P>(d, L >> 2, k, tb, count);
  if constexpr (L > 4) {
   for (int t= 0; t < 4; ++t) tile_fwd<P, L / 4>::run(d + 8 * t * (L / 4), 4 * k + t, tb, count);
  }
 }
};
// タイルの逆変換の段の列。L = 4, 16, …, NV。
template <unsigned P, int L> struct tile_inv {
 static void run(u32* d, int k, const u32* tb, int count) {
  if constexpr (L > 4) {
   for (int t= 0; t < 4; ++t) tile_inv<P, L / 4>::run(d + 8 * t * (L / 4), 4 * k + t, tb, count);
  }
  stage_dit_inv<P>(d, L >> 2, k, tb, count);
 }
};

// 葉の積 (mod x^8 - w)。5 本目と同じ。
template <unsigned P> void leaf_build(u32* buf, u32* cb, V ya, V yb, V wv, V wq) {
 vstore<P>(buf, vshoups<P>(ya, wv, wq));
 vstore<P>(buf + 8, ya);
 vstore<P>(cb, yb);
}
template <unsigned P> V leaf_mac(const u32* buf, const u32* cb) {
 const V pv= vsplat<P>(P), niv= vsplat<P>(mc<P>::ni);
 V prev= vload<P>(buf + 8);
 V e, o;
 {
  const V y0= vsplat<P>(cb[0]);
  e= _mm256_mul_epu32(prev, y0);
  o= _mm256_mul_epu32(_mm256_srli_epi64(prev, 32), y0);
 }
 for (int i= 1; i < 8; ++i) {
  const V x= vload<P>(buf + 8 - i);
  const V yi= vsplat<P>(cb[i]);
  e= _mm256_add_epi64(e, _mm256_mul_epu32(x, yi));
  o= _mm256_add_epi64(o, _mm256_mul_epu32(prev, yi));
  prev= x;
 }
 e= _mm256_add_epi64(e, _mm256_mul_epu32(_mm256_mul_epu32(e, niv), pv));
 o= _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_mul_epu32(o, niv), pv));
 return _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xAA);
}

// タイル (NV ベクトル) を、順変換の段、葉の積、逆変換の段まで回す。
template <unsigned P, int NV> void tile_work_c(u32* fa, u32* fb, int m, int B, int k, bool square, V sc, V scq) {
 const u32* tw= root_table<P>(m, false);
 const int count= m >> 1;
 tile_fwd<P, NV>::run(fa, k, tw, count);
 if (!square) tile_fwd<P, NV>::run(fb, k, tw, count);
 const u32* lw= leaf_table_bits<P>(m);
 const u32* lq= lw + m;
 alignas(32) u32 buf[2][16], cb[2][8];
 auto build= [&](int i, int slot) {
  const V ya= vcanon<P>(vload<P>(fa + 8 * i));
  const V yb= square ? ya : vcanon<P>(vload<P>(fb + 8 * i));
  leaf_build<P>(buf[slot], cb[slot], ya, yb, vsplat<P>(lw[B + i]), vsplat<P>(lq[B + i]));
 };
 build(0, 0);
 for (int i= 0; i < NV; ++i) {
  if (i + 1 < NV) build(i + 1, (i + 1) & 1);
  const V c= leaf_mac<P>(buf[i & 1], cb[i & 1]);
  vstore<P>(fa + 8 * i, vlow<P>(vshoups<P>(c, sc, scq)));
 }
 const u32* twi= root_table<P>(m, true);
 tile_inv<P, NV>::run(fa, k, twi, count);
}

template <unsigned P> void tile_work(u32* fa, u32* fb, int m, int nv, int B, int k, bool square, V sc, V scq) {
 switch (nv) {
  case 4: tile_work_c<P, 4>(fa, fb, m, B, k, square, sc, scq); break;
  case 16: tile_work_c<P, 16>(fa, fb, m, B, k, square, sc, scq); break;
  case 64: tile_work_c<P, 64>(fa, fb, m, B, k, square, sc, scq); break;
  case 256: tile_work_c<P, 256>(fa, fb, m, B, k, square, sc, scq); break;
  case 1024: tile_work_c<P, 1024>(fa, fb, m, B, k, square, sc, scq); break;
  default: std::abort();
 }
}

// 深さ優先で区画を回す。nv は 2 の指数が偶数 (基数 2 の段は外で済ませてある)。
template <unsigned P> void rec(u32* fa, u32* fb, int nv, int k, int m, int B, bool square, V sc, V scq) {
 if (nv <= tile_vec) {
  tile_work<P>(fa, fb, m, nv, B, k, square, sc, scq);
  return;
 }
 const u32* tw= root_table<P>(m, false);
 const int count= m >> 1;
 stage_dit<P>(fa, nv >> 2, k, tw, count);
 if (!square) stage_dit<P>(fb, nv >> 2, k, tw, count);
 const int h= nv >> 2;
 for (int t= 0; t < 4; ++t) rec<P>(fa + 8 * t * h, fb + 8 * t * h, h, 4 * k + t, m, B + t * h, square, sc, scq);
 const u32* twi= root_table<P>(m, true);
 stage_dit_inv<P>(fa, nv >> 2, k, twi, count);
}

// 作業の配列。
template <unsigned P> struct work_buf {
 static inline u32* fa= nullptr;
 static inline u32* fb= nullptr;
 static inline int cap= 0;
 static void reserve(int l) {
  if (l <= cap) return;
  std::free(fa), std::free(fb);
  void *pa= nullptr, *pb= nullptr;
  if (posix_memalign(&pa, 64, size_t(l) * 4) || posix_memalign(&pb, 64, size_t(l) * 4)) std::abort();
  fa= static_cast<u32*>(pa), fb= static_cast<u32*>(pb), cap= l;
 }
};

// 畳み込みの入口。
template <unsigned P> void convolve_entry(const ZMod<P>* a, int n, const ZMod<P>* b, int m_, int lo, int hi, ZMod<P>* out) {
 // 巡回の長さは L >= max(hi, n + m - 1 - lo) でよい。
 const int need= std::max(hi, n + m_ - 1 - lo);
 int l= 1;
 while (l < need) l<<= 1;
 if (std::min(n, m_) >= 60 && l >= 128 && (P - 1) % unsigned(l) == 0) {
  const int mv= l >> 3;
  const bool square= (a == b && n == m_);
  work_buf<P>::reserve(l);
  u32* fa= work_buf<P>::fa;
  u32* fb= work_buf<P>::fb;
  std::copy(reinterpret_cast<const u32*>(a), reinterpret_cast<const u32*>(a) + n, fa), std::fill(fa + n, fa + l, 0);
  if (!square) std::copy(reinterpret_cast<const u32*>(b), reinterpret_cast<const u32*>(b) + m_, fb), std::fill(fb + m_, fb + l, 0);
  // 葉の積が R^{-1} を 1 回付け、残りの逆の段が m 倍するので、R / m を掛ける (8 R / l)。
  const u32 c= u32(u64(mc<P>::r) * inv_n_scalar<P>(l) % P);
  const u32 c8= u32(u64(c) * 8 % P);
  const V scv= vsplat<P>(c8), scq= vsplat<P>(shoup_q<P>(c8));
  if (__builtin_ctz(unsigned(mv)) & 1) {
   // 基数 2 の段を外に置き、半分ずつ k = 0 と k = 1 で回す。
   const int h= mv >> 1;
   stage_top<P>(fa, mv);
   if (!square) stage_top<P>(fb, mv);
   rec<P>(fa, fb, h, 0, mv, 0, square, scv, scq);
   rec<P>(fa + 8 * h, fb + 8 * h, h, 1, mv, h, square, scv, scq);
   stage_top<P>(fa, mv);
  } else {
   rec<P>(fa, fb, mv, 0, mv, 0, square, scv, scq);
  }
  for (int k= lo; k < hi; ++k) out[k - lo]= ZMod<P>::raw(fa[k]);
  return;
 }
 Naive::convolve_range(a, n, b, m_, lo, hi, out);
}
}  // namespace ntt_dit_internal

// 13 本目の核。畳み込みは上の道、素の変換は 3 本目のものを使う。
struct NttAvx2Dit {
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  if constexpr (ntt_avx2_internal::static_ntt_mod<K>::value) {
   ntt_dit_internal::convolve_entry<ntt_avx2_internal::static_ntt_mod<K>::mod>(a, n, b, m, lo, hi, out);
   return;
  }
  Naive::convolve_range(a, n, b, m, lo, hi, out);
 }

 template <unsigned P> requires ntt_friendly<P> static void dft(ZMod<P>* a, int n) { NttAvx2::dft(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void idft(ZMod<P>* a, int n) { NttAvx2::idft(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void dft_doubling(ZMod<P>* a, int n) { NttAvx2::dft_doubling(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void even_dft(ZMod<P>* a, int n) { NttAvx2::even_dft(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void odd_dft(ZMod<P>* a, int n) { NttAvx2::odd_dft(a, n); }
};
