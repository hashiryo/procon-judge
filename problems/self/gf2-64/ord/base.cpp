// harness: T 個の a (0 でない) を読み、GF(2^64)^× での a の位数 (a^k = 1 となる最小の k ≥ 1) を 1 つずつ出力する。
// 2^64-1 = 3·5·17·257·641·65537·6700417 は平方因子を持たないので、位数は a^((2^64-1)/p) ≠ 1 となる素数 p の積になる。
// 入出力は計測区間の外。run(as) だけを測る。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<u64> as(t);
  for (int i = 0; i < t; ++i) must_scan(scanf("%llu", &as[i]), 1);

  auto t0 = chrono::steady_clock::now();
  auto r = run(as);
  auto t1 = chrono::steady_clock::now();

  print_all(r);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}

// _shared/gf2-64/_common.hpp が開いた宣言の領域を閉じる。clang は翻訳単位の中で閉じる必要がある。
#ifdef GF2_64_TARGET_END
GF2_64_TARGET_END
#endif
