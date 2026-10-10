// https://judge.yosupo.jp/problem/bipartitematching
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int l, int r, const vector<array<int, 2>> &edges);
//     void run();                            // 最大マッチングを求める
//     vector<array<int, 2>> answer() const;  // マッチングの辺 (a, b)
//   };
//
// 左の頂点は [0, l)、右の頂点は [0, r) で番号を分けて渡す。答えは一意でないので
// 判定はチェッカに任せる。構築と run を測る。ライブラリの関数として呼ぶときは
// 隣接リストを組む時間も込みになるので、構築に寄せた実装が得をしないようにする。
// edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-bipartite-matching.hpp"
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

  auto ans = sol.answer();
  string out = to_string(ans.size());
  out += '\n';
  out.reserve(ans.size() * 14 + out.size());
  for (auto &e : ans) {
    out += to_string(e[0]);
    out += ' ';
    out += to_string(e[1]);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
