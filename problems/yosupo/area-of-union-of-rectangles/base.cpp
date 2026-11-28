// https://judge.yosupo.jp/problem/area_of_union_of_rectangles
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<long long> l, vector<long long> d,
//            vector<long long> r, vector<long long> u);  // 構築。計測区間の外。
//     void run();              // ここだけ測る。
//     long long area() const;  // 和集合の面積
//   };
//
// 長方形 i は { (x,y) : l_i <= x <= r_i, d_i <= y <= u_i }。座標は整数で、
// 面積は (r-l)*(u-d) の足し合わせになる (端を含むが、幅は r-l でよい)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<long long> l(n), d(n), r(n), u(n);
  for (int i = 0; i < n; ++i) {
    must_scan(scanf("%lld %lld %lld %lld", &l[i], &d[i], &r[i], &u[i]), 4);
  }

  Solver sol(n, l, d, r, u);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.area());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
