// NeoLibrary の PrimalDual (neo/combinatorial_optimization/PrimalDual.hpp) で、流しながら頂点を足す。料理人 j が後ろから k 番目に作る枠を頂点に
// し、料理 i からその枠へ容量 1、費用 k t_ij の辺を張る。料理人ごとにまだ使っていない枠を 1 つだけ置き、1 皿ずつ流して、
// 使った枠の次の枠 (k + 1 番目) を足す。k 番目の枠は k + 1 番目より安いので、使っていない枠を 1 つ置けば足りる。
#include <iostream>
#include <vector>
#include "neo/combinatorial_optimization/PrimalDual.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, m;
 cin >> n >> m;
 vector<int> p(n);
 int total= 0;
 for(int& x: p) cin >> x, total+= x;
 vector<vector<long long>> t(n, vector<long long>(m));
 for(auto& row: t)
  for(auto& x: row) cin >> x;
 const int S= 0, T= 1;
 PrimalDual<int, long long> g(2 + n);
 for(int i= 0; i < n; ++i) g.add_edge(S, 2 + i, p[i], 0);
 vector<int> front(m), kth(m, 1);
 auto add_slot= [&](int j) {
  const int v= g.add_vertex();
  for(int i= 0; i < n; ++i) g.add_edge(2 + i, v, 1, kth[j] * t[i][j]);
  front[j]= g.add_edge(v, T, 1, 0);
 };
 for(int j= 0; j < m; ++j) add_slot(j);
 long long ans= 0;
 for(int k= 0; k < total; ++k) {
  ans+= g.flow(S, T, 1).second;
  for(int j= 0; j < m; ++j)
   if(g.get_edge(front[j]).flow) ++kth[j], add_slot(j);
 }
 cout << ans << '\n';
}
