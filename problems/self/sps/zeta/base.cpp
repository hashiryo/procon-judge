// ゼータ変換・メビウス変換・アダマール変換 (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int t, int n, const vector<i64> &a);  // 構築。計測区間の外。
//     void run();                                  // 変換。ここだけ測る。
//     vector<i64> answer() const;                  // 2^n 個
//   };
//
// t は変換と値の型を決める。
//   0: 部分集合の向きの max のゼータ。値は int。
//   1: 上位集合の向きの min のゼータ。値は long long。
//   2: 部分集合の向きの和のゼータ。値は unsigned long long で、2^64 を法として足し、答えは i64 に読み替えて出す。
//   3: 部分集合の向きのゼータ。値は mod 998244353。
//   4: 上位集合の向きのメビウス変換。値は mod 998244353。
//   5: アダマール変換。値は mod 998244353。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t, n;
  must_scan(scanf("%d %d", &t, &n), 2);
  vector<i64> a = read_ints(1 << n);

  Solver sol(t, n, a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
