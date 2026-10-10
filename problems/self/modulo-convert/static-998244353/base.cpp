// harness: 各 submissions/*.hpp が定義する struct MP を使い、i64 の配列を MP の表現に変換して足し込む c[i] = c[i] + from_i64(a[i]) を
// R 回くり返す時間を測る (mod = 998244353 固定)。入力で 10^18 級の値や負の値を受けて mint にする場面を真似る。
// a は符号付き 64 bit の全範囲の一様乱数 (半分は負) で、先頭には端の値 (LLONG_MIN、LLONG_MAX、-1、0、±mod など) を置く。
// 要素どうしは独立なので throughput を測る。ループは main に普通に書いて __restrict を付けない (使う側が vector で書くループと同じ形)。
// 提出が実装するのは set、get、plus、from_i64 の 4 つ。今の Library の ModInt は整数を __int128_t で受けて % mod する。
// 設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-throughput/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  constexpr u32 MOD= 998244353;
  const unsigned long long mod_= MOD;
  unsigned long long n_, r_, seed_;
  must_scan(scanf("%llu %llu %llu", &n_, &r_, &seed_), 3);

  // const にしているのは volatile 系の MP (constexpr 不可) を許容するため。
  const MP mp(MOD);
  using V= decltype(mp.set(0u));
  const size_t n= n_;
  vector<long long> a(n);
  vector<V> c(n);
  {
    modulo_throughput::SplitMix64 rng(seed_);
    for (size_t i= 0; i < n; ++i) a[i]= (long long)rng(), c[i]= mp.set(0);
    const long long m= (long long)mod_;
    const long long edge[]= {LLONG_MIN, LLONG_MAX, -1, 0, 1, -m, m, -m - 1, m + 1, LLONG_MIN + 1};
    for (size_t i= 0; i < n && i < size(edge); ++i) a[i]= edge[i];
  }

  auto t0= chrono::steady_clock::now();
  for (u64 r= 0; r < r_; ++r)
    for (size_t i= 0; i < n; ++i) c[i]= mp.plus(c[i], mp.from_i64(a[i]));
  auto t1= chrono::steady_clock::now();

  u64 h= 0;
  for (const auto &v : c) h= h * 1000003 + u64(mp.get(v));
  printf("%llu\n", (unsigned long long)h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  return 0;
}
