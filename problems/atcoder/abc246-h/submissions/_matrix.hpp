#pragma once
// 行列を載せる型の試作。NeoLibrary に置く案 (algo-notes の notes/linear-algebra-problems.md) の形を比べる。
//   SquareMatrix<T, N>: 大きさを型で決める。std::array に持ち、要素の型の演算子だけで掛ける。
//   SquareMatrixAcc<N>: SquareMatrix<ZMod, N> の積を、N 個の積を u64 にためてから 1 回で還元する形にしたもの。
//   DynMatrix<T>: 大きさを実行時に決める。vector に持つので、積のたびに確保する。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
template <class T, int N> struct SquareMatrix {
 array<T, N * N> a{};
 static SquareMatrix identity() {
  SquareMatrix r;
  for(int i= 0; i < N; ++i) r.a[i * N + i]= T(1);
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
template <int N> struct SquareMatrixAcc {
 using Z= ZMod<MOD>;
 array<u32, N * N> a{};  // M 未満
 static SquareMatrixAcc identity() {
  SquareMatrixAcc r;
  for(int i= 0; i < N; ++i) r.a[i * N + i]= 1;
  return r;
 }
 u32& operator()(int i, int j) { return a[i * N + j]; }
 u32 operator()(int i, int j) const { return a[i * N + j]; }
 SquareMatrixAcc operator*(const SquareMatrixAcc& r) const {
  static_assert(N <= 16);  // M 未満どうしの積 N 個の和が u64 に入る
  SquareMatrixAcc c;
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < N; ++j) {
    u64 s= 0;
    for(int k= 0; k < N; ++k) s+= u64(a[i * N + k]) * r.a[k * N + j];
    c.a[i * N + j]= Z(s).val();
   }
  return c;
 }
};
template <class T> struct DynMatrix {
 int n= 0;
 vector<T> a;
 DynMatrix()= default;
 explicit DynMatrix(int n): n(n), a((size_t)n * n) {}
 static DynMatrix identity(int n) {
  DynMatrix r(n);
  for(int i= 0; i < n; ++i) r.a[(size_t)i * n + i]= T(1);
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
