// https://loj.ac/p/2419 (原題は USACO 2016 US Open Platinum の Landscaping)
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int x, int y, int z, const vector<array<int, 2>> &ab);  // 費用と {A_i, B_i}。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;
//   };
//
// 花壇ごとの土の量を A から B に直す最小費用。買う (X)、捨てる (Y)、隣へ運ぶ
// (Z x 距離) を、左から右へ持ち越す量を変数にした凸関数で積む。区分線形凸関数
// (slope trick) を主形式で持つか共役形式で持つかを比べる。テストデータは LOJ から
// 手で取り込む (source = manual)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, x, y, z;
  must_scan(scanf("%d %d %d %d", &n, &x, &y, &z), 4);
  vector<array<int, 2>> ab(n);
  for (auto &e : ab) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver sol(x, y, z, ab);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
