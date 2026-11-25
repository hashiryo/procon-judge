// self-ntt-range: 範囲の積 c[k - lo] = Σ_{i+j=k} a[i] b[j] (lo ≤ k < hi) を、範囲ごとのケースで測る
// (mod 998244353)。各ケースの入力は「n m lo hi t fill mode seed」の 1 行で、ハーネスが seed から
// 長さ n と m の列を作り、t 回「写して (計測の外)、convolve_range を測る」をくり返す。出力は c の hash。
// mode = 1 は b を反転してから渡し、c[i] = Σ_j a[i+j] b[j] の middle product (Bostan–Mori の形) にする。
// Newton 法の inv の段 (n = 2m、[m, 2m))、先頭、末尾、中ほどの範囲を、大きさごとのケースに分ける。
//
// 提出は struct Kernel に次を実装する。
//   static void convolve_range(const u32* a, int n, const u32* b, int m, int lo, int hi, u32* c);
//   // c[k - lo] = Σ_{i+j=k} a[i] b[j] (lo ≤ k < hi)。c は a、b と重ならない
#include "pj.hpp"
#include "_shared/ntt/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int n, m, lo, hi, t, fill_mode, mode;
 unsigned long long seed;
 must_scan(scanf("%d %d %d %d %d %d %d %llu", &n, &m, &lo, &hi, &t, &fill_mode, &mode, &seed), 8);
 if (lo < 0 || lo >= hi || hi > n + m - 1) {
  fprintf(stderr, "0 ≤ lo < hi ≤ n + m - 1 が要ります\n");
  return 1;
 }

 constexpr u32 M= 998244353;
 const vector<u32> base_a= ntt_common::make_values(n, seed * 3 + 0, fill_mode, M);
 vector<u32> base_b= ntt_common::make_values(m, seed * 3 + 1, fill_mode, M);
 if (mode == 1) reverse(base_b.begin(), base_b.end());
 vector<u32> wa(n), wb(m), c(hi - lo);
 long long total= 0;
 for (int q= 0; q < t; ++q) {
  copy(base_a.begin(), base_a.end(), wa.begin());
  copy(base_b.begin(), base_b.end(), wb.begin());
  ntt_common::tweak(wa, (unsigned long long)q, M);

  auto s0= chrono::steady_clock::now();
  Kernel::convolve_range(wa.data(), n, wb.data(), m, lo, hi, c.data());
  auto s1= chrono::steady_clock::now();

  total+= (long long)(s1 - s0).count();
 }
 printf("%llu\n", (unsigned long long)ntt_common::hash_values(c));
 report_metrics(total);
}
