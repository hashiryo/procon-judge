// harness: 各 submissions/*.hpp が定義する struct MP を使い、配列の要素ごとに決まった値 w を掛けて足す c[i] = c[i] * w + b[i] を
// R 回くり返す時間を測る (mod は入力、2^20 < mod < 2^30 の偶数)。DP で確率や係数を掛ける形を想定する。要素どうしは独立なので
// throughput を測る。modulo-throughput と違い、ループは main に普通に書いて __restrict を付けない。使う側が vector で書く
// ループと同じ形で、GCC の -O2 はベクトル化せず、clang は別名の検査を足してベクトル化することがある。
// 提出は fix(w) で掛ける値の前計算を作り、mul_fix(a, fix(w)) で掛ける。前計算をしない提出は fix が w をそのまま返す。
// 配列の値の作り方は _shared/modulo-throughput/_common.hpp を使う。ハッシュは mp を関数へ渡さないように main の中で計算する
// (mp のアドレスが外へ出ると、配列への書き込みのたびに mp の値を読み直すことになるため)。
// 設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-throughput/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  unsigned long long n_, r_, seed_, mod_, w_;
  int fill_;
  must_scan(scanf("%llu %llu %llu %d %llu %llu", &n_, &r_, &seed_, &fill_, &mod_, &w_), 6);

  const MP mp(mod_);
  using V= decltype(mp.set(0u));
  const size_t n= n_;
  const u32 mod= u32(mod_);
  vector<V> b(n), c(n);
  {
    auto vb= modulo_throughput::make_values(n, seed_ * 3 + 1, fill_, mod);
    auto vc= modulo_throughput::make_values(n, seed_ * 3 + 2, fill_, mod);
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
