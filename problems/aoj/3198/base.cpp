// https://onlinejudge.u-aizu.ac.jp/problems/3198
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 構築。計測区間の外。
//     bool toggle(int x, int y);  // 辺 (x, y) を足すか抜くかして、完全マッチングが
//                                 // あるかを返す
//   };
//
// 左右とも頂点数 n で、どちらも 0-indexed で渡す。構築は辺を並べるだけなので
// 計測区間の外に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-bipartite-matching.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    --e[0], --e[1];
  }
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 2>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    --e[0], --e[1];
  }

  Solver s(n, edges);
  vector<char> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) ans.push_back(s.toggle(e[0], e[1]));
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(ans.size() * 4);
  for (char c : ans) out += c ? "Yes\n" : "No\n";
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
