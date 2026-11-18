// 今の Library には Gomory-Hu が無いので、今の Library の MaxFlow (Dinic) で Gusfield の方法を書く。組ごとに網を作り直し、
// 無向辺は add_edge の bidir で張る。最小カットは mincut(s)。処理済みの頂点も付け替え、t の親が s の側なら s を間に入れる
// (木の辺で切った集合が最小カットになる形)。順列は common.hpp で組む。
#include <vector>
#include "common.hpp"
#include "mylib/optimization/MaxFlow.hpp"
int main() {
 CF343E p;
 if(!p.read()) return 1;
 const int n= p.n;
 std::vector<int> par(n, 0);
 std::vector<long long> w(n, 0);
 par[0]= -1;
 for(int s= 1; s < n; ++s) {
  const int t= par[s];
  MaxFlow<Dinic<long long>> g(n);
  for(auto& [a, b, c]: p.e) g.add_edge(a, b, c, true);
  const long long f= g.maxflow(s, t);
  const std::vector<bool> X= g.mincut(s);
  w[s]= f;
  for(int i= 1; i < n; ++i)
   if(i != s && X[i] && par[i] == t) par[i]= s;
  if(t != 0 && X[par[t]]) par[s]= par[t], par[t]= s, w[s]= w[t], w[t]= f;
 }
 p.print(par, w);
}
