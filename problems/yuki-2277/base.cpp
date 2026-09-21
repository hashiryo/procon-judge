// https://yukicoder.me/problems/no/2277
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &claims);  // 構築。計測区間の外。
//                                                          // claims は {a, b, c} で a, b は 0-indexed
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 矛盾しない組み合わせの個数 mod 998244353
//   };
//
// 嘘つきを 1 とすると、a が b を c だと言う証言は x_a xor x_b = c になる。全部の
// 証言に矛盾が無ければ、成分ごとに 2 通りなので 2^(成分数)。xor を重みに持つ
// ポテンシャル付き Union-Find の、経路圧縮あり / なしを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 3>> claims(q);
  for (auto &e : claims) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    --e[0], --e[1];
  }

  Solver sol(n, claims);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
