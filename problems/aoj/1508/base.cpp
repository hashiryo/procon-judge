// https://onlinejudge.u-aizu.ac.jp/challenges/sources/UOA/UAPC/1508
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void rotate(int l, int r);              // [l, r) を右へ 1 つ回す
//     i64 min_of(int l, int r);               // [l, r) の最小値
//     void set(int i, i64 x);                 // a[i] = x
//   };
//
// 入力の区間は閉じているが、提出には半開区間で渡す。判定サイトごとの癖は
// ハーネスで吸収する。構築はどの実装も配列から O(N) で組むので外に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-rbst.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 1) ++gets;
  }

  Solver s(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    int y = (int)e[1], z = (int)e[2];
    if (e[0] == 0) s.rotate(y, z + 1);
    else if (e[0] == 1) ans.push_back(s.min_of(y, z + 1));
    else s.set(y, e[2]);
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
