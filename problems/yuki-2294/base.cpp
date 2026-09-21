// https://yukicoder.me/problems/no/2294
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);          // 構築。計測区間の外。
//     void link(int x, int v, i64 w);  // x と v を重み w の辺で結ぶ。別の成分なのは保証されている
//     i64 dist(int u, int v);          // 繋がっていれば経路の重みの xor、いなければ -1
//     i64 pair_sum(int v);             // v の成分の全対の dist の和 mod 998244353
//   };
//
// 変数 X の更新は入出力の側の話なのでハーネスが持つ。タイプ 2 の答えで X が
// 動くので、クエリはオンラインになる。
//
// 経路の xor をポテンシャルに持つ Union-Find の、経路圧縮あり / なしを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  i64 x;
  must_scan(scanf("%d %lld %d", &n, &x, &q), 3);

  // 1 v w / 2 u v / 3 v / 4 value で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 3>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 1 || e[0] == 2) must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
    else must_scan(scanf("%lld", &e[1]), 1);
  }

  Solver s(n);
  vector<i64> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 1) {
      s.link((int)x, (int)e[1], e[2]);
    } else if (e[0] == 2) {
      i64 d = s.dist((int)e[1], (int)e[2]);
      ans.push_back(d);
      if (d >= 0) x = (x + d) % n;
    } else if (e[0] == 3) {
      ans.push_back(s.pair_sum((int)e[1]));
    } else {
      x = (x + e[1]) % n;
    }
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
