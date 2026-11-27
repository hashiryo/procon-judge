// https://judge.yosupo.jp/problem/division_of_polynomials
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &f, const vector<i64> &g);  // 構築。計測区間の外。
//     void run();                                         // 商と余りを作る。ここだけ測る。
//     vector<i64> quotient() const;                       // 商の係数 (次数 + 1 個。0 は空)
//     vector<i64> remainder() const;                      // 余りの係数 (同上)
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。内部表現を提出が選べるようにする
// ためで、理由はハーネスの節に書いてある。変換は O(N)、計算は O(N log N) なので、
// 一緒に測ると表現の選び方で乗り方が変わる。構築と取り出しを分けてあるのはそのため。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-polynomial.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<i64> f = read_ints(n), g = read_ints(m);

  Solver sol(f, g);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<i64> q = sol.quotient(), r = sol.remainder();
  printf("%d %d\n", (int)q.size(), (int)r.size());
  if (!q.empty()) print_all(q, ' ');
  if (!r.empty()) print_all(r, ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
