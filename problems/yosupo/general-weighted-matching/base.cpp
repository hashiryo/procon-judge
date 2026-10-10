// https://judge.yosupo.jp/problem/general_weighted_matching
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 3>> &edges);  // 無向辺 {u, v, w}。単純グラフで、1 ≤ w ≤ 10^6
//     void run();                  // 重みの和が最大のマッチングを求める
//     vector<int> answer() const;  // マッチングに使う辺の番号
//   };
//
// N ≤ 500。答えは一意でないので判定はチェッカに任せる。構築と run を測る。edges は計測区間のあとまで生きているので、
// 提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 3>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> ans = sol.answer();
  long long sum = 0;
  for (int i : ans) sum += edges[i][2];
  string out = to_string(ans.size()) + ' ' + to_string(sum) + '\n';
  for (int i : ans) {
    out += to_string(edges[i][0]);
    out += ' ';
    out += to_string(edges[i][1]);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
