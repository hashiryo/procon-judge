// https://yukicoder.me/problems/no/1569
// 係数がすべて 1 の線形漸化式を Bostan-Mori で解き、多項式の積に GF2p64 の畳み込みを使う。足し算は xor で、
// 掛ける係数は 0 か 1 なので、A_i をそのまま GF2p64 の元とみて計算してよい
#include <iostream>
#include <vector>
#include "neo/fft/convolve_GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 u64 K;
 cin >> N >> K;
 vector<GF2p64> A(N), Q(N + 1, GF2p64(1));  // Q(x) = 1 + x + ... + x^N (標数 2 なので符号は無い)
 for(auto& a: A) {
  u64 x;
  cin >> x;
  a= GF2p64(x);
 }
 auto P= convolve(A, Q);
 P.resize(N);
 // 標数 2 では Q(-x) = Q(x) で、係数が 0 か 1 なので Q(x)^2 = Q(x^2) となり、Q は変わらない
 for(u64 n= K - 1; n; n>>= 1) {
  const auto U= convolve(P, Q);
  for(int i= 0; i < N; ++i) P[i]= U[2 * i + (n & 1)];
 }
 cout << u64(P[0]) << '\n';
 return 0;
}
