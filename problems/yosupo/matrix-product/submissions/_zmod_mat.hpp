#pragma once
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
// NeoLibrary の ZMod で行列の積を求める試作 (行列のヘッダの核)。A は n×m、B は m×p で、行優先の 1 本の配列で持つ。
// 設計の記録は algo-notes の notes/modular_arithmetic/modint_problems.md。

// ZMod の演算子だけで書く。今の Library の Matrix と同じ i-k-j の順で、積を 1 つずつ還元する。
template <class Z> vector<Z> mat_mul_naive(int n, int m, int p, const vector<Z>& A, const vector<Z>& B) {
 vector<Z> C((size_t)n * p);
 for(int i= 0; i < n; ++i) {
  Z* c= &C[(size_t)i * p];
  for(int k= 0; k < m; ++k) {
   const Z a= A[(size_t)i * m + k];
   const Z* b= &B[(size_t)k * p];
   for(int j= 0; j < p; ++j) c[j]+= a * b[j];
  }
 }
 return C;
}

// B を転置し、値を M 未満に直してから、内積の積を 16 個ずつ u64 にためて還元する (modulo-dot の barrett_lazy16)。
// M 未満どうしの積は M^2 < 2^60 なので、16 個までなら u64 に入る。
template <class Z> vector<Z> mat_mul_dot16(int n, int m, int p, const vector<Z>& A, const vector<Z>& B) {
 vector<u32> a((size_t)n * m), bt((size_t)p * m);
 for(size_t i= 0; i < a.size(); ++i) a[i]= A[i].val();
 for(int k= 0; k < m; ++k)
  for(int j= 0; j < p; ++j) bt[(size_t)j * m + k]= B[(size_t)k * p + j].val();
 vector<Z> C((size_t)n * p);
 for(int i= 0; i < n; ++i) {
  const u32* x= &a[(size_t)i * m];
  for(int j= 0; j < p; ++j) {
   const u32* y= &bt[(size_t)j * m];
   Z s;
   int k= 0;
   for(; k + 16 <= m; k+= 16) {
    u64 t= 0;
    for(int l= 0; l < 16; ++l) t+= u64(x[k + l]) * y[k + l];
    s+= Z(t);
   }
   u64 t= 0;
   for(; k < m; ++k) t+= u64(x[k]) * y[k];
   C[(size_t)i * p + j]= s + Z(t);
  }
 }
 return C;
}

// i-k-j の順で、C の行を u64 の和で持つ。値を M 未満に直してから足し、8 段ごとに和の 2^32 の位を R = 2^32 mod M に掛けて
// 畳む (t を t / 2^32 * R + t mod 2^32 に置き換える)。畳んだ値は 2^32 M + 2^32 < 2^62 + 2^32 で、積は M^2 < 2^60 なので、
// 8 個足しても u64 に入る。畳むのも 32 bit どうしの積なので、還元を待たずに列の向きに並べて計算できる。最後に 1 回だけ還元する。
// 列は L 個ずつの塊で回す。GCC の -O2 は、回数が決まっていて別名の検査が要らないループだけをベクトル化するので、
// 列を L の倍数まで 0 で埋め、__restrict を付ける。R 行ずつまとめて回すと、B の行を読む回数が R 分の 1 になる。
template <int R, class Z> vector<Z> mat_mul_acc(int n, int m, int p, const vector<Z>& A, const vector<Z>& B) {
 constexpr int L= 16;
 const int q= (p + L - 1) / L * L;
 vector<u32> b((size_t)m * q);
 for(int k= 0; k < m; ++k)
  for(int j= 0; j < p; ++j) b[(size_t)k * q + j]= B[(size_t)k * p + j].val();
 const u32 r32= u32((u64(1) << 32) % Z::mod());
 vector<u64> acc((size_t)R * q);
 vector<Z> C((size_t)n * p);
 for(int i0= 0; i0 < n; i0+= R) {
  const int rs= min(R, n - i0);
  fill(acc.begin(), acc.end(), 0);
  for(int k= 0; k < m; ++k) {
   u32 x[R];
#pragma GCC unroll 8
   for(int r= 0; r < R; ++r) x[r]= r < rs ? A[(size_t)(i0 + r) * m + k].val() : 0;
   const u32* __restrict y= &b[(size_t)k * q];
   u64* __restrict c= acc.data();
   for(int j= 0; j < q; j+= L) {
#pragma GCC unroll 8
    for(int r= 0; r < R; ++r)
     for(int l= 0; l < L; ++l) c[r * q + j + l]+= u64(x[r]) * y[j + l];
   }
   if((k & 7) == 7 && k + 1 < m) {
    for(int j= 0; j < R * q; j+= L)
     for(int l= 0; l < L; ++l) c[j + l]= u64(u32(c[j + l] >> 32)) * r32 + u32(c[j + l]);
   }
  }
  for(int r= 0; r < rs; ++r)
   for(int j= 0; j < p; ++j) C[(size_t)(i0 + r) * p + j]= Z(acc[(size_t)r * q + j]);
 }
 return C;
}

// ハーネスの形 (u32 の行優先の配列) に合わせる。
template <class F> inline vector<u32> run_zmod(int N, int M, int P, const vector<u32>& a, const vector<u32>& b, F f) {
 using Z= ZMod<998244353>;
 vector<Z> A(a.size()), B(b.size());
 for(size_t i= 0; i < a.size(); ++i) A[i]= Z(a[i]);
 for(size_t i= 0; i < b.size(); ++i) B[i]= Z(b[i]);
 const vector<Z> C= f(N, M, P, A, B);
 vector<u32> ret(C.size());
 for(size_t i= 0; i < C.size(); ++i) ret[i]= C[i].val();
 return ret;
}
