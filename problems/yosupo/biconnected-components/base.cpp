// https://judge.yosupo.jp/problem/biconnected_components
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 無向辺の列。自己ループは無く、多重辺を含みうる。edges を覚えるだけにする。
//                                                          // edges は Solver より長く生きる
//     void run();                                          // ここだけ測る。
//     vector<vector<int>> answer() const;                  // 二重連結成分ごとの頂点の列。辺を持たない頂点は、それだけで 1 つの成分
//   };
//
// 隣接を組むところと作業領域の確保も run() の中に置く (yosupo-scc と同じ)。切断点は複数の成分に入るので、頂点ごとの番号でなく
// 成分ごとの頂点の列で返す。answer() は計測区間の外で呼ぶので、run() の内部の持ち方 (成分ごとに切り出した頂点の列、辺ごとの
// 成分の番号など) から列を作り直すのはここでしてよい。成分と頂点の並びは問わない。設計は algo-notes の notes/graph-basics.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver s(n, edges);

  auto t0 = chrono::steady_clock::now();
  s.run();
  auto t1 = chrono::steady_clock::now();

  const vector<vector<int>> blocks = s.answer();
  string out;
  out.reserve(size_t(n + m) * 8 + 16);
  out += to_string(blocks.size());
  out += '\n';
  for (auto &b : blocks) {
    out += to_string(b.size());
    for (int v : b) {
      out += ' ';
      out += to_string(v);
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
