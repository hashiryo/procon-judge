// harness: T 個の x (≠ 0) を読み、log_g(x) ∈ [0, 2^64-2] を出力する。g = 2 (多項式基底の x) は原始元。
// 旧 judge の gf2-64-log から移した。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/pclmul.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<u64> as(t);
  for (int i = 0; i < t; ++i) must_scan(scanf("%llu", &as[i]), 1);

  auto t0 = chrono::steady_clock::now();
  auto r = GF2_64Op::run(as);
  auto t1 = chrono::steady_clock::now();

  print_all(r);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}

// _shared/gf2-64/_common.hpp が開いた宣言の領域を閉じる。clang は翻訳単位の中で閉じる必要がある。
#ifdef GF2_64_TARGET_END
GF2_64_TARGET_END
#endif
