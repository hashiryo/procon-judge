// https://judge.yosupo.jp/problem/majority_voting
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。a は初期の列、queries は Q 個のクエリ。
//     Solver(vector<int> a, const vector<array<int, 3>> &queries);
//     void run();                               // ここだけ測る。
//     const vector<int> &answers() const;       // タイプ 1 の答えを聞かれた順に
//   };
//
// queries[i] は e[0] が種別で、続きは入力の順に詰める。どちらの種別も 3 項目。
//   0 p x -> e[1] = p, e[2] = x (a_p を x にする)
//   1 l r -> e[1] = l, e[2] = r (a_l ... a_{r-1} の過半数)
// 値は 10^9 以下、添字は N 以下なので、どれも int で渡す。
//
// クエリ列を先に全部読んで Solver に渡すので、値の座標圧縮のような構築は
// 計測区間の外でできる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  Solver sol(a, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
