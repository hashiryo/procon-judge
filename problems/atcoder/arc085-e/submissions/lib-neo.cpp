// NeoLibrary の BinaryOptimization (neo/combinatorial_optimization/BinaryOptimization.hpp) で解く。宝石 i を割るなら x_i = 1 と
// し、残すと a_i 円もらえることを、x_i = 0 の費用 -a_i にする。割る宝石の集合は倍数について閉じているので、i の倍数 j に
// ついて、x_i = 1 かつ x_j = 0 の組に大きな費用を置いて禁じる。
#include <iostream>
#include <vector>
#include "neo/combinatorial_optimization/BinaryOptimization.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n;
 cin >> n;
 const long long INF= 1e18;
 BinaryOptimization<long long> bo(n);
 for(int i= 0; i < n; ++i) {
  long long a;
  cin >> a;
  bo.add(i, -a, 0);
 }
 for(int i= 1; i <= n; ++i)
  for(int j= 2 * i; j <= n; j+= i) bo.add(i - 1, j - 1, 0, 0, INF, 0);
 cout << -*bo.solve() << '\n';
}
