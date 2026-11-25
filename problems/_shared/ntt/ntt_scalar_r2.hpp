#pragma once
// 単独の NTT の核の 1 本目 (実験版)。スカラー、基数 2。
// dft は DIF で bit 反転の並べ替えをせず、index i に ω^{bitrev(i)} の評価を置く。idft は DIT でその逆。
// 口は algo-notes の notes/ntt-fps.md の「核が出す関数」と「素の変換」に合わせてある:
//   convolve_range<K>(a, n, b, m, lo, hi, out)、dft<P>、idft<P>、dft_doubling<P>、even_dft<P>、odd_dft<P>。
// 小さい積と、NTT に使えない係数の型 (ntt_friendly でない素数、実行時の法 ZMod<0>、ZMod 以外) は Naive に回す。
// 置き場は実験用の procon-judge の _shared。採る方式が決まったら NeoLibrary の neo/base/core/convolve/ntt.hpp へ移す。
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/naive.hpp"
#include "_shared/ntt/_kernel_base.hpp"

namespace ntt_scalar_r2_internal {
using u32= unsigned;
using u64= unsigned long long;

template <unsigned P> ZMod<P> pow_mod(ZMod<P> a, u64 e) {
 ZMod<P> r= 1;
 while (e) {
  if (e & 1) r*= a;
  a*= a, e>>= 1;
 }
 return r;
}

// mod は素数。mod - 1 の素因数を集め、どの p でも g^((mod-1)/p) != 1 となる最小の g を返す。
template <unsigned P> const ZMod<P>& primitive_root() {
 static const ZMod<P> g= [] {
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
    if (pow_mod(ZMod<P>(g), (P - 1) / p) == ZMod<P>(1)) {
     ok= false;
     break;
    }
   if (ok) return ZMod<P>(g);
  }
 }();
 return g;
}

// DIF。a[i] に Σ_j a[j] ω^{j bitrev(i)} が入る。n は 2 冪で、n | P - 1。
template <unsigned P> void dft_impl(ZMod<P>* a, int n) {
 if (n <= 1) return;
 assert((n & (n - 1)) == 0 && (P - 1) % unsigned(n) == 0);
 const ZMod<P> g= primitive_root<P>();
 for (int len= n; len >= 2; len>>= 1) {
  const int h= len>> 1;
  const ZMod<P> wlen= pow_mod(g, (P - 1) / unsigned(len));
  for (int i= 0; i < n; i+= len) {
   ZMod<P> w= 1;
   for (int j= 0; j < h; ++j) {
    const ZMod<P> u= a[i + j], v= a[i + j + h];
    a[i + j]= u + v;
    a[i + j + h]= (u - v) * w;
    w*= wlen;
   }
  }
 }
}

// DIT。bit 反転の順の入力を係数の順に戻し、1/n を掛ける。
template <unsigned P> void idft_impl(ZMod<P>* a, int n) {
 if (n <= 1) return;
 assert((n & (n - 1)) == 0 && (P - 1) % unsigned(n) == 0);
 const ZMod<P> g= primitive_root<P>();
 for (int len= 2; len <= n; len<<= 1) {
  const int h= len>> 1;
  const ZMod<P> wlen= pow_mod(g, u64(P - 1) - u64(P - 1) / unsigned(len));
  for (int i= 0; i < n; i+= len) {
   ZMod<P> w= 1;
   for (int j= 0; j < h; ++j) {
    const ZMod<P> u= a[i + j], v= a[i + j + h] * w;
    a[i + j]= u + v;
    a[i + j + h]= u - v;
    w*= wlen;
   }
  }
 }
 const ZMod<P> iv= ZMod<P>(n).inv();
 for (int i= 0; i < n; ++i) a[i]*= iv;
}

// a[0..n) は長さ n の変換。係数に戻して 0 を詰め、長さ 2n の変換を a[0..2n) に作る。
template <unsigned P> void dft_doubling_impl(ZMod<P>* a, int n) {
 std::vector<ZMod<P>> c(a, a + n);
 idft_impl<P>(c.data(), n);
 std::fill(a, a + 2 * n, ZMod<P>());
 std::copy(c.begin(), c.end(), a);
 dft_impl<P>(a, 2 * n);
}

// 長さ n の変換から、偶数次の部分の長さ n/2 の変換を a[0..n/2) に作る (ライブラリの even_dft と同じ意味)。
template <unsigned P> void even_dft_impl(ZMod<P>* a, int n) {
 const ZMod<P> iv2= ZMod<P>(2).inv();
 for (int i= 0, j= 0; i < n; i+= 2) a[j++]= (a[i] + a[i + 1]) * iv2;
}

// 長さ n の変換から、奇数次の部分の長さ n/2 の変換を a[0..n/2) に作る。j 番目の出力のひねりは ω^{-bitrev(j)}。
template <unsigned P> void odd_dft_impl(ZMod<P>* a, int n) {
 const int h= n>> 1;
 const ZMod<P> iv2= ZMod<P>(2).inv();
 const ZMod<P> wi= pow_mod(primitive_root<P>(), u64(P - 1) - u64(P - 1) / unsigned(n));
 std::vector<ZMod<P>> pw(h);
 pw[0]= 1;
 for (int k= 1; k < h; ++k) pw[k]= pw[k - 1] * wi;
 const int bits= __builtin_ctz(unsigned(h));
 for (int j= 0; j < h; ++j) {
  int q= 0;
  for (int b= 0, x= j; b < bits; ++b, x>>= 1) q= (q << 1) | (x & 1);
  const ZMod<P> u= a[2 * j], v= a[2 * j + 1];
  a[j]= (u - v) * iv2 * pw[q];
 }
}

template <class K> struct static_ntt_mod: std::false_type {
 static constexpr unsigned mod= 0;
};
template <unsigned P> struct static_ntt_mod<ZMod<P>>: std::bool_constant<ntt_friendly<P>> {
 static constexpr unsigned mod= P;
};
}  // namespace ntt_scalar_r2_internal

struct Ntt {
 // out[k - lo] = Σ_{i+j=k} a[i] b[j] (lo <= k < hi)。n >= 1、m >= 1、0 <= lo < hi <= n + m - 1。
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  if constexpr (ntt_scalar_r2_internal::static_ntt_mod<K>::value) {
   constexpr unsigned P= ntt_scalar_r2_internal::static_ntt_mod<K>::mod;
   int l= 1;
   while (l < n + m - 1) l<<= 1;
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

 template <unsigned P> requires ntt_friendly<P> static void dft(ZMod<P>* a, int n) { ntt_scalar_r2_internal::dft_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void idft(ZMod<P>* a, int n) { ntt_scalar_r2_internal::idft_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void dft_doubling(ZMod<P>* a, int n) { ntt_scalar_r2_internal::dft_doubling_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void even_dft(ZMod<P>* a, int n) { ntt_scalar_r2_internal::even_dft_impl<P>(a, n); }
 template <unsigned P> requires ntt_friendly<P> static void odd_dft(ZMod<P>* a, int n) { ntt_scalar_r2_internal::odd_dft_impl<P>(a, n); }
};

// 既定の核。ntt_friendly な素数では単独の NTT を選ぶ。宣言だけで、定義しない。
template <unsigned P> requires ntt_friendly<P> Ntt default_kernel(const ZMod<P>*);
