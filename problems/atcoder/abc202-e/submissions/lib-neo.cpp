// lib.cpp を NeoLibrary の HeavyLightDecomposition (親の列から作る) と WaveletMatrix で書いたもの。部分木の位置の区間で、深さが
// D の頂点の個数を数える。
#include <iostream>
#include <vector>
#include "neo/base/ds/range/WaveletMatrix.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<int> par(N, -1);
 for(int i= 1; i < N; ++i) cin >> par[i], --par[i];
 HeavyLightDecomposition tree(par);
 vector<int> d(N);
 for(int v= N; v--;) d[tree.to_seq(v)]= tree.depth(v);
 WaveletMatrix<int> wm(d);
 int Q;
 cin >> Q;
 while(Q--) {
  int U, D;
  cin >> U >> D;
  auto [l, r]= tree.subtree(U - 1);
  cout << wm.count(l, r, D, D + 1) << '\n';
 }
 return 0;
}
