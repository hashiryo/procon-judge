// NeoLibrary の det (neo/linalg/det.hpp) で行列木定理。根 r から出る全域有向木の数は、入次数のラプラシアン (対角に入次数、
// u → v の辺で (u, v) に -1) から行 r と列 r を除いた行列式。
#include <cstdio>
#include "neo/linalg/det.hpp"
int main() {
 using Z= ZMod<998244353>;
 int N, M, r;
 if(scanf("%d %d %d", &N, &M, &r) != 3) return 1;
 auto id= [&](int x) { return x < r ? x : x - 1; };
 Matrix<Z> L(N - 1, N - 1);
 for(int i= 0; i < M; ++i) {
  int u, v;
  if(scanf("%d %d", &u, &v) != 2) return 1;
  if(u == v || v == r) continue;
  L[id(v)][id(v)]+= Z::raw(1);
  if(u != r) L[id(u)][id(v)]-= Z::raw(1);
 }
 printf("%u\n", det(std::move(L)).val());
}
