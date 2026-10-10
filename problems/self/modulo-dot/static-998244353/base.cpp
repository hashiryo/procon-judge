// harness: 各 submissions/*.hpp が定義する struct MP を使い、n×n の行列の積 C = A B の時間を測る (mod = 998244353 固定)。
// B は計測区間の外で転置しておき、C[i][j] = dot(A の i 行, B^T の j 行, n) とする。行列累乗や遷移行列の掛け算を真似る。
// 提出は set、get と、内積 dot(const V* a, const V* b, size_t n) を実装する。積を足してから還元を遅らせる工夫を比べる。
// A と B の要素は set に mod 未満の値を渡して作る。設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-throughput/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  constexpr u32 MOD= 998244353;
  const unsigned long long mod_= MOD;
  unsigned long long n_, seed_;
  must_scan(scanf("%llu %llu", &n_, &seed_), 2);

  // const にしているのは volatile 系の MP (constexpr 不可) を許容するため。
  const MP mp(MOD);
  using V= decltype(mp.set(0u));
  const size_t n= n_;
  vector<V> A(n * n), BT(n * n), C(n * n);
  {
    modulo_throughput::SplitMix64 rng(seed_);
    for (auto &x : A) x= mp.set(u32(rng.below(mod_)));
    vector<V> B(n * n);
    for (auto &x : B) x= mp.set(u32(rng.below(mod_)));
    for (size_t i= 0; i < n; ++i)
      for (size_t j= 0; j < n; ++j) BT[j * n + i]= B[i * n + j];
  }

  auto t0= chrono::steady_clock::now();
  for (size_t i= 0; i < n; ++i)
    for (size_t j= 0; j < n; ++j) C[i * n + j]= mp.dot(&A[i * n], &BT[j * n], n);
  auto t1= chrono::steady_clock::now();

  u64 h= 0;
  for (const auto &v : C) h= h * 1000003 + u64(mp.get(v));
  printf("%llu\n", (unsigned long long)h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  return 0;
}
