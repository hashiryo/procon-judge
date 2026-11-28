#pragma once
// 5 本目 (splat) に prefetch のヒントを足した実験版。段の内側で 2 つ先の j と表を先に読む。
// 単独の NTT の核の 5 本目 (実験版)。ひねりを全レーン同じ (broadcast) にして、表を要素あたり 1/8 にする形。
// 8 レーンは葉の 8 個の係数で、段はベクトルの添字 (葉の添字) の上の DFT になる。段のひねりは位置ごとに
// 1 つの値なので、表はベクトルあたり 2 個 (3 つのひねりと商で 6 個 / 4 ベクトル) で済む。
// 葉の上の DFT は、長さ 8 の DFT を 8 次の環 x^8 - w (w = η^{k2}、k2 は葉の位置の桁を逆にした数) の積に
// 置き換える。w は葉ごとに変わるので、w の表 (m 個) を持つ。
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

namespace ntt_splat_pf_internal {
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
template <unsigned P> u32 ii_normal() {
 static const u32 v= spow<P>(primitive_root<P>(), u64(P - 1) - u64(P - 1) / 4);
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

// 段 (ベクトル数 L) のひねり。長さ m の DFT の、L の段の根は η_m^{m/L}。
// h = L/4 個ずつ、[w1|w2|w3|s1|s2|s3]。
template <unsigned P> struct stage_tables {
 static inline tbuf t[2][26];
};
template <unsigned P> u32 root_of(int m, int L, bool inv) {
 const u32 g= primitive_root<P>();
 const u64 e= u64(P - 1) / unsigned(m) * unsigned(m / L);
 return spow<P>(g, inv ? u64(P - 1) - e : e);
}
template <unsigned P> const u32* stage_table(int m, int L, bool inv) {
 auto& tb= stage_tables<P>::t[inv ? 1 : 0][__builtin_ctz(unsigned(L))];
 const int h= L >> 2;
 if (tb.n != 6 * h) {
  tb.assign(6 * h);
  const u32 rl= root_of<P>(m, L, inv);
  const u32 c2= u32(u64(rl) * rl % P), c3= u32(u64(c2) * rl % P);
  u32 a1= 1, a2= 1, a3= 1;
  u32* tp= tb.p;
  for (int j= 0; j < h; ++j) {
   tp[j]= a1, tp[h + j]= a2, tp[2 * h + j]= a3;
   tp[3 * h + j]= shoup_q<P>(a1), tp[4 * h + j]= shoup_q<P>(a2), tp[5 * h + j]= shoup_q<P>(a3);
   a1= u32(u64(a1) * rl % P), a2= u32(u64(a2) * c2 % P), a3= u32(u64(a3) * c3 % P);
  }
 }
 return tb.p;
}

// 葉の重み w = η^{rev(i)} の表 (m 個) と商。
template <unsigned P> struct leaf_tables {
 static inline tbuf t[26];
};
// i の桁 (4 進、奇数指数では最後が 2 進) を逆にした数。
template <unsigned P> int rev_index(int i, int e) {
 if (!(e & 1)) {
  int r= 0, x= i;
  for (int t= 0; t < e / 2; ++t) r= (r << 2) | (x & 3), x>>= 2;
  return r;
 }
 const int k= (e - 1) / 2;
 const int b= i & 1;
 int q[16], x= i >> 1;
 for (int t= 0; t < k; ++t) q[t]= x & 3, x>>= 2;
 int r= 0;
 for (int t= 0; t < k; ++t) r|= q[k - 1 - t] << (2 * t);
 return r | (b << (2 * k));
}
template <unsigned P> const u32* leaf_table(int m) {
 const int e= __builtin_ctz(unsigned(m));
 auto& tb= leaf_tables<P>::t[e];
 if (tb.n != 2 * m) {
  tb.assign(2 * m);
  const u32 g= primitive_root<P>();
  const u32 eta= spow<P>(g, u64(P - 1) / unsigned(m));
  u32* w= tb.p;
  u32* q= tb.p + m;
  // w[i] = eta^{rev(i)} を、i を 1 進めるごとの比で作る (1 つずつ pow を取らない)。
  // 比は、i の末尾の最大桁 (4 進の 3、2 進なら 1) の数 s で決まる。
  if (!(e & 1)) {
   const int k= e / 2;
   std::vector<u32> step(k);
   for (int s= 0; s < k; ++s) {
    const u64 d1= u64(1) << (2 * (k - 1 - s)), d2= u64(1) << (2 * (k - s));
    step[s]= spow<P>(eta, (d1 + d2) % unsigned(m));
   }
   u32 cur= 1;
   w[0]= 1;
   for (int i= 0; i < m - 1; ++i) {
    int s= __builtin_ctz(~unsigned(i)) >> 1;
    if (s >= k) s= k - 1;
    cur= u32(u64(cur) * step[s] % P);
    w[i + 1]= cur;
   }
  } else {
   const int k= (e - 1) / 2;
   const u32 half= spow<P>(eta, u64(m) / 2);  // eta^{4^k} = -1。b が 0 から 1 に進むときの比
   std::vector<u32> step(k);
   for (int s= 0; s < k; ++s) {
    const u64 d1= u64(1) << (2 * (k - 1 - s)), d2= u64(1) << (2 * (k - s));
    step[s]= spow<P>(eta, (d1 + d2) % unsigned(m));
   }
   u32 cur= 1;
   w[0]= 1;
   for (int i= 0; i < m - 1; ++i) {
    if (!(i & 1)) cur= u32(u64(cur) * half % P);
    else {
     int s= __builtin_ctz(~unsigned(i >> 1)) >> 1;
     if (s >= k) s= k - 1;
     cur= u32(u64(cur) * step[s] % P);
    }
    w[i + 1]= cur;
   }
  }
  for (int i= 0; i < m; ++i) q[i]= shoup_q<P>(w[i]);
 }
 return tb.p;
}

// 段 (ベクトル数 4) はひねりがすべて 1 なので、掛け算を省く。
template <unsigned P> void stage4v_h1(u32* d, int nv, bool inv) {
 const u32 imn= inv ? ii_normal<P>() : im_normal<P>();
 const V im= vsplat<P>(imn), imq= vsplat<P>(shoup_q<P>(imn));
 for (int b= 0; b < nv; b+= 4) {
  u32* p= d + 8 * b;
  const V u0= vload<P>(p), u1= vload<P>(p + 8), u2= vload<P>(p + 16), u3= vload<P>(p + 24);
  if (!inv) {
   const V t0= vplus<P>(u0, u2), t2= vplus<P>(u1, u3);
   const V t1= vdiff<P>(u0, u2), t3= vdiff<P>(u1, u3);
   const V l0= vlow<P>(t0), l1= vlow<P>(t1), l2= vlow<P>(t2);
   const V pi= vshoups<P>(vlow<P>(t3), im, imq);
   vstore<P>(p, vlow<P>(vplus<P>(l0, l2)));
   vstore<P>(p + 8, vlow<P>(vplus<P>(l1, pi)));
   vstore<P>(p + 16, vlow<P>(vdiff<P>(l0, l2)));
   vstore<P>(p + 24, vlow<P>(vdiff<P>(l1, pi)));
  } else {
   const V y2= vlow<P>(u2);
   const V y1= vlow<P>(u1), y3= vlow<P>(u3);
   const V t0= vlow<P>(vplus<P>(u0, y2)), t2= vlow<P>(vdiff<P>(u0, y2));
   const V t1= vlow<P>(vplus<P>(y1, y3)), t3= vshoups<P>(vlow<P>(vdiff<P>(y1, y3)), im, imq);
   vstore<P>(p, vlow<P>(vplus<P>(t0, t1)));
   vstore<P>(p + 8, vlow<P>(vplus<P>(t2, t3)));
   vstore<P>(p + 16, vlow<P>(vdiff<P>(t0, t1)));
   vstore<P>(p + 24, vlow<P>(vdiff<P>(t2, t3)));
  }
 }
}

// 段 (ベクトル数 L) の順変換 (DIF) と逆変換 (DIT)。L は 4 以上。
template <unsigned P> void stage4v(u32* d, int nv, int m, int L, bool inv) {
 const int h= L >> 2;
 if (h == 1) {
  stage4v_h1<P>(d, nv, inv);
  return;
 }
 const u32* t= stage_table<P>(m, L, inv);
 const u32 *w1= t, *w2= t + h, *w3= t + 2 * h;
 const u32 *s1= t + 3 * h, *s2= t + 4 * h, *s3= t + 5 * h;
 const u32 imn= inv ? ii_normal<P>() : im_normal<P>();
 const V im= vsplat<P>(imn), imq= vsplat<P>(shoup_q<P>(imn));
 for (int b= 0; b < nv; b+= L) {
  u32* p= d + 8 * b;
  for (int j= 0; j < h; ++j) {
   // 2 つ先の j の 4 本を先に読む (段の内側は 4 本の列を h の間隔でなめる)。
   const u32* pf= p + 8 * (j + 2);
   _mm_prefetch(reinterpret_cast<const char*>(pf), _MM_HINT_T0), _mm_prefetch(reinterpret_cast<const char*>(pf + h), _MM_HINT_T0);
   _mm_prefetch(reinterpret_cast<const char*>(pf + 2 * h), _MM_HINT_T0), _mm_prefetch(reinterpret_cast<const char*>(pf + 3 * h), _MM_HINT_T0);
   const V W1= vsplat<P>(w1[j]), W2= vsplat<P>(w2[j]), W3= vsplat<P>(w3[j]);
   const V S1= vsplat<P>(s1[j]), S2= vsplat<P>(s2[j]), S3= vsplat<P>(s3[j]);
   if (!inv) {
    const V u0= vload<P>(p + 8 * j), u1= vload<P>(p + 8 * (j + h)), u2= vload<P>(p + 8 * (j + 2 * h)), u3= vload<P>(p + 8 * (j + 3 * h));
    const V t0= vplus<P>(u0, u2), t2= vplus<P>(u1, u3);
    const V t1= vdiff<P>(u0, u2), t3= vdiff<P>(u1, u3);
    const V l0= vlow<P>(t0), l1= vlow<P>(t1), l2= vlow<P>(t2);
    const V pi= vshoups<P>(vlow<P>(t3), im, imq);
    const V a1= vplus<P>(l1, pi), a3= vdiff<P>(l1, pi);
    vstore<P>(p + 8 * j, vlow<P>(vplus<P>(l0, l2)));
    vstore<P>(p + 8 * (j + h), vshoups<P>(a1, W1, S1));
    vstore<P>(p + 8 * (j + 2 * h), vshoups<P>(vlow<P>(vdiff<P>(l0, l2)), W2, S2));
    vstore<P>(p + 8 * (j + 3 * h), vshoups<P>(a3, W3, S3));
   } else {
    const V y1= vshoups<P>(vload<P>(p + 8 * (j + h)), W1, S1);
    const V y2= vshoups<P>(vload<P>(p + 8 * (j + 2 * h)), W2, S2);
    const V y3= vshoups<P>(vload<P>(p + 8 * (j + 3 * h)), W3, S3);
    const V u0= vload<P>(p + 8 * j);
    const V t0= vlow<P>(vplus<P>(u0, y2)), t2= vlow<P>(vdiff<P>(u0, y2));
    const V t1= vlow<P>(vplus<P>(y1, y3)), t3= vshoups<P>(vlow<P>(vdiff<P>(y1, y3)), im, imq);
    vstore<P>(p + 8 * j, vlow<P>(vplus<P>(t0, t1)));
    vstore<P>(p + 8 * (j + h), vlow<P>(vplus<P>(t2, t3)));
    vstore<P>(p + 8 * (j + 2 * h), vlow<P>(vdiff<P>(t0, t1)));
    vstore<P>(p + 8 * (j + 3 * h), vlow<P>(vdiff<P>(t2, t3)));
   }
  }
 }
}

// 段 (ベクトル数 L) の、長さをコンパイル時に決めた版 (タイルの中の段)。
template <unsigned P, int L> void stage4v_c(u32* d, int nv, int m, bool inv) {
 constexpr int h= L >> 2;
 if constexpr (h == 1) {
  stage4v_h1<P>(d, nv, inv);
  return;
 }
 const u32* t= stage_table<P>(m, L, inv);
 const u32 *w1= t, *w2= t + h, *w3= t + 2 * h;
 const u32 *s1= t + 3 * h, *s2= t + 4 * h, *s3= t + 5 * h;
 const u32 imn= inv ? ii_normal<P>() : im_normal<P>();
 const V im= vsplat<P>(imn), imq= vsplat<P>(shoup_q<P>(imn));
 for (int b= 0; b < nv; b+= L) {
  u32* p= d + 8 * b;
#pragma GCC unroll 2
  for (int j= 0; j < h; ++j) {
   // 2 つ先の j の 4 本を先に読む (段の内側は 4 本の列を h の間隔でなめる)。
   const u32* pf= p + 8 * (j + 2);
   _mm_prefetch(reinterpret_cast<const char*>(pf), _MM_HINT_T0), _mm_prefetch(reinterpret_cast<const char*>(pf + h), _MM_HINT_T0);
   _mm_prefetch(reinterpret_cast<const char*>(pf + 2 * h), _MM_HINT_T0), _mm_prefetch(reinterpret_cast<const char*>(pf + 3 * h), _MM_HINT_T0);
   const V W1= vsplat<P>(w1[j]), W2= vsplat<P>(w2[j]), W3= vsplat<P>(w3[j]);
   const V S1= vsplat<P>(s1[j]), S2= vsplat<P>(s2[j]), S3= vsplat<P>(s3[j]);
   if (!inv) {
    const V u0= vload<P>(p + 8 * j), u1= vload<P>(p + 8 * (j + h)), u2= vload<P>(p + 8 * (j + 2 * h)), u3= vload<P>(p + 8 * (j + 3 * h));
    const V t0= vplus<P>(u0, u2), t2= vplus<P>(u1, u3);
    const V t1= vdiff<P>(u0, u2), t3= vdiff<P>(u1, u3);
    const V l0= vlow<P>(t0), l1= vlow<P>(t1), l2= vlow<P>(t2);
    const V pi= vshoups<P>(vlow<P>(t3), im, imq);
    const V a1= vplus<P>(l1, pi), a3= vdiff<P>(l1, pi);
    vstore<P>(p + 8 * j, vlow<P>(vplus<P>(l0, l2)));
    vstore<P>(p + 8 * (j + h), vshoups<P>(a1, W1, S1));
    vstore<P>(p + 8 * (j + 2 * h), vshoups<P>(vlow<P>(vdiff<P>(l0, l2)), W2, S2));
    vstore<P>(p + 8 * (j + 3 * h), vshoups<P>(a3, W3, S3));
   } else {
    const V y1= vshoups<P>(vload<P>(p + 8 * (j + h)), W1, S1);
    const V y2= vshoups<P>(vload<P>(p + 8 * (j + 2 * h)), W2, S2);
    const V y3= vshoups<P>(vload<P>(p + 8 * (j + 3 * h)), W3, S3);
    const V u0= vload<P>(p + 8 * j);
    const V t0= vlow<P>(vplus<P>(u0, y2)), t2= vlow<P>(vdiff<P>(u0, y2));
    const V t1= vlow<P>(vplus<P>(y1, y3)), t3= vshoups<P>(vlow<P>(vdiff<P>(y1, y3)), im, imq);
    vstore<P>(p + 8 * j, vlow<P>(vplus<P>(t0, t1)));
    vstore<P>(p + 8 * (j + h), vlow<P>(vplus<P>(t2, t3)));
    vstore<P>(p + 8 * (j + 2 * h), vlow<P>(vdiff<P>(t0, t1)));
    vstore<P>(p + 8 * (j + 3 * h), vlow<P>(vdiff<P>(t2, t3)));
   }
  }
 }
}

// 段 (ベクトル数 2) の順変換と逆変換。ひねりは 1。
template <unsigned P> void stage2v(u32* d, int nv) {
 for (int b= 0; b < nv; b+= 2) {
  const V u0= vload<P>(d + 8 * b), u1= vload<P>(d + 8 * (b + 1));
  vstore<P>(d + 8 * b, vlow<P>(vplus<P>(u0, u1)));
  vstore<P>(d + 8 * (b + 1), vlow<P>(vdiff<P>(u0, u1)));
 }
}

// タイル (NV ベクトル) の段の列。L = NV, NV/4, …, 4 (2 の指数が奇数なら 8 のあと 2)。
template <unsigned P, int NV, int L> struct tile_seq {
 static void fwd(u32* d, int m) { stage4v_c<P, L>(d, NV, m, false); tile_seq<P, NV, L / 4>::fwd(d, m); }
 static void inv(u32* d, int m) { tile_seq<P, NV, L / 4>::inv(d, m); stage4v_c<P, L>(d, NV, m, true); }
};
template <unsigned P, int NV> struct tile_seq<P, NV, 1> {
 static void fwd(u32*, int) {}
 static void inv(u32*, int) {}
};
template <unsigned P, int NV> struct tile_seq<P, NV, 2> {
 static void fwd(u32* d, int) { stage2v<P>(d, NV); }
 static void inv(u32* d, int) { stage2v<P>(d, NV); }
};
template <unsigned P, int NV> struct tile_seq<P, NV, 4> {
 static void fwd(u32* d, int) { stage4v_h1<P>(d, NV, false); }
 static void inv(u32* d, int) { stage4v_h1<P>(d, NV, true); }
};

// 葉の積 (mod x^8 - w)。窓 [w a, a] を 16 個の作業に置き、8 回の読み出しで回転を作る。
// 窓のストアと読み出しが近いと待つので、呼び出し側で 1 つ先の窓を作ってから今の積を取る。
// 積は Montgomery の MAC で、R^{-1} が 1 回付く。奇数レーンの積は 1 つ前の窓の読み出しの偶数レーンを使う。
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

// タイル (NV ベクトル) を、順変換の下の段、葉の積、逆変換の下の段まで回す。段の長さはコンパイル時に決まる。
template <unsigned P, int NV> void tile_work_c(u32* fa, u32* fb, int m, int B, bool square, V sc, V scq) {
 tile_seq<P, NV, NV>::fwd(fa, m);
 if (!square) tile_seq<P, NV, NV>::fwd(fb, m);
 const u32* lw= leaf_table<P>(m);
 const u32* lq= lw + m;
 // 窓は 1 つ先に作る (ストアした窓をすぐ読むと待つため)。
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
 tile_seq<P, NV, NV>::inv(fa, m);
}

// タイル (nv ベクトル、全体では m ベクトル、先頭はベクトル B) を回す。nv は 16, 32, …, 256 のどれか。
template <unsigned P> void tile_work(u32* fa, u32* fb, int m, int nv, int B, bool square, V sc, V scq) {
 switch (nv) {
  case 16: tile_work_c<P, 16>(fa, fb, m, B, square, sc, scq); break;
  case 32: tile_work_c<P, 32>(fa, fb, m, B, square, sc, scq); break;
  case 64: tile_work_c<P, 64>(fa, fb, m, B, square, sc, scq); break;
  case 128: tile_work_c<P, 128>(fa, fb, m, B, square, sc, scq); break;
  case 256: tile_work_c<P, 256>(fa, fb, m, B, square, sc, scq); break;
  default: std::abort();
 }
}

// 深さ優先で区画を回す。
template <unsigned P> void rec(u32* fa, u32* fb, int nv, int m, int B, bool square, V sc, V scq) {
 if (nv <= tile_vec) {
  tile_work<P>(fa, fb, m, nv, B, square, sc, scq);
  return;
 }
 const int h= nv >> 2;
 if (square) stage4v<P>(fa, nv, m, nv, false);
 else stage4v<P>(fa, nv, m, nv, false), stage4v<P>(fb, nv, m, nv, false);
 for (int t= 0; t < 4; ++t) rec<P>(fa + 8 * t * h, fb + 8 * t * h, h, m, B + t * h, square, sc, scq);
 stage4v<P>(fa, nv, m, nv, true);
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
  rec<P>(fa, fb, mv, mv, 0, square, vsplat<P>(c8), vsplat<P>(shoup_q<P>(c8)));
  for (int k= lo; k < hi; ++k) out[k - lo]= ZMod<P>::raw(fa[k]);
  return;
 }
 Naive::convolve_range(a, n, b, m_, lo, hi, out);
}
}  // namespace ntt_splat_pf_internal

// 5 本目の核。畳み込みは上の道、素の変換は 3 本目のものを使う。
struct NttAvx2SplatPf {
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  if constexpr (ntt_avx2_internal::static_ntt_mod<K>::value) {
   ntt_splat_pf_internal::convolve_entry<ntt_avx2_internal::static_ntt_mod<K>::mod>(a, n, b, m, lo, hi, out);
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
