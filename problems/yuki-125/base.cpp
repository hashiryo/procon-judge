// https://yukicoder.me/problems/no/125
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &c);  // 色ごとの花びらの枚数。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 花のパターン数 mod 1e9+7
//   };
//
// 回転で一致するものを同一視した円順列の数。枚数の最大公約数の約数 d ごとに周期 d
// の並べ方 (多項係数) を数えて、Euler の phi で重み付けするか、約数上の配列に
// 倍数メビウス変換をかけて周期がちょうど d のものに直すかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-totient.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int k;
  must_scan(scanf("%d", &k), 1);
  vector<i64> c = read_ints(k);

  Solver sol(c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
