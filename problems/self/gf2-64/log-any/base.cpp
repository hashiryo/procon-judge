// harness: T 組の (a, b) (どちらも 0 でない) を読み、a^k = b となる k ∈ [0, 2^64-2] を 1 つずつ出力する。
// 解が無ければ 2^64-1 を出力する。k は ord(a) を法としてしか決まらないので、判定は checker.cpp が
// a^k = b を確かめて行う。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<u64> as(t), bs(t);
  for (int i = 0; i < t; ++i) must_scan(scanf("%llu %llu", &as[i], &bs[i]), 2);

  auto t0 = chrono::steady_clock::now();
  auto r = run(as, bs);
  auto t1 = chrono::steady_clock::now();

  print_all(r);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}

// _shared/gf2-64/_common.hpp が開いた宣言の領域を閉じる。clang は翻訳単位の中で閉じる必要がある。
#ifdef GF2_64_TARGET_END
GF2_64_TARGET_END
#endif
