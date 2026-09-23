// https://yukicoder.me/problems/no/1502
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, i64 k, const vector<array<i64, 3>> &eqs);  // 構築。計測区間の外。
//                                                              // eqs は {x, y, z} で x, y は 0-indexed
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 個数 mod 1e9+7
//   };
//
// A_x + A_y = z を辿ると、各成分の値は代表の値 t を使って ±t + b と書ける。
// 符号付きの平行移動は非可換な群なので、それを重みに持つポテンシャル付き
// Union-Find で成分ごとに t の範囲を絞る。比べるのは経路圧縮あり / なしの
// Union-Find。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  i64 k;
  must_scan(scanf("%d %d %lld", &n, &m, &k), 3);
  vector<array<i64, 3>> eqs(m);
  for (auto &e : eqs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    --e[0], --e[1];
  }

  Solver sol(n, k, eqs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
