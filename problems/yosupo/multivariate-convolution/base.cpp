// https://judge.yosupo.jp/problem/multivariate_convolution
//
// k と各次元の大きさ dim、n = ∏ dim 個の f と g が与えられ、各次元で dim_i 以上の次数を落とした積を出す。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<int>& dim, const vector<i64>& f, const vector<i64>& g);  // 構築。計測区間の外。
//     void run();                                                                  // 積。ここだけ測る。
//     vector<i64> answer() const;                                                  // n 個の係数
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-polynomial.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int k;
 must_scan(scanf("%d", &k), 1);
 vector<int> dim(k);
 for (auto& d: dim) must_scan(scanf("%d", &d), 1);
 int n= 1;
 for (int d: dim) n*= d;
 vector<i64> f = read_ints(n), g = read_ints(n);

 Solver sol(dim, f, g);

 auto t0 = chrono::steady_clock::now();
 sol.run();
 auto t1 = chrono::steady_clock::now();

 print_all(sol.answer(), ' ');
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
