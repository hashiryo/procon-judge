// https://judge.yosupo.jp/problem/dominatortree
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int s, const vector<array<int, 2>> &edges);  // 有向辺 (a, b) の列。自己ループと多重辺を含みうる。edges を
//                                                                // 覚えるだけにする。edges は Solver より長く生きる
//     void run();                                                // ここだけ測る。
//     int parent(int v) const;                                   // 頂点 v の直接支配点。s なら s、s から届かなければ -1
//   };
//
// 隣接を組むところと作業領域の確保も run() の中に置く (yosupo-scc と同じ)。parent() は run() の結果から O(1) で返す。
// 設計は algo-notes の notes/graph-basics.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lt.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, s;
  must_scan(scanf("%d %d %d", &n, &m, &s), 3);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(n, s, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(n) * 8 + 16);
  for (int v = 0; v < n; ++v) {
    if (v) out += ' ';
    out += to_string(sol.parent(v));
  }
  out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
