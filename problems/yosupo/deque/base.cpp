// https://judge.yosupo.jp/problem/deque
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。queries は Q 個のクエリ。
//     explicit Solver(const vector<array<int, 2>> &queries);
//     void run();                          // ここだけ測る。
//     const vector<int> &answers() const;  // タイプ 4 の答えを聞かれた順に
//   };
//
// queries[i] は e[0] が種別で、続きは入力の順に詰める。使わない項は 0。
//   0 x -> e[1] = x (先頭に足す)
//   1 x -> e[1] = x (末尾に足す)
//   2   -> 先頭を消す
//   3   -> 末尾を消す
//   4 i -> e[1] = i (a_i を出す)
// 値 x は 10^9 以下、添字 i は Q 未満なので、どちらも int で渡す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  // 種別ごとに項目数が違うので、種別を読んでから残りを読む。
  vector<array<int, 2>> qs(q);
  for (auto &e : qs) {
    e[0] = e[1] = 0;
    must_scan(scanf("%d", &e[0]), 1);
    if (e[0] <= 1 || e[0] == 4) must_scan(scanf("%d", &e[1]), 1);
  }

  Solver sol(qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
