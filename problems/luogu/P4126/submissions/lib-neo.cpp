// NeoLibrary の MaxFlow (neo/combinatorial_optimization/MaxFlow.hpp) で s から t へ流し、cut_kinds(s, t) で辺ごとに、どれかの
// 最小カットに入るか (1 以上) と、すべての最小カットに入るか (2) を答える。
#include <cstdio>
#include <string>
#include "neo/combinatorial_optimization/MaxFlow.hpp"
int main() {
 int n, m, s, t;
 if(scanf("%d %d %d %d", &n, &m, &s, &t) != 4) return 1;
 MaxFlow<long long> g(n);
 for(int i= 0; i < m; ++i) {
  int u, v, c;
  if(scanf("%d %d %d", &u, &v, &c) != 3) return 1;
  g.add_edge(u - 1, v - 1, c);
 }
 g.flow(s - 1, t - 1);
 const std::vector<int> k= g.cut_kinds(s - 1, t - 1);
 std::string out;
 for(int i= 0; i < m; ++i) out+= k[i] >= 1 ? '1' : '0', out+= ' ', out+= k[i] == 2 ? '1' : '0', out+= '\n';
 fputs(out.c_str(), stdout);
}
