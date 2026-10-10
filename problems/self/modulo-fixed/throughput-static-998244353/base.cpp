// harness: 各 submissions/*.hpp が定義する struct MP を使い、配列の要素ごとに決まった値 w を掛けて足す c[i] = c[i] * w + b[i] を
// R 回くり返す時間を測る (mod = 998244353 固定)。throughput-runtime-30 の法をコンパイル時の定数にした版で、掛ける値 w は
// 入力で与える実行時の値のまま。ループは main に普通に書いて __restrict を付けない (使う側が vector で書くループと同じ形)。
// 提出は fix(w) で掛ける値の前計算を作り、mul_fix(a, fix(w)) で掛ける。前計算をしない提出は fix が w をそのまま返す。
// 配列の値の作り方は _shared/modulo-throughput/_common.hpp を使う。ハッシュは mp を関数へ渡さないように main の中で計算する。
// 設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-throughput/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  constexpr u32 MOD= 998244353;
  unsigned long long n_, r_, seed_, w_;
  int fill_;
  must_scan(scanf("%llu %llu %llu %d %llu", &n_, &r_, &seed_, &fill_, &w_), 5);

  // const にしているのは volatile 系の MP (constexpr 不可) を許容するため。
  const MP mp(MOD);
  using V= decltype(mp.set(0u));
  const size_t n= n_;
  vector<V> b(n), c(n);
  {
    auto vb= modulo_throughput::make_values(n, seed_ * 3 + 1, fill_, MOD);
    auto vc= modulo_throughput::make_values(n, seed_ * 3 + 2, fill_, MOD);
    for (size_t i= 0; i < n; ++i) b[i]= mp.set(vb[i]), c[i]= mp.set(vc[i]);
  }
  const auto w= mp.fix(mp.set(w_));

  auto t0= chrono::steady_clock::now();
  for (u64 r= 0; r < r_; ++r)
    for (size_t i= 0; i < n; ++i) c[i]= mp.plus(mp.mul_fix(c[i], w), b[i]);
  auto t1= chrono::steady_clock::now();

  u64 h= 0;
  for (const auto &v : c) h= h * 1000003 + u64(mp.get(v));
  printf("%llu\n", (unsigned long long)h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  return 0;
}
