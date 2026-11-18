// 今の Library の MaxFlow (Dinic) で s から t へ流し、残余グラフ (残りの容量のある向きの弧) を Graph に組んで
// StronglyConnectedComponents で強連結成分に分ける。辺が飽和していて両端の成分が違えば、どれかの最小カットに入る。さらに
// 始点が s の成分、終点が t の成分にあれば、すべての最小カットに入る。
#include <cstdio>
#include <string>
#include <vector>
#include "mylib/graph/StronglyConnectedComponents.hpp"
#include "mylib/optimization/MaxFlow.hpp"
int main() {
 int n, m, s, t;
 if(scanf("%d %d %d %d", &n, &m, &s, &t) != 4) return 1;
 --s, --t;
 using MF= MaxFlow<Dinic<long long>>;
 MF g(n);
 std::vector<MF::EdgePtr> es;
 es.reserve(m);
 for(int i= 0; i < m; ++i) {
  int u, v, c;
  if(scanf("%d %d %d", &u, &v, &c) != 3) return 1;
  es.push_back(g.add_edge(u - 1, v - 1, c));
 }
 g.maxflow(s, t);
 Graph r(n);
 for(auto& e: es) {
  if(e.flow() < e.cap()) r.add_edge(e.src(), e.dst());
  if(e.flow() > 0) r.add_edge(e.dst(), e.src());
 }
 StronglyConnectedComponents scc(r);
 std::string out;
 for(auto& e: es) {
  const bool full= e.flow() == e.cap();
  const int a= scc(e.src()), b= scc(e.dst());
  out+= full && a != b ? '1' : '0', out+= ' ', out+= full && a == scc(s) && b == scc(t) ? '1' : '0', out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
