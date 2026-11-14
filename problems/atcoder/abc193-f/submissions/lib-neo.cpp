// NeoLibrary の BinaryOptimization (neo/combinatorial_optimization/BinaryOptimization.hpp) で解く。マスごとに x = 0 (黒) か
// 1 (白) を選び、隣り合う 2 マスの色が違えば -1 の費用にする。この 2 変数の項は劣モジュラでないが、BinaryOptimization が
// 市松模様の反転を見つけて直すので、問題文のまま書ける。色の決まったマスは、逆の色に大きな費用を置いて強制する。
#include <iostream>
#include <string>
#include <vector>
#include "neo/combinatorial_optimization/BinaryOptimization.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector<string> c(n);
 for(auto& row: c) cin >> row;
 const long long INF= 1e18;
 BinaryOptimization<long long> bo(n * n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) {
   const int v= i * n + j;
   if(c[i][j] == 'B') bo.add(v, 0, INF);
   if(c[i][j] == 'W') bo.add(v, INF, 0);
   if(i + 1 < n) bo.add(v, v + n, 0, -1, -1, 0);
   if(j + 1 < n) bo.add(v, v + 1, 0, -1, -1, 0);
  }
 cout << -*bo.solve() << '\n';
}
