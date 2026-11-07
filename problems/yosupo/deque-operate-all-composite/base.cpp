// https://judge.yosupo.jp/problem/deque_operate_all_composite
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int q);          // 構築。計測区間の外。q はクエリの数 (確保の目安)
//     void push_front(i64 a, i64 b);   // 列の先頭に x -> a x + b を足す
//     void push_back(i64 a, i64 b);    // 列の末尾に x -> a x + b を足す
//     void pop_front();                // 列の先頭を消す
//     void pop_back();                 // 列の末尾を消す
//     i64 query(i64 x);                // 列の合成を x に当てた値 mod 998244353。空なら x
//   };
//
// 値は 998244353 未満の整数でやりとりする。剰余の型は提出が選ぶ。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/swag.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  // 0 a b、1 a b、2、3、4 x で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] <= 1) must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
    else if (e[0] == 4) must_scan(scanf("%lld", &e[1]), 1), ++gets;
  }

  Solver s(q);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    switch (e[0]) {
      case 0: s.push_front(e[1], e[2]); break;
      case 1: s.push_back(e[1], e[2]); break;
      case 2: s.pop_front(); break;
      case 3: s.pop_back(); break;
      default: ans.push_back(s.query(e[1])); break;
    }
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
