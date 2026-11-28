// 円形クエリ。1 点につき 1 ノードを置く KD 木 (problems/_shared/kd-tree/kd_ball_pp.hpp) で、
// 円に入る点を数える。lib.cpp (今の Library の KDTree::prod_ball) と書き比べる。
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "_shared/kd-tree/kd_ball_pp.hpp"
using namespace std;

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  for (int AN, BN, R; cin >> AN >> BN >> R && AN;) {
    vector<int> ax(AN), ay(AN), bx(BN), by(BN), bid(BN);
    for (int i = 0; i < AN; ++i) cin >> ax[i] >> ay[i];
    for (int i = 0; i < BN; ++i) cin >> bx[i] >> by[i], bid[i] = i;
    KdBallPP kd(bx, by, bid);
    long long ans = 0;
    const long long r2 = (long long)R * 4 * R * 4;
    for (int i = 0; i < AN; ++i) ans += kd.count(ax[i], ay[i], r2);
    cout << ans << '\n';
  }
  return 0;
}
