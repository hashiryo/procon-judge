// https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &a, const vector<i64> &c, u64 k);  // 構築。計測区間の外。
//     void run();                                                // a_k を求める。ここだけ測る。
//     i64 value() const;
//   };
//
// a は初項 (長さ d)、c は漸化式 a_i = Σ_{j<d} c_j a_{i-1-j} の係数 (長さ d)。
// 剰余は 998244353 で、入出力は素の整数で渡す。内部表現を提出が選べるようにする
// ためで、理由はハーネスの節に書いてある。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-bostan-mori.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int d;
  u64 k;
  must_scan(scanf("%d %llu", &d, &k), 2);
  vector<i64> a = read_ints(d), c = read_ints(d);

  Solver sol(a, c, k);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.value());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
