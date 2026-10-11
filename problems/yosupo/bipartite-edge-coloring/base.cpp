// https://judge.yosupo.jp/problem/bipartite_edge_coloring
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int l, int r, const vector<array<int, 2>> &edges);  // 辺 {a, b} (左 a、右 b)
//     void run();                          // 色の数が最大次数の辺彩色を求める
//     const vector<int> &colors() const;   // 辺ごとの色 (0 以上)
//   };
//
// L, R, M ≤ 10^5。出力する色の数 K は colors の最大 + 1 で、チェッカが K が最大次数に等しいことと、隣り合う辺の色が違う
// ことを確かめる。構築と run を測る。edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int l, r, m;
  must_scan(scanf("%d %d %d", &l, &r, &m), 3);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  auto t0 = chrono::steady_clock::now();
  Solver sol(l, r, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> &c = sol.colors();
  int k = 0;
  for (int x : c) k = max(k, x + 1);
  string out = to_string(k);
  out += '\n';
  out.reserve(out.size() + c.size() * 7);
  for (int x : c) {
    out += to_string(x);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
