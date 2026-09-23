// harness: T 個の (a, b) を読み、a / b ∈ GF(2^64) を出力する (b ≠ 0)。旧 judge の gf2-64-div から移した。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<u64> as(t), bs(t);
  for (int i = 0; i < t; ++i) must_scan(scanf("%llu %llu", &as[i], &bs[i]), 2);

  auto t0 = chrono::steady_clock::now();
  auto r = GF2_64Op::run(as, bs);
  auto t1 = chrono::steady_clock::now();

  print_all(r);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
