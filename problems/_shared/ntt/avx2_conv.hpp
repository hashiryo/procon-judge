#pragma once
// 実験中の AVX2 の核を、係数の型で振り分ける核。ntt_friendly な素数 (2^20 までの変換を持つ) は
// 単独の NTT (NttAvx2)、それ以外の法 (1e9+7 のような素数、実行時の法) は多素数の CRT
// (crt_engine<NttAvx2>) で畳み込む。hurwitz や fps の核の引数にこれを渡す。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/convolve/crt.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

namespace avx2_conv_internal {
template <class K> struct pick {
 using type= crt_engine<NttAvx2>;
};
template <unsigned P> requires ntt_internal::ntt_friendly<P> struct pick<ZMod<P>> {
 using type= NttAvx2;
};
}  // namespace avx2_conv_internal

struct Avx2Conv {
 template <class K> static void convolve_range(const K* a, int n, const K* b, int m, int lo, int hi, K* out) {
  using C= typename avx2_conv_internal::pick<K>::type;
  C::template convolve_range<K>(a, n, b, m, lo, hi, out);
 }
};
