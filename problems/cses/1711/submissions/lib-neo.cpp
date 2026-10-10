// NeoLibrary の MaxFlow (neo/flow/MaxFlow.hpp) で解く。転送装置を容量 1 の辺にして flow(1, n) で流し、decompose(1, n) が
// 返す道 (辺の番号の列) をそのまま部屋の列に直して出す。容量が 1 なので、どの道の流量も 1 になる。
#include <array>
#include <cstdio>
#include <vector>
#include "neo/flow/MaxFlow.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 MaxFlow<int> g(n + 1);
 std::vector<std::array<int, 2>> es(m);
 for(auto& [a, b]: es) {
  if(scanf("%d %d", &a, &b) != 2) return 1;
  g.add_edge(a, b, 1);
 }
 const int k= g.flow(1, n);
 printf("%d\n", k);
 for(auto& [d, path]: g.decompose(1, n))
  for(int rep= 0; rep < d; ++rep) {
   printf("%d\n1", (int)path.size() + 1);
   for(int e: path) printf(" %d", es[e][1]);
   printf("\n");
  }
}
