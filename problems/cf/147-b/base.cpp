// https://codeforces.com/contest/147/problem/B
//
// 提出は次を実装する。
//   int solve(int n, const vector<array<int, 4>> &doors);  // doors は (i, j, c_ij, c_ji) で、部屋の番号は 0 始まり。計測区間の中
//
// 解き方の型は、扉の重みの max-plus の n × n の行列の対角に 0 を足したもの (k 乗の (i, j) 成分が、i から j への k 歩以下の
// 歩みの重みの最大になる) の 2 の冪を作り、上の bit から掛けて、対角に正の値が出ない最大の歩数を決めるもの (O(n^3 log n))。
// 重みは負にもなるので、MaxPlus の両側の範囲 (-INF か、絶対値が小さい値) の積を比べる。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 4>> doors(m);
  for (auto &[i, j, c, d] : doors) {
    must_scan(scanf("%d %d %d %d", &i, &j, &c, &d), 4);
    --i, --j;
  }

  auto t0 = chrono::steady_clock::now();
  const int ans = solve(n, doors);
  auto t1 = chrono::steady_clock::now();

  printf("%d\n", ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
