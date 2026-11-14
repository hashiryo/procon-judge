// lib-neo と同じ網に、t から s へ下限 0、上限 H W の辺を足し、solve で流れがあるかを見る。max_flow は流れを見つけたあと
// s から t へ続けて流すので、その分がかからない。
#include <iostream>
#include <string>
#include <vector>
#include "neo/combinatorial_optimization/MaxFlowLowerBound.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int h, w;
 cin >> h >> w;
 vector<string> c(h);
 for(auto& row: c) cin >> row;
 const int s= h * w, t= s + 1;
 MaxFlowLowerBound<long long> g(h * w + 2);
 for(int i= 0; i < h; ++i)
  for(int j= 0; j < w; ++j) {
   if(c[i][j] == '1') continue;
   const int v= i * w + j, lo= c[i][j] == '2';
   if((i + j) % 2) {
    g.add_edge(v, t, lo, 1);
    continue;
   }
   g.add_edge(s, v, lo, 1);
   if(i > 0 && c[i - 1][j] != '1') g.add_edge(v, v - w, 0, 1);
   if(i + 1 < h && c[i + 1][j] != '1') g.add_edge(v, v + w, 0, 1);
   if(j > 0 && c[i][j - 1] != '1') g.add_edge(v, v - 1, 0, 1);
   if(j + 1 < w && c[i][j + 1] != '1') g.add_edge(v, v + 1, 0, 1);
  }
 g.add_edge(t, s, 0, h * w);
 cout << (g.solve() ? "Yes" : "No") << '\n';
}
