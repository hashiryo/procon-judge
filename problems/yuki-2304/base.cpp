// https://yukicoder.me/problems/no/2304
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // a は昇順に並んでいる。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 要素を 1 ずつ動かして全部を相異なる値にするときの、操作回数の最小値。昇順に
// 並べてから見ると「1 つ前より真に大きくする」制約になる。並べ替えはどの実装
// でも同じで比べたいところではないので、ハーネスで済ませて昇順の a を渡す。
// 区分線形凸関数 (slope trick) を主形式で持つか共役形式で持つかを比べる。
#include <algorithm>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(n);
  std::sort(a.begin(), a.end());

  Solver sol(a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
