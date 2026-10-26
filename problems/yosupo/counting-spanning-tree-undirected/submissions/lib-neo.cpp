// NeoLibrary の det (neo/linalg/det.hpp) で行列木定理。ラプラシアンから行 0 と列 0 を除いた行列式。
#include <cstdio>
#include "neo/linalg/det.hpp"
int main() {
 using Z= ZMod<998244353>;
 int N, M;
 if(scanf("%d %d", &N, &M) != 2) return 1;
 Matrix<Z> L(N - 1, N - 1);
 for(int i= 0; i < M; ++i) {
  int u, v;
  if(scanf("%d %d", &u, &v) != 2) return 1;
  if(u == v) continue;
  if(u) L[u - 1][u - 1]+= Z::raw(1);
  if(v) L[v - 1][v - 1]+= Z::raw(1);
  if(u && v) L[u - 1][v - 1]-= Z::raw(1), L[v - 1][u - 1]-= Z::raw(1);
 }
 printf("%u\n", det(std::move(L)).val());
}
