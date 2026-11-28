// https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。a は初期値、eu[i]-ev[i] は i 番目の辺、
//     // queries は Q 個のクエリ。
//     Solver(int n, const vector<i64> &a, const vector<int> &eu,
//            const vector<int> &ev, const vector<array<i64, 4>> &queries);
//     void run();                          // ここだけ測る。
//     const vector<i64> &answers() const;  // タイプ 1 の答えを聞かれた順に
//   };
//
// queries[i] は e[0] が種別で、続きは入力の順に詰める。使わない項は 0。
//   0 p x   -> e[1] = p, e[2] = x
//   1 p l r -> e[1] = p, e[2] = l, e[3] = r
// x は負にもなるので i64。p は N 未満、l, r は N 以下なので int に収まるが、
// 種別ごとに項の型を分けると読みにくいので、この 2 種も i64 で詰める。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  vector<int> eu(n - 1), ev(n - 1);
  for (int i = 0; i < n - 1; ++i) must_scan(scanf("%d %d", &eu[i], &ev[i]), 2);

  // 0 p x と 1 p l r で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 4>> qs(q);
  for (auto &e : qs) {
    e[1] = e[2] = e[3] = 0;
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
    else must_scan(scanf("%lld %lld %lld", &e[1], &e[2], &e[3]), 3);
  }

  Solver sol(n, a, eu, ev, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
