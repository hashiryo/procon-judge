// harness: 各 submissions/*.hpp が定義する struct MP を使い、配列の要素ごとの積和 c[i] = c[i] * a[i] + b[i] を
// R 回くり返す時間を測る (mod は入力、2^20 < mod < 2^30 の偶数)。modulo-test と違って要素どうしは独立なので、
// latency ではなく throughput を測る。値の型は decltype(mp.set(0u)) で決まり、u64 で持つ実装は配列も u64 になる。
// 配列の値の作り方、計算、ハッシュは _shared/modulo-throughput/_common.hpp にある。提出は modulo-test の
// runtime-30-even から写した。設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-throughput/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  unsigned long long n_, r_, seed_, mod_;
  int fill_;
  must_scan(scanf("%llu %llu %llu %d %llu", &n_, &r_, &seed_, &fill_, &mod_), 5);

  const MP mp(mod_);
  using V= decltype(mp.set(0u));
  const size_t n= n_;
  const u32 mod= u32(mod_);
  vector<V> a(n), b(n), c(n);
  {
    auto va= modulo_throughput::make_values(n, seed_ * 3 + 0, fill_, mod);
    auto vb= modulo_throughput::make_values(n, seed_ * 3 + 1, fill_, mod);
    auto vc= modulo_throughput::make_values(n, seed_ * 3 + 2, fill_, mod);
    for (size_t i= 0; i < n; ++i) a[i]= mp.set(va[i]), b[i]= mp.set(vb[i]), c[i]= mp.set(vc[i]);
  }

  auto t0= chrono::steady_clock::now();
  for (u64 r= 0; r < r_; ++r) modulo_throughput::step(mp, c.data(), a.data(), b.data(), n);
  auto t1= chrono::steady_clock::now();

  printf("%llu\n", (unsigned long long)modulo_throughput::hash_values(mp, c));
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  return 0;
}
