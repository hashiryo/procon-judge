// https://judge.yosupo.jp/problem/point_set_range_sort_range_composite
//
// 提出は次を実装する。
//   struct Solver {
//     // init[i] = {p_i, a_i, b_i}。queries は下に書く形の列で、run() のあいだ生きている。
//     Solver(int n, const vector<array<i64, 3>> &init, const vector<array<i64, 5>> &queries);
//     void run();                          // ここだけ測る。queries を順に処理する。
//     const vector<i64> &answer() const;   // タイプ 1 の答えを順に
//   };
//
// queries[j] は e[0] が種別で、続きは入力の順に詰める。使わない項は 0。
//   0 i p a b  -> e[1] = i, e[2] = p, e[3] = a, e[4] = b
//   1 l r x    -> e[1] = l, e[2] = r, e[3] = x  (f_{r-1}(...f_l(x)) mod 998244353 を出す)
//   2 l r      -> e[1] = l, e[2] = r           ([l, r) を p 昇順に並べ替える)
//   3 l r      -> e[1] = l, e[2] = r           ([l, r) を p 降順に並べ替える)
//
// 値は 998244353 未満の整数でやりとりする。剰余の型は提出が選ぶ。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 3>> init(n);
  for (auto &e : init) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  // 種別ごとに項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> queries(q);
  int gets = 0;
  for (auto &e : queries) {
    e.fill(0);
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4]), 4);
    else if (e[0] == 1) must_scan(scanf("%lld %lld %lld", &e[1], &e[2], &e[3]), 3), ++gets;
    else must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
  }

  Solver sol(n, init, queries);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
