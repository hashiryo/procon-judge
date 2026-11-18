// https://judge.yosupo.jp/problem/sum_of_exponential_times_polynomial
//
// 提出は次を実装する。
//   u32 run(u32 r, u32 d, u64 n);
// Σ_{i=0}^{n-1} r^i i^d mod 998244353 を返す (0 ≤ r < 998244353、0 ≤ d ≤ 10^7、0 ≤ n ≤ 10^18、0^0 = 1)。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  u32 r, d;
  u64 n;
  must_scan(scanf("%u %u %llu", &r, &d, &n), 3);
  auto t0 = chrono::steady_clock::now();
  u32 ans = run(r, d, n);
  auto t1 = chrono::steady_clock::now();
  printf("%u\n", ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
