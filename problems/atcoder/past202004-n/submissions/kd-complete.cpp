// 正方形の加算と点の取得。点の集合 (クエリの点) に長方形の加算を積み、点の値を読む双対の KD 木
// (problems/_shared/kd-tree/kd_dual_2d.hpp) で解く。lib.cpp (今の Library の KDTree の双対) と書き比べる。
#include <array>
#include <iostream>
#include <vector>
#include "_shared/kd-tree/kd_dual_2d.hpp"
using namespace std;

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  int N, Q;
  cin >> N >> Q;
  vector<array<int, 4>> sq(N);
  for (auto &e : sq) cin >> e[0] >> e[1] >> e[2] >> e[3];
  vector<int> x(Q), y(Q);
  for (int i = 0; i < Q; ++i) cin >> x[i] >> y[i];
  vector<long long> init(Q, 0);
  KdDual2D<KdRingI64> kd(x, y, init);
  for (auto [a, b, D, C] : sq) kd.apply(a, b, a + D + 1, b + D + 1, 1, C);
  for (int i = 0; i < Q; ++i) cout << kd.get(i) << '\n';
  return 0;
}
