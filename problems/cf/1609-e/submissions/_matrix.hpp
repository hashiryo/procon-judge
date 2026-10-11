#pragma once
// 行列を載せる型の試作。NeoLibrary に置く案 (algo-notes の notes/linear-algebra-problems.md) の形を比べる。
//   MinPlus<T>: 和を min、積を + にした半環の値の型。0 は INF、1 は 0。和は INF を超えないように丸める。
//   SquareMatrix<T, N>: 大きさを型で決める。std::array に持ち、要素の型の演算子だけで掛ける。
//   DynMatrix<T>: 大きさを実行時に決める。vector に持つので、積のたびに確保する。
// 行列の側は、要素の型の zero() と one() で 0 と単位行列を作る。
#include "../common.hpp"
template <class T> struct MinPlus {
 static constexpr T INF= T(1) << (sizeof(T) * 8 - 3);
 T x= INF;
 constexpr MinPlus()= default;
 constexpr explicit MinPlus(T x): x(x) {}
 static constexpr MinPlus zero() { return MinPlus(INF); }
 static constexpr MinPlus one() { return MinPlus(0); }
 constexpr MinPlus operator+(MinPlus r) const { return MinPlus(x < r.x ? x : r.x); }
 constexpr MinPlus& operator+=(MinPlus r) { return x= x < r.x ? x : r.x, *this; }
 constexpr MinPlus operator*(MinPlus r) const {
  const T s= x + r.x;
  return MinPlus(s < INF ? s : INF);
 }
};
template <class T, int N> struct SquareMatrix {
 array<T, N * N> a;
 SquareMatrix() { a.fill(T::zero()); }
 static SquareMatrix identity() {
  SquareMatrix r;
  for(int i= 0; i < N; ++i) r.a[i * N + i]= T::one();
  return r;
 }
 T& operator()(int i, int j) { return a[i * N + j]; }
 const T& operator()(int i, int j) const { return a[i * N + j]; }
 SquareMatrix operator*(const SquareMatrix& r) const {
  SquareMatrix c;
  for(int i= 0; i < N; ++i)
   for(int k= 0; k < N; ++k)
    for(int j= 0; j < N; ++j) c.a[i * N + j]+= a[i * N + k] * r.a[k * N + j];
  return c;
 }
};
template <class T> struct DynMatrix {
 int n= 0;
 vector<T> a;
 DynMatrix()= default;
 explicit DynMatrix(int n): n(n), a((size_t)n * n, T::zero()) {}
 static DynMatrix identity(int n) {
  DynMatrix r(n);
  for(int i= 0; i < n; ++i) r.a[(size_t)i * n + i]= T::one();
  return r;
 }
 T& operator()(int i, int j) { return a[(size_t)i * n + j]; }
 const T& operator()(int i, int j) const { return a[(size_t)i * n + j]; }
 DynMatrix operator*(const DynMatrix& r) const {
  DynMatrix c(n);
  for(int i= 0; i < n; ++i)
   for(int k= 0; k < n; ++k)
    for(int j= 0; j < n; ++j) c.a[(size_t)i * n + j]+= a[(size_t)i * n + k] * r.a[(size_t)k * n + j];
  return c;
 }
};
