// 今の Library の MaxFlow (Dinic) で解く。転送装置を容量 1 の辺にして流し、EdgePtr::flow() で流れた辺を集め、1 から
// たどって道を作る (流れの保存から、使っていない流れた辺をたどれば必ず n に着く)。
#include <cstdio>
#include <vector>
#include "mylib/optimization/MaxFlow.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 MaxFlow<Dinic<int>> g(n + 1);
 std::vector<MaxFlow<Dinic<int>>::EdgePtr> es;
 es.reserve(m);
 for(int i= 0; i < m; ++i) {
  int a, b;
  if(scanf("%d %d", &a, &b) != 2) return 1;
  es.push_back(g.add_edge(a, b, 1));
 }
 const int k= g.maxflow(1, n);
 std::vector<std::vector<int>> out(n + 1);
 for(auto& e: es)
  if(e.flow() > 0) out[e.src()].push_back(e.dst());
 printf("%d\n", k);
 for(int r= 0; r < k; ++r) {
  std::vector<int> route{1};
  for(int v= 1; v != n;) {
   const int w= out[v].back();
   out[v].pop_back(), route.push_back(w), v= w;
  }
  printf("%d\n", (int)route.size());
  for(size_t i= 0; i < route.size(); ++i) printf("%d%c", route[i], i + 1 == route.size() ? '\n' : ' ');
 }
}
