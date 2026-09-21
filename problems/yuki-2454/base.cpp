// https://yukicoder.me/problems/no/2454
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<string> &cases);  // テストケースごとの文字列。計測区間の外。
//     void run();                                     // ここだけ測る。
//     const vector<i64> &answer() const;              // テストケースごとの個数
//   };
//
// 文字列を i 文字目で切った前半が後半より辞書順で真に小さい i の数。前半と後半の
// 最長共通接頭辞が分かれば次の 1 文字で決まるので、それをローリングハッシュの
// 二分探索で取るか Z アルゴリズムで取るかを比べる。テストケースは全部まとめて渡す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-z-algorithm.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<string> cases(t);
  for (auto &s : cases) {
    int n;
    must_scan(scanf("%d", &n), 1);
    s = read_token();
  }

  Solver sol(cases);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
