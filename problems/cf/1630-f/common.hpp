#pragma once
// cf-1630-f の提出で共有する、2 つに分けた約数の順序の辺の作り方。値 a_u を持つ頂点 u を (u, 0) = u と (u, 1) = n + u に
// 分け、a_u が a_v を割る (u != v) なら (u, 0) から (v, 0)、(u, 1) から (v, 1)、(u, 0) から (v, 1) へ、どの頂点でも (u, 0) から
// (u, 1) へ辺を張る。この関係は推移的で、最大反鎖の大きさが、3 つの値が割り切れる鎖にならない部分集合の最大の大きさになる
// (Greene-Kleitman の定理)。割り切れる組は、組の総当たり (n^2 / 2) と倍数の数え上げ (最大値 / a_u の和) の安い方で数える。
// pos は値から頂点の番号を引く表で、呼ぶ前は全部 -1。呼んだあとも -1 に戻す。
#include <algorithm>
#include <array>
#include <vector>
template <class F> void cf1630f_edges(const std::vector<int>& a, std::vector<int>& pos, F&& add) {
 const int n= a.size();
 const int mx= *std::max_element(a.begin(), a.end());
 long long mult= 0;
 for(int x: a) mult+= mx / x;
 auto pair_edge= [&](int u, int v) { add(u, v), add(n + u, n + v), add(u, n + v); };
 for(int u= 0; u < n; ++u) add(u, n + u);
 if((long long)n * (n - 1) / 2 <= mult) {
  for(int u= 0; u < n; ++u)
   for(int v= 0; v < n; ++v)
    if(u != v && a[v] % a[u] == 0) pair_edge(u, v);
 } else {
  for(int u= 0; u < n; ++u) pos[a[u]]= u;
  for(int u= 0; u < n; ++u)
   for(int x= 2 * a[u]; x <= mx; x+= a[u])
    if(pos[x] >= 0) pair_edge(u, pos[x]);
  for(int u= 0; u < n; ++u) pos[a[u]]= -1;
 }
}
