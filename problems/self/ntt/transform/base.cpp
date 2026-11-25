// self-ntt-transform: dft と idft の組を、大きさごとのケースで測る (mod 998244353)。
// 各ケースの入力は「n t fill seed」の 1 行で、ハーネスが seed から長さ n の列を 1 本作り、t 回
// 「写して (計測の外)、dft を測り、点ごとに 2 乗し (計測の外)、idft を測る」をくり返す。出力は最後の
// 巡回の自己畳み込み (dft → 2 乗 → idft) の hash。評価点の順は核が決めるので、順序が消えるこの形で
// 測る (順を固定すると、基数 4 の核が並べ替えの分だけ損をする)。小さい大きさは t を大きくして 1 回の
// 手間を、大きい大きさは t = 1 にして核の速さを見る。
//
// 提出は struct Kernel に次を実装する。
//   static void dft(u32* a, int n);   // n は 2 冪。その場で評価値に置き換える
//   static void idft(u32* a, int n);  // 逆変換 (1/n まで掛ける)
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
 vector<u32> work(n);
 long long total= 0;
 for (int q= 0; q < t; ++q) {
  copy(base.begin(), base.end(), work.begin());
  ntt_common::tweak(work, (unsigned long long)q, M);

  auto s0= chrono::steady_clock::now();
  Kernel::dft(work.data(), n);
  auto s1= chrono::steady_clock::now();

  for (auto &x: work) x= u32(u64(x) * x % M);

  auto s2= chrono::steady_clock::now();
  Kernel::idft(work.data(), n);
  auto s3= chrono::steady_clock::now();

  total+= (long long)(s1 - s0).count() + (long long)(s3 - s2).count();
 }
 printf("%llu\n", (unsigned long long)ntt_common::hash_values(work));
 report_metrics(total);
}
