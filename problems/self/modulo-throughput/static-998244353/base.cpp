// harness: 各 submissions/*.hpp が定義する struct MP を使い、配列の要素ごとの積和 c[i] = c[i] * a[i] + b[i] を
// R 回くり返す時間を測る (mod = 998244353 固定)。modulo-test と違って要素どうしは独立なので、latency ではなく
// throughput を測る。値の型は decltype(mp.set(0u)) で決まり、u64 で持つ実装は配列も u64 になる。
// 配列の値の作り方、計算、ハッシュは _shared/modulo-throughput/_common.hpp にある。提出は modulo-test の
// static-998244353 から写した。設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-throughput/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  constexpr u32 MOD= 998244353;
  unsigned long long n_, r_, seed_;
  int fill_;
  must_scan(scanf("%llu %llu %llu %d", &n_, &r_, &seed_, &fill_), 4);

  // const にしているのは volatile 系の MP (constexpr 不可) を許容するため。
  const MP mp(MOD);
  using V= decltype(mp.set(0u));
  const size_t n= n_;
  vector<V> a(n), b(n), c(n);
  {
    auto va= modulo_throughput::make_values(n, seed_ * 3 + 0, fill_, MOD);
    auto vb= modulo_throughput::make_values(n, seed_ * 3 + 1, fill_, MOD);
    auto vc= modulo_throughput::make_values(n, seed_ * 3 + 2, fill_, MOD);
    for (size_t i= 0; i < n; ++i) a[i]= mp.set(va[i]), b[i]= mp.set(vb[i]), c[i]= mp.set(vc[i]);
  }

  auto t0= chrono::steady_clock::now();
  for (u64 r= 0; r < r_; ++r) modulo_throughput::step(mp, c.data(), a.data(), b.data(), n);
  auto t1= chrono::steady_clock::now();

  printf("%llu\n", (unsigned long long)modulo_throughput::hash_values(mp, c));
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  return 0;
}
