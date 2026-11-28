// 距離が K 以下の点の組を列挙する。neo の KdBall (neo/base/ds/kd/KdBall.hpp、1 点 1 ノード) で、
// 各点の円に入る点を集める。lib.cpp (今の Library の KDTree::enum_ball) と書き比べる。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/base/ds/kd/KdBall.hpp"
using namespace std;

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int N, K;
  cin >> N >> K;
  vector<array<int, 2>> pt(N);
  vector<int> id(N);
  for (int i = 0; i < N; ++i) cin >> pt[i][0] >> pt[i][1], id[i] = i;
  KdBall<int, 2> kd(pt, id);
  vector<pair<int, int>> ans;
  const long long r2 = (long long)K * K;
  for (int i = N; i--;) {
    kd.search(pt[i], r2, [&ans, i](int p) {
      if (i < p) ans.emplace_back(i + 1, p + 1);
    });
  }
  sort(ans.begin(), ans.end());
  cout << ans.size() << '\n';
  for (auto [a, b] : ans) cout << a << " " << b << '\n';
  return 0;
}
