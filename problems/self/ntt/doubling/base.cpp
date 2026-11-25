// self-ntt-doubling: dft(n) と dft_doubling と idft(2n) の組を、大きさごとのケースで測る
// (mod 998244353)。各ケースの入力は「n t fill seed」の 1 行で、ハーネスが seed から長さ n の列を
// 1 本作り、t 回「写して (計測の外)、dft と dft_doubling を測り、全体を 2 乗し (計測の外)、idft(2n) を
// 測る」をくり返す。出力は 0 を詰めた入力の自己畳み込み (長さ 2n) の hash。Newton 法の inv が
// 「長さ n の変換から長さ 2n の変換を作る」形をそのまま測る。
//
// 提出は struct Kernel に次を実装する。
//   static void dft(u32* a, int n);
//   static void dft_doubling(u32* a, int n);  // a[0..n) の変換から a[n..2n) を作る
//   static void idft(u32* a, int n);
#include "pj.hpp"
#include "_shared/ntt/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int n, t, fill_mode;
 unsigned long long seed;
 must_scan(scanf("%d %d %d %llu", &n, &t, &fill_mode, &seed), 4);

 constexpr u32 M= 998244353;
 const vector<u32> base= ntt_common::make_values(n, seed, fill_mode, M);
 vector<u32> work(2 * (size_t)n);
 long long total= 0;
 for (int q= 0; q < t; ++q) {
  copy(base.begin(), base.end(), work.begin());
  fill(work.begin() + n, work.end(), 0u);
  ntt_common::tweak(work, (unsigned long long)q, M);

  auto s0= chrono::steady_clock::now();
  Kernel::dft(work.data(), n);
  auto s1= chrono::steady_clock::now();

  auto s2= chrono::steady_clock::now();
  Kernel::dft_doubling(work.data(), n);
  auto s3= chrono::steady_clock::now();

  for (auto &x: work) x= u32(u64(x) * x % M);

  auto s4= chrono::steady_clock::now();
  Kernel::idft(work.data(), 2 * n);
  auto s5= chrono::steady_clock::now();

  total+= (long long)(s1 - s0).count() + (long long)(s3 - s2).count() + (long long)(s5 - s4).count();
 }
 printf("%llu\n", (unsigned long long)ntt_common::hash_values(work));
 report_metrics(total);
}
