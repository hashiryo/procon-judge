// https://yukicoder.me/problems/no/2114
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(i64 k, const vector<i64> &b, const vector<i64> &r);  // 青と赤の頂点の値。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 結べなければ -1
//   };
//
// 値を K ずつ増やして、青と赤の頂点を min(N, M) 組結ぶ最小費用。K で割った余りが
// 同じ頂点どうししか結べないので、余りごとに商の数直線上で最小費用マッチングを
// 解いて足す。余りで分けて座標圧縮するところは common.hpp に置き、区分線形凸関数
// (slope trick) を主形式で持つか共役形式で持つかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-primal.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  i64 k;
  must_scan(scanf("%d %d %lld", &n, &m, &k), 3);
  vector<i64> b = read_ints(n);
  vector<i64> r = read_ints(m);

  Solver sol(k, b, r);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
