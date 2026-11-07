#pragma once
// lib-neo の、積ごとの範囲の判定がどれだけ効くかを測るための MinPlus<int> の写し。NeoLibrary の MinPlus<int> (neo/algebra/semiring.hpp)
// と同じ値の表し方 (INF は int の最大の半分) で、2 つの配列を受ける fast_one_sided だけを変える。測り終えたら消す。
//   MODE 0 (lib-neo-nocheck): 調べずに true を返す。この問題の重みは負にならないので答えは同じ。判定を省いたときの速さを見るためだけの形
//   MODE 1 (lib-neo-vptest): 8 個ずつ読んで or し、INF の外の bit を vptest 1 回で調べる (8 で割り切れない分は、最後の 8 個を
//   重ねて読む。n < 8 はスカラー)
#include "../common.hpp"
#include "neo/algebra/semiring.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
template <int MODE> struct MinPlusAB {
 using T= int;
 using U= unsigned;
 static constexpr T INF= MinPlus<int>::INF;
 T x;
 constexpr MinPlusAB(): x(INF) {}
 constexpr MinPlusAB(T x): x(x) {}
 static constexpr MinPlusAB zero() { return MinPlusAB(INF); }
 static constexpr MinPlusAB one() { return MinPlusAB(0); }
 static bool fast_one_sided(const MinPlusAB* p, std::size_t n) { return fast_one_sided(p, p, n); }
 static bool fast_one_sided(const MinPlusAB* p, const MinPlusAB* q, std::size_t n) {
  if constexpr(MODE == 0) return true;
  else {
   if(n < 8) {
    U s= 0;
    for(std::size_t i= 0; i < n; ++i) s|= U(p[i].x) | U(q[i].x);
    return !(s & ~U(INF));
   }
   const __m256i mask= _mm256_set1_epi32(int(~U(INF)));
   __m256i o= _mm256_or_si256(_mm256_loadu_si256((const __m256i*)(p + n - 8)), _mm256_loadu_si256((const __m256i*)(q + n - 8)));
   for(std::size_t i= 0; i + 8 <= n; i+= 8) o= _mm256_or_si256(o, _mm256_or_si256(_mm256_loadu_si256((const __m256i*)(p + i)), _mm256_loadu_si256((const __m256i*)(q + i))));
   return _mm256_testz_si256(o, mask);
  }
 }
 static constexpr bool fast_bounded(const MinPlusAB*, std::size_t) { return false; }
 static constexpr MinPlusAB fast_mul(MinPlusAB l, MinPlusAB r) { return MinPlusAB(T(l.x + r.x)); }
 static constexpr MinPlusAB fast_fix(MinPlusAB v, bool bounded) {
  const T t= bounded ? T(INF >> 1) : INF;
  return MinPlusAB(v.x >= t ? INF : v.x);
 }
 constexpr MinPlusAB& operator+=(MinPlusAB r) { return x= r.x < x ? r.x : x, *this; }
 constexpr MinPlusAB& operator*=(MinPlusAB r) {
  const T s= T(U(x) + U(r.x)), t= x > r.x ? x : r.x;
  const U m= U(0) - U((t > s ? t : s) >= INF);
  x= T((U(s) & ~m) | (U(INF) & m));
  return *this;
 }
 friend constexpr MinPlusAB operator+(MinPlusAB l, MinPlusAB r) { return l+= r; }
 friend constexpr MinPlusAB operator*(MinPlusAB l, MinPlusAB r) { return l*= r; }
 friend constexpr bool operator==(MinPlusAB l, MinPlusAB r) { return l.x == r.x; }
 friend constexpr bool operator!=(MinPlusAB l, MinPlusAB r) { return l.x != r.x; }
};
