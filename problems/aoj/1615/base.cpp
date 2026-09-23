// https://onlinejudge.u-aizu.ac.jp/challenges/sources/ICPC/Prelim/1615
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 頂点は 0-indexed
//     void run();                     // ここだけ測る。
//     array<i64, 2> answer() const;   // {下限, 上限}
//   };
//
// 各辺をどちらかの端点に割り当てたときの、頂点あたりの本数の幅を最小にする。
// 幅を小さい方から試して、下限つき最大流が実行可能かを見る。
//
// 1 つの入力に複数のデータセットが入っているので、ハーネスが全部読んでから
// データセットごとに Solver を作る。計測区間は全部の合計。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dinic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  vector<int> ns;
  vector<vector<array<int, 2>>> datasets;
  for (;;) {
    int n, m;
    must_scan(scanf("%d %d", &n, &m), 2);
    if (n == 0) break;
    vector<array<int, 2>> edges(m);
    for (auto &e : edges) {
      must_scan(scanf("%d %d", &e[0], &e[1]), 2);
      --e[0], --e[1];
    }
    ns.push_back(n);
    datasets.push_back(std::move(edges));
  }

  vector<array<i64, 2>> ans;
  ans.reserve(ns.size());

  auto t0 = chrono::steady_clock::now();
  for (size_t i = 0; i < ns.size(); ++i) {
    Solver s(ns[i], datasets[i]);
    s.run();
    ans.push_back(s.answer());
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  for (auto &a : ans) {
    out += to_string(a[0]);
    out += ' ';
    out += to_string(a[1]);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
