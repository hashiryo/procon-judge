// https://judge.yosupo.jp/problem/sum_of_exponential_times_polynomial_limit
//
// 提出は次を実装する。
//   u32 run(u32 r, u32 d);
// Σ_{i=0}^{∞} r^i i^d mod 998244353 を返す (r ≠ 1、0 ≤ d ≤ 10^7、0^0 = 1)。r は -1 < r < 1 の有理数を mod で表したもので、
// 和は (1 - r)^{d+1} を分母に持つ有理式になる。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  u32 r, d;
  must_scan(scanf("%u %u", &r, &d), 2);
  auto t0 = chrono::steady_clock::now();
  u32 ans = run(r, d);
  auto t1 = chrono::steady_clock::now();
  printf("%u\n", ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
