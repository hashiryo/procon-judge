// https://judge.yosupo.jp/problem/point_set_range_frequency
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。a は初期の列、queries は Q 個のクエリ。
//     Solver(const vector<int> &a, const vector<array<int, 4>> &queries);
//     void run();                          // ここだけ測る。
//     const vector<int> &answers() const;  // タイプ 1 の答えを聞かれた順に
//   };
//
// queries[i] は e[0] が種別で、続きは入力の順に詰める。使わない項は 0。
//   0 k v   -> e[1] = k, e[2] = v (a_k を v にする)
//   1 l r x -> e[1] = l, e[2] = r, e[3] = x (a_l ... a_{r-1} に x が現れる回数)
// N, Q は 2 * 10^5 以下 (0 もありうる)。a_i, v, x は 0 以上 10^9 以下なので int で渡す。
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
  // 種別ごとに項目数が違うので、種別を読んでから残りを読む。
  vector<array<int, 4>> qs(q);
  for (auto &e : qs) {
    e = {0, 0, 0, 0};
    must_scan(scanf("%d", &e[0]), 1);
    if (e[0] == 0) {
      must_scan(scanf("%d %d", &e[1], &e[2]), 2);
    } else {
      must_scan(scanf("%d %d %d", &e[1], &e[2], &e[3]), 3);
    }
  }

  Solver sol(a, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
