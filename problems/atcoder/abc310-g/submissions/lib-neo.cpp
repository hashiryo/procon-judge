// lib.cpp を NeoLibrary の FunctionalGraph と ZMod で書いたもの。頂点ごとに道の区間へ回数を掛けた値を差分の列で足し、位置の順の
// 累積和にする。
#include <iostream>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/graph/FunctionalGraph.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<998244353>;
 int N;
 long long K;
 cin >> N >> K;
 vector<int> A(N), B(N);
 for(int i= 0; i < N; ++i) cin >> A[i], --A[i];
 for(int i= 0; i < N; ++i) cin >> B[i];
 FunctionalGraph g(A);
 vector<Mint> sum(N + 1);
 for(int i= N; i--;) {
  g.path(i, K, [&](int l, int r, long long c) {
   const Mint x= Mint(c) * Mint(B[i]);
   sum[l]+= x, sum[r]-= x;
  });
 }
 for(int i= 0; i < N; ++i) sum[i + 1]+= sum[i];
 for(int i= N; i--;) sum[g.to_seq(i)]-= Mint(B[i]);
 Mint iv= Mint(1) / Mint(K);
 for(int i= 0; i < N; ++i) cout << sum[g.to_seq(i)] * iv << " \n"[i + 1 == N];
 return 0;
}
