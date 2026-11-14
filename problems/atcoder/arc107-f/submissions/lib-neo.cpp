// NeoLibrary の KaryOptimization (neo/combinatorial_optimization/KaryOptimization.hpp) で解く。頂点ごとに x = 0 (B を引く)、
// 1 (A を払って消す)、2 (B を足す) の 3 値を選び、得点の符号を反転した費用を最小にする。残した頂点どうしを結ぶ辺の両端で
// 0 と 2 に分かれる組は禁じる。禁止は辺ごとの 2 変数の項として足すので、今の Library の monge_mincut のように全組を
// 呼ばない。
#include <iostream>
#include <vector>
#include "neo/combinatorial_optimization/KaryOptimization.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int n, m;
 cin >> n >> m;
 vector<long long> a(n), b(n);
 for(auto& x: a) cin >> x;
 for(auto& x: b) cin >> x;
 KaryOptimization<long long> ko(n, 3);
 for(int i= 0; i < n; ++i) ko.add(i, vector<long long>{b[i], a[i], -b[i]});
 const long long INF= 1e18;
 for(int k= 0; k < m; ++k) {
  int u, v;
  cin >> u >> v, --u, --v;
  ko.add(u, v, [&](int x, int y) { return x + y == 2 && x != y ? INF : 0LL; });
 }
 cout << -*ko.solve() << '\n';
}
