// 最小費用流を cost scaling (procon-judge の _shared/flow/mcf_cs3.hpp。partial augment、Dial の price update、早い打ち切り)
// で解く。頂点 0 から V - 1 へ F 流す b-flow にする。
#include <iostream>
#include "_shared/flow/mcf_cs3.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int V, E, F;
 cin >> V >> E >> F;
 vector<array<long long, 5>> es(E);
 for(auto& x: es) {
  int u, v, c, d;
  cin >> u >> v >> c >> d;
  x= {u, v, 0, c, d};
 }
 vector<long long> b(V);
 b[0]+= F, b[V - 1]-= F;
 const auto r= mcf_cs3::b_flow<16, 4, 2, 1, 1, 100>(V, b, es);
 if(r.ok) cout << (long long)r.cost << '\n';
 else cout << -1 << '\n';
}
