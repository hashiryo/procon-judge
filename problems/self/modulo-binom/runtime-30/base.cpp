// harness: 各 submissions/*.hpp が定義する struct MP を使い、階乗と階乗の逆元の表を作ってから、二項係数の問い合わせに答える時間を測る
// (法 p は入力、30 bit の素数)。コンテストで ModInt を使う形のうち、いちばんよく書く組合せの計算を真似る。
// 計測区間には、整数から MP の表現への変換 (set(i))、両方の値が変わる掛け算の鎖 (表を作る段)、フェルマーの小定理による逆元、
// 表を引く問い合わせが入る。問い合わせの (n, k) と表の領域は計測区間の外で用意する。
// 提出は modulo-test と同じ struct MP で、呼ぶのは set、get、plus、mul の 4 つ。累乗はハーネスが mul で書く。
// 設計は algo-notes の notes/modular_arithmetic/modint_problems.md。
#include "pj.hpp"
#include "_shared/modulo-throughput/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  unsigned long long n_, q_, seed_, p_;
  must_scan(scanf("%llu %llu %llu %llu", &n_, &q_, &seed_, &p_), 4);

  const MP mp(p_);
  using V= decltype(mp.set(0u));
  const u32 N= u32(n_);
  const size_t Q= q_;
  vector<u32> qn(Q), qk(Q);
  {
    modulo_throughput::SplitMix64 rng(seed_);
    for (size_t j= 0; j < Q; ++j) {
      u32 n= u32(rng.below(u64(N) + 1));
      qn[j]= n, qk[j]= u32(rng.below(u64(n) + 1));
    }
  }
  vector<V> fact(N + 1), ifact(N + 1);

  auto t0= chrono::steady_clock::now();
  fact[0]= mp.set(1);
  for (u32 i= 1; i <= N; ++i) fact[i]= mp.mul(fact[i - 1], mp.set(i));
  V x= fact[N], r= mp.set(1);
  for (u64 e= p_ - 2; e; e>>= 1) {
    if (e & 1) r= mp.mul(r, x);
    x= mp.mul(x, x);
  }
  ifact[N]= r;
  for (u32 i= N; i >= 1; --i) ifact[i - 1]= mp.mul(ifact[i], mp.set(i));
  V acc= mp.set(0);
  for (size_t j= 0; j < Q; ++j) acc= mp.plus(acc, mp.mul(mp.mul(fact[qn[j]], ifact[qk[j]]), ifact[qn[j] - qk[j]]));
  auto t1= chrono::steady_clock::now();

  printf("%llu\n", (unsigned long long)mp.get(acc));
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
  return 0;
}
