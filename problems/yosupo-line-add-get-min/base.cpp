// https://judge.yosupo.jp/problem/line_add_get_min
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<i64, 2>> &lines);  // {a, b} で y = ax + b
//     void insert(i64 a, i64 b);
//     i64 query(i64 p);
//   };
//
// 最初の N 本を入れるところも計測区間に入れてある。クエリの `0 a b` と同じ
// 操作なので、外に出すと追加の半分だけを測らないことになる。入力の解析だけを
// 外に出す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-convex-hull-trick.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 2>> lines(n);
  for (auto &e : lines) must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);

  // 0 a b と 1 p で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
    if (e[0] == 0) must_scan(scanf("%lld", &e[2]), 1);
    else ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(lines);
  for (auto &e : qs) {
    if (e[0] == 0) s.insert(e[1], e[2]);
    else ans.push_back(s.query(e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
