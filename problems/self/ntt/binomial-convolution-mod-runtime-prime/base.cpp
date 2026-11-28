// self-ntt-binomial-convolution-mod-runtime-prime: 二項畳み込み c[k] = Σ_i C(k, i) a[i] b[k-i] (mod M) を、
// 形ごとのケースで測る。法 M を入力で受けて測る。ケースの M は素数で、n + m - 1 より大きいもの
// (階乗で分ける道) と、小さいもの (M の素因数が階乗を割り切るので、p を抜いた階乗で分ける道) を混ぜる。
// 各ケースの入力は「n m t fill same mod seed」の 1 行で、ハーネスが seed から長さ n と m の列を作り、
// t 回「写して (計測の外)、convolve(a, n, b, m, c, M) を測る」をくり返す。
// same = 1 は a と b に同じ配列を渡す (n = m)。出力は答えの列全部の hash。
//
// 提出は struct Kernel に次を実装する。
//   static void convolve(const u32* a, int n, const u32* b, int m, u32* c, u32 mod);
//   // c[k] = Σ_i C(k, i) a[i] b[k-i] (0 ≤ k < n + m - 1)。c は a、b と重ならない。a、b の値は mod 未満
#include "pj.hpp"
#include "_shared/ntt/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int n, m, t, fill_mode, same;
 unsigned mod;
 unsigned long long seed;
 must_scan(scanf("%d %d %d %d %d %u %llu", &n, &m, &t, &fill_mode, &same, &mod, &seed), 7);
 if (same && n != m) {
  fprintf(stderr, "same = 1 には n = m が要ります\n");
  return 1;
 }

 const vector<u32> base_a= ntt_common::make_values(n, seed * 3 + 0, fill_mode, mod);
 const vector<u32> base_b= same ? vector<u32>() : ntt_common::make_values(m, seed * 3 + 1, fill_mode, mod);
 vector<u32> wa(n), wb(same ? 0 : m), c(n + m - 1);
 long long total= 0;
 for (int q= 0; q < t; ++q) {
  copy(base_a.begin(), base_a.end(), wa.begin());
  if (!same) copy(base_b.begin(), base_b.end(), wb.begin());
  ntt_common::tweak(wa, (unsigned long long)q, mod);

  auto s0= chrono::steady_clock::now();
  Kernel::convolve(wa.data(), n, same ? wa.data() : wb.data(), m, c.data(), mod);
  auto s1= chrono::steady_clock::now();

  total+= (long long)(s1 - s0).count();
 }
 printf("%llu\n", (unsigned long long)ntt_common::hash_values(c));
 report_metrics(total);
}
