#pragma once
// 単独の NTT の核の 4 本目 (実験版)。畳み込みを 1 本のパイプラインで回す。
// 3 本目 (ntt_avx2.hpp) との違いは 3 つ。
//  1. ひねりは Shoup の方法。値はふつうの表現のまま持ち、表に w と floor(w 2^32 / P) を持つ。
//  2. 点ごとの積をタイルの中で取る。タイルの中は L1 に載せたまま、順変換の下の段、8 次の葉の積、
//     逆変換の下の段を続けて回す。タイルの外の段は、NttAvx2Fused が深さ優先の再帰で区画ごとに回し
//     (子の区画がキャッシュに残る)、NttAvx2FusedFlat が配列全体をなめる。
//  3. 下の 8 次の段 (長さ 8 と 4 と 2 の段) を、8 次の巡回畳み込み (葉の積) に置き換える。
//     葉の積は Montgomery の MAC なので R^{-1} が 1 回付き、8 R / l を葉の中で掛けて戻す。
// dft / idft / dft_doubling / even_dft / odd_dft は 3 本目のものを使う (この実験は畳み込みの口だけを見る)。
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

namespace ntt_fused_internal {
using u32= unsigned;
using u64= unsigned long long;
using V= __m256i;
using ntt_avx2_internal::inv_n_scalar;
using ntt_avx2_internal::mc;
using ntt_avx2_internal::primitive_root;
using ntt_avx2_internal::spow;

// タイルの長さの上限 (要素数)。L1 に 2 本載る大きさ。
constexpr int tile_max= 2048;

template <unsigned P> V vsplat(u32 x) { return _mm256_set1_epi32(int(x)); }
template <unsigned P> V vload(const u32* p) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p)); }
template <unsigned P> void vstore(u32* p, V x) { _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), x); }
template <unsigned P> V vplus(V x, V y) { return _mm256_add_epi32(x, y); }
template <unsigned P> V vdiff(V x, V y) { return _mm256_sub_epi32(_mm256_add_epi32(x, vsplat<P>(mc<P>::p2)), y); }
template <unsigned P> V vlow(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, vsplat<P>(mc<P>::p2))); }
template <unsigned P> V vcanon(V x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, vsplat<P>(P))); }

// 8 レーンの下位 32 bit の積。本物の vpmulld は下位 32 bit だけで、符号を問わない。
// SIMDe の mullo は符号付きで掛けて UBSan に鳴るので、SIMDe のときは mul_epu32 2 本で作る。
#if defined(__AVX2__) && !defined(USE_SIMDE)
template <unsigned P> V vpmulld(V a, V b) { return _mm256_mullo_epi32(a, b); }
#else
template <unsigned P> V vpmulld(V a, V b) {
 const V e= _mm256_mul_epu32(a, b);
 const V o= _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
 // o の下位 32 bit は偶数レーンに入っているので、64 bit ごとに 32 上がげてから混ぜる。
 return _mm256_blend_epi32(e, _mm256_slli_epi64(o, 32), 0xAA);
}
#endif

// Shoup の掛け算。x < 2^32、w < P、wi = floor(w 2^32 / P)。結果は 2P 未満。
// q = floor(x wi / 2^32) を 32 bit ごとに作り、x w - q P を下位 32 bit で取る。
template <unsigned P> V vshoup(V x, V w, V wi) {
 const V pv= vsplat<P>(P);
 const V qe= _mm256_mul_epu32(x, wi);
 const V qo= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(wi, 32));
 // qo は 64 bit のレーンの上半分に奇数レーンの商がそのまま入るので、右にずらさない。
 const V q= _mm256_blend_epi32(_mm256_srli_epi64(qe, 32), qo, 0xAA);
 return _mm256_sub_epi32(vpmulld<P>(x, w), vpmulld<P>(q, pv));
}
// w と wi が全レーン同じとき (4 乗根、1 か所の定数)。右にずらすのが 1 つ減る。
template <unsigned P> V vshoups(V x, V w, V wi) {
 const V pv= vsplat<P>(P);
 const V qe= _mm256_mul_epu32(x, wi);
 const V qo= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), wi);
 // qo は 64 bit のレーンの上半分に奇数レーンの商がそのまま入るので、右にずらさない。
 const V q= _mm256_blend_epi32(_mm256_srli_epi64(qe, 32), qo, 0xAA);
 return _mm256_sub_epi32(vpmulld<P>(x, w), vpmulld<P>(q, pv));
}

// floor(w 2^32 / P)。P が定数なので、コンパイラが掛け算と移しに直す。
template <unsigned P> u32 shoup_q(u32 w) { return u32((u64(w) << 32) / P); }

// 4 乗根 (順と逆、ふつうの値)。
template <unsigned P> u32 im_normal() {
 static const u32 v= spow<P>(primitive_root<P>(), u64(P - 1) / 4);
 return v;
}
template <unsigned P> u32 ii_normal() {
 static const u32 v= spow<P>(primitive_root<P>(), u64(P - 1) - u64(P - 1) / 4);
 return v;
}

// 段のひねりの表。長さ len の基数 4 の段は h = len/4 で、
// 前半 3h が w (ω_len^j、ω_len^{2j}、ω_len^{3j})、後半 3h がその Shoup の商。
// 長さと向きごとに 1 回だけ作って残す。
// 表の置き場。32 byte にそろえて、段の読み込みがキャッシュ線をまたがないようにする。
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
template <unsigned P> struct table_cache {
 static inline tbuf r4[2][26];
 static inline tbuf r2[2][26];
};
template <unsigned P> const u32* get_r4(int len, bool inv) {
 auto& t= table_cache<P>::r4[inv ? 1 : 0][__builtin_ctz(unsigned(len))];
 const int h= len >> 2;
 if (t.n != 6 * h) {
  t.assign(6 * h);
  const u32 g= primitive_root<P>();
  const u32 wl= spow<P>(g, inv ? u64(P - 1) - u64(P - 1) / unsigned(len) : u64(P - 1) / unsigned(len));
  const u32 c2= u32(u64(wl) * wl % P), c3= u32(u64(c2) * wl % P);
  u32 a1= 1, a2= 1, a3= 1;
  u32* tp= t.p;
  for (int j= 0; j < h; ++j) {
   tp[j]= a1, tp[h + j]= a2, tp[2 * h + j]= a3;
   tp[3 * h + j]= shoup_q<P>(a1), tp[4 * h + j]= shoup_q<P>(a2), tp[5 * h + j]= shoup_q<P>(a3);
   a1= u32(u64(a1) * wl % P), a2= u32(u64(a2) * c2 % P), a3= u32(u64(a3) * c3 % P);
  }
 }
 return t.p;
}
// 基数 2 の段 (長さ len、h = len/2) のひねり。前半 h が w、後半 h が商。
template <unsigned P> const u32* get_r2(int len, bool inv) {
 auto& t= table_cache<P>::r2[inv ? 1 : 0][__builtin_ctz(unsigned(len))];
 const int h= len >> 1;
 if (t.n != 2 * h) {
  t.assign(2 * h);
  const u32 g= primitive_root<P>();
  const u32 wl= spow<P>(g, inv ? u64(P - 1) - u64(P - 1) / unsigned(len) : u64(P - 1) / unsigned(len));
  u32 a= 1;
  u32* tp= t.p;
  for (int j= 0; j < h; ++j) tp[j]= a, tp[h + j]= shoup_q<P>(a), a= u32(u64(a) * wl % P);
 }
 return t.p;
}

// 基数 4 の段 (h = len/4 が 8 以上)。順変換 (inv = false) は DIF、逆は DIT。
// 値は 2P 未満で保つ。和と差は 4P 未満まで許し、Shoup の入口で 2P 未満に落とす。
template <unsigned P> void stage4(u32* d, int n, int len, bool inv) {
 const int h= len >> 2, hv= len >> 5;
 const u32* t= get_r4<P>(len, inv);
 const u32 *w1= t, *w2= t + h, *w3= t + 2 * h;
 const u32 *s1= t + 3 * h, *s2= t + 4 * h, *s3= t + 5 * h;
 const u32 imn= inv ? ii_normal<P>() : im_normal<P>();
 const V im= vsplat<P>(imn), imq= vsplat<P>(shoup_q<P>(imn));
 for (int b= 0; b < n; b+= len) {
  u32* q= d + b;
  for (int jv= 0; jv < hv; ++jv) {
   u32* p= q + (jv << 3);
   const int j= jv << 3;
   const V W1= vload<P>(w1 + j), W2= vload<P>(w2 + j), W3= vload<P>(w3 + j);
   const V S1= vload<P>(s1 + j), S2= vload<P>(s2 + j), S3= vload<P>(s3 + j);
   if (!inv) {
    const V u0= vload<P>(p), u1= vload<P>(p + h), u2= vload<P>(p + 2 * h), u3= vload<P>(p + 3 * h);
    const V t0= vplus<P>(u0, u2), t2= vplus<P>(u1, u3);
    const V t1= vdiff<P>(u0, u2), t3= vdiff<P>(u1, u3);
    const V l0= vlow<P>(t0), l1= vlow<P>(t1), l2= vlow<P>(t2);
    const V pi= vshoups<P>(vlow<P>(t3), im, imq);
    const V a1= vplus<P>(l1, pi), a3= vdiff<P>(l1, pi);
    vstore<P>(p, vlow<P>(vplus<P>(l0, l2)));
    vstore<P>(p + h, vshoup<P>(a1, W1, S1));
    vstore<P>(p + 2 * h, vshoup<P>(vlow<P>(vdiff<P>(l0, l2)), W2, S2));
    vstore<P>(p + 3 * h, vshoup<P>(a3, W3, S3));
   } else {
    const V y1= vshoup<P>(vload<P>(p + h), W1, S1);
    const V y2= vshoup<P>(vload<P>(p + 2 * h), W2, S2);
    const V y3= vshoup<P>(vload<P>(p + 3 * h), W3, S3);
    const V u0= vload<P>(p);
    const V t0= vlow<P>(vplus<P>(u0, y2)), t2= vlow<P>(vdiff<P>(u0, y2));
    const V t1= vlow<P>(vplus<P>(y1, y3)), t3= vshoups<P>(vlow<P>(vdiff<P>(y1, y3)), im, imq);
    vstore<P>(p, vlow<P>(vplus<P>(t0, t1)));
    vstore<P>(p + h, vlow<P>(vplus<P>(t2, t3)));
    vstore<P>(p + 2 * h, vlow<P>(vdiff<P>(t0, t1)));
    vstore<P>(p + 3 * h, vlow<P>(vdiff<P>(t2, t3)));
   }
  }
 }
}

// 基数 2 の段 (長さ 16、h = 8)。タイルの指数が偶数のときに、16 個の塊を 8 個に割る。
// 段ごとの 1/2 は掛けない。
template <unsigned P> void stage2_16(u32* d, int n, bool inv) {
 const u32* t= get_r2<P>(16, inv);
 const V W= vload<P>(t), S= vload<P>(t + 8);
 for (int b= 0; b < n; b+= 16) {
  const V u0= vload<P>(d + b), u1= vload<P>(d + b + 8);
  if (!inv) {
   vstore<P>(d + b, vlow<P>(vplus<P>(u0, u1)));
   vstore<P>(d + b + 8, vshoup<P>(vdiff<P>(u0, u1), W, S));
  } else {
   const V y1= vshoup<P>(u1, W, S);
   vstore<P>(d + b, vlow<P>(vplus<P>(u0, y1)));
   vstore<P>(d + b + 8, vlow<P>(vdiff<P>(u0, y1)));
  }
 }
}

// 8 次の巡回畳み込み (mod x^8 - 1)。c[j] = Σ_i b[i] a[(j-i) mod 8]。
// 8 レーンは葉の 8 個の係数で、回転は vpermd で作る。積は 64 bit に貯めて最後に 1 回 Montgomery で還元する
// (R^{-1} が 1 回付く)。奇数レーンの積は、1 つ前の回転の偶数レーンをそのまま使う。
// 回転の添字 (i = 1..7 について、レーン j が読む位置は (j - i) mod 8)。定数なので実行時の初期化はしない。
struct rot_table {
 u32 v[56];
 constexpr rot_table(): v() {
  for (int i= 1; i < 8; ++i)
   for (int j= 0; j < 8; ++j) v[8 * (i - 1) + j]= u32((j - i) & 7);
 }
};
template <unsigned P> inline constexpr rot_table rot_tab{};
template <unsigned P> V leaf_mul(V ya, V yb) {
 const V pv= vsplat<P>(P), niv= vsplat<P>(mc<P>::ni);
 alignas(32) u32 cb[8];
 vstore<P>(cb, yb);
 const u32* idx= rot_tab<P>.v;
 V prev= ya;
 V e, o;
 {
  const V y0= vsplat<P>(cb[0]);
  e= _mm256_mul_epu32(ya, y0);
  o= _mm256_mul_epu32(_mm256_srli_epi64(ya, 32), y0);
 }
 for (int i= 1; i < 8; ++i) {
  const V x= _mm256_permutevar8x32_epi32(ya, vload<P>(idx + 8 * (i - 1)));
  const V yi= vsplat<P>(cb[i]);
  e= _mm256_add_epi64(e, _mm256_mul_epu32(x, yi));
  o= _mm256_add_epi64(o, _mm256_mul_epu32(prev, yi));
  prev= x;
 }
 e= _mm256_add_epi64(e, _mm256_mul_epu32(_mm256_mul_epu32(e, niv), pv));
 o= _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_mul_epu32(o, niv), pv));
 // o の結果は 64 bit のレーンの上半分にそのまま入っている (奇数レーン) ので、右にずらさない。
 return _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xAA);
}

// タイルの中の順変換の下の段。8 個の塊まで割って止める。
template <unsigned P> void tile_forward(u32* d, int T) {
 const bool odd= __builtin_ctz(unsigned(T)) & 1;
 int len= T;
 for (; len >= 32; len>>= 2) stage4<P>(d, T, len, false);
 if (!odd) stage2_16<P>(d, T, false);
}
// タイルの中の逆変換の下の段。葉の積のあと、8 個の塊から戻す。
template <unsigned P> void tile_inverse(u32* d, int T) {
 const bool odd= __builtin_ctz(unsigned(T)) & 1;
 int len;
 if (odd) len= 32;
 else stage2_16<P>(d, T, true), len= 64;
 for (; len <= T; len<<= 2) stage4<P>(d, T, len, true);
}

// タイルの長さ。l を 4 で割っていって tile_max 以下にした値 (段の長さと揃える)。
constexpr int tile_len(int l) {
 int t= l;
 while (t > tile_max) t>>= 2;
 return t;
}

// 1 つのタイル (n = T 要素) を、順変換の下の段、葉の積、逆変換の下の段まで続けて回す。
template <unsigned P> void tile_work(u32* fa, u32* fb, int T, bool square, V sc, V scq) {
 tile_forward<P>(fa, T);
 if (!square) tile_forward<P>(fb, T);
 for (int i= 0; i < T; i+= 8) {
  // 葉の積の入力は P 未満に直す (8 個の 64 bit の和を R P 未満に収め、還元の結果を 2^32 未満にするため)。
  // 葉の積が 8 次の巡回畳み込み (正規化なし) で R^{-1} が付き、残りの逆の段が l/8 倍なので、ここで 8 R / l を掛ける。
  const V ya= vcanon<P>(vload<P>(fa + i));
  if (square) vstore<P>(fa + i, vlow<P>(vshoups<P>(leaf_mul<P>(ya, ya), sc, scq)));
  else vstore<P>(fa + i, vlow<P>(vshoups<P>(leaf_mul<P>(ya, vcanon<P>(vload<P>(fb + i))), sc, scq)));
 }
 tile_inverse<P>(fa, T);
}

// 長さ n (要素数、2 の冪) の区画を、深さ優先で回す。区画の段を順に掛け、4 つの子を回してから、
// 同じ段の逆を掛ける。こうすると、各段の順と逆の間で子の区画がキャッシュに残る。
template <unsigned P> void fused_rec(u32* fa, u32* fb, int n, int T, bool square, V sc, V scq) {
 if (n <= T) {
  tile_work<P>(fa, fb, n, square, sc, scq);
  return;
 }
 stage4<P>(fa, n, n, false);
 if (!square) stage4<P>(fb, n, n, false);
 const int h= n >> 2;
 for (int t= 0; t < 4; ++t) fused_rec<P>(fa + t * h, fb + t * h, h, T, square, sc, scq);
 stage4<P>(fa, n, n, true);
}

// fa[0..l) と fb[0..l) から、巡回の畳み込みを fa に作る (長さ l、2 の冪)。深さ優先で区画を回す。
// square のときは fb を使わず、fa の 2 乗にする。
template <unsigned P> void convolve_fused(u32* fa, u32* fb, int l, bool square) {
 const int T= tile_len(l);
 const u32 c= u32(u64(u64(8) * mc<P>::r % P) * inv_n_scalar<P>(l) % P);
 const V sc= vsplat<P>(c), scq= vsplat<P>(shoup_q<P>(c));
 fused_rec<P>(fa, fb, l, T, square, sc, scq);
}
// 同じ入力で、外の段を配列全体のなめで回す形 (区画を深さ優先にしない)。比べるために残す。
template <unsigned P> void convolve_fused_flat(u32* fa, u32* fb, int l, bool square) {
 const int T= tile_len(l);
 const u32 c= u32(u64(u64(8) * mc<P>::r % P) * inv_n_scalar<P>(l) % P);
 const V sc= vsplat<P>(c), scq= vsplat<P>(shoup_q<P>(c));
 for (int len= l; len > T; len>>= 2) {
  stage4<P>(fa, l, len, false);
  if (!square) stage4<P>(fb, l, len, false);
 }
 for (int b= 0; b < l; b+= T) tile_work<P>(fa + b, fb + b, T, square, sc, scq);
 for (int len= T << 2; len <= l; len<<= 2) stage4<P>(fa, l, len, true);
}

// 作業の配列。呼び出しをまたいで残す (タイルの段は小さい呼び出しで何度も来るため)。
// 判定は 1 本の糸で回るので、ここは静的でよい。スレッドを分けるときは注意。
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
// 畳み込みの入口。deep で外の段の回し方 (深さ優先か、配列全体のなめか) を選ぶ。
template <unsigned P> void convolve_entry(const ZMod<P>* a, int n, const ZMod<P>* b, int m, int lo, int hi, ZMod<P>* out, bool deep) {
 // 巡回の長さは L >= max(hi, n + m - 1 - lo) でよい (折り返した項が [lo, hi) に入らない)。
 const int need= std::max(hi, n + m - 1 - lo);
 int l= 1;
 while (l < need) l<<= 1;
 if (std::min(n, m) >= 60 && l >= 128 && (P - 1) % unsigned(l) == 0) {
  const bool square= (a == b && n == m);
  work_buf<P>::reserve(l);
  u32* fa= work_buf<P>::fa;
  u32* fb= work_buf<P>::fb;
  std::copy(reinterpret_cast<const u32*>(a), reinterpret_cast<const u32*>(a) + n, fa), std::fill(fa + n, fa + l, 0);
  if (!square) std::copy(reinterpret_cast<const u32*>(b), reinterpret_cast<const u32*>(b) + m, fb), std::fill(fb + m, fb + l, 0);
  if (deep) convolve_fused<P>(fa, fb, l, square);
  else convolve_fused_flat<P>(fa, fb, l, square);
  for (int k= lo; k < hi; ++k) out[k - lo]= ZMod<P>::raw(fa[k]);
  return;
 }
 Naive::convolve_range(a, n, b, m, lo, hi, out);
}
}  // namespace ntt_fused_internal

// 4 本目の核。畳み込みは融合した道、素の変換は 3 本目のものを使う。
// NttAvx2Fused は外の段を深さ優先で回し (子の区画がキャッシュに残る)、NttAvx2FusedFlat は配列全体をなめる。
struct NttAvx2Fused {
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  if constexpr (ntt_avx2_internal::static_ntt_mod<K>::value) {
   ntt_fused_internal::convolve_entry<ntt_avx2_internal::static_ntt_mod<K>::mod>(a, n, b, m, lo, hi, out, true);
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

// 外の段を配列全体のなめで回す版 (深さ優先と比べるため)。
struct NttAvx2FusedFlat {
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  if constexpr (ntt_avx2_internal::static_ntt_mod<K>::value) {
   ntt_fused_internal::convolve_entry<ntt_avx2_internal::static_ntt_mod<K>::mod>(a, n, b, m, lo, hi, out, false);
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
