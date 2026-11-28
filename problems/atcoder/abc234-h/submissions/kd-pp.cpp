// 距離が K 以下の点の組を列挙する。1 点につき 1 ノードを置く KD 木
// (problems/_shared/kd-tree/kd_ball_2d.hpp) で、各点の円に入る点を集める。
// lib.cpp (今の Library の KDTree::enum_ball) と書き比べる。
#include <algorithm>
#include <iostream>
#include <vector>
#include "_shared/kd-tree/kd_ball_pp.hpp"
using namespace std;

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int N, K;
  cin >> N >> K;
  vector<int> x(N), y(N), id(N);
  for (int i = 0; i < N; ++i) cin >> x[i] >> y[i], id[i] = i;
  KdBallPP kd(x, y, id);
  vector<pair<int, int>> ans;
  const long long r2 = (long long)K * K;
  for (int i = N; i--;) {
    kd.search(x[i], y[i], r2, [&ans, i](int p) {
      if (i < p) ans.emplace_back(i + 1, p + 1);
    });
  }
  sort(ans.begin(), ans.end());
  cout << ans.size() << '\n';
  for (auto [a, b] : ans) cout << a << " " << b << '\n';
  return 0;
}
