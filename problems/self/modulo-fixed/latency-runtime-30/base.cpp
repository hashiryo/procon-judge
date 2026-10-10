// harness: 各 submissions/*.hpp が定義する struct MP を使い、決まった値 w を掛けて b を足す s = s * w + b を N 回くり返す時間を測る
// (mod は入力、2^20 < mod < 2^30 の奇数)。modulo-test と同じく依存の鎖なので latency を測る。
// 提出は fix(w) で掛ける値の前計算を作り、mul_fix(a, fix(w)) で掛ける。前計算をしない提出は fix が w をそのまま返す。
// fix は計測区間の外で 1 回だけ呼ぶ。設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-test/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  unsigned long long n_, mod_, s_, w_, b_;
  must_scan(scanf("%llu %llu %llu %llu %llu", &n_, &mod_, &s_, &w_, &b_), 5);

  const MP mp(mod_);
  auto s= mp.set(s_);
  const auto w= mp.fix(mp.set(w_));
  const auto b= mp.set(b_);

  auto t0= chrono::steady_clock::now();
  for (u64 i= 0; i < n_; ++i) s= mp.plus(mp.mul_fix(s, w), b);
  auto t1= chrono::steady_clock::now();

  printf("%llu\n", (unsigned long long)mp.get(s));
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  return 0;
}
