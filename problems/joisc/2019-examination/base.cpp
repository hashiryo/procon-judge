// https://www2.ioi-jp.org/camp/2019/2019-sp-tasks/day1/examination.pdf
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<array<int, 2>> &st,    // 生徒ごとの {S, T}
//            const vector<array<int, 3>> &qs);   // 質問ごとの {X, Y, Z}
//     void run();                                 // ここだけ測る。
//     const vector<i64> &answer() const;          // 質問ごとの人数
//   };
//
// S >= X かつ T >= Y かつ S + T >= Z の生徒を数える。3 次元の支配数え上げで、質問を
// 全部読んでから解く。Z の降順に生徒を足しながら 2 次元の点集合に問い合わせる
// (kd 木 / 2 次元セグメント木) か、3 次元の kd 木に静的に載せるかを比べる。
// テストデータは JOI 春合宿 2019 の配布データを手で取り込む (source = manual)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-2d.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<int, 2>> st(n);
  for (auto &e : st) must_scan(scanf("%d %d", &e[0], &e[1]), 2);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  Solver sol(st, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
