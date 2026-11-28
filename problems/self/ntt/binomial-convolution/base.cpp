// self-ntt-binomial-convolution: 二項畳み込み c[k] = Σ_i C(k, i) a[i] b[k-i] (mod 998244353) を、形ごとの
// ケースで測る。各ケースの入力は「n m t fill same seed」の 1 行で、ハーネスが seed から長さ n と m の
// 列を作り、t 回「写して (計測の外)、convolve(a, n, b, m, c) を測る」をくり返す。
// same = 1 は a と b に同じ配列を渡す (n = m)。出力は答えの列全部の hash。同じ長さ、細長い、
// 2 冪を少し超える長さ、全部 P - 1 の入力で、道具の分岐と速さを見る。
//
// 提出は struct Kernel に次を実装する。
//   static void convolve(const u32* a, int n, const u32* b, int m, u32* c);
//   // c[k] = Σ_i C(k, i) a[i] b[k-i] (0 ≤ k < n + m - 1)。c は a、b と重ならない
#include "pj.hpp"
#include "_shared/ntt/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int n, m, t, fill_mode, same;
 unsigned long long seed;
 must_scan(scanf("%d %d %d %d %d %llu", &n, &m, &t, &fill_mode, &same, &seed), 6);
 if (same && n != m) {
  fprintf(stderr, "same = 1 には n = m が要ります\n");
  return 1;
 }

 constexpr u32 M= 998244353;
 const vector<u32> base_a= ntt_common::make_values(n, seed * 3 + 0, fill_mode, M);
 const vector<u32> base_b= same ? vector<u32>() : ntt_common::make_values(m, seed * 3 + 1, fill_mode, M);
 vector<u32> wa(n), wb(same ? 0 : m), c(n + m - 1);
 long long total= 0;
 for (int q= 0; q < t; ++q) {
  copy(base_a.begin(), base_a.end(), wa.begin());
  if (!same) copy(base_b.begin(), base_b.end(), wb.begin());
  ntt_common::tweak(wa, (unsigned long long)q, M);

  auto s0= chrono::steady_clock::now();
  Kernel::convolve(wa.data(), n, same ? wa.data() : wb.data(), m, c.data());
  auto s1= chrono::steady_clock::now();

  total+= (long long)(s1 - s0).count();
 }
 printf("%llu\n", (unsigned long long)ntt_common::hash_values(c));
 report_metrics(total);
}
