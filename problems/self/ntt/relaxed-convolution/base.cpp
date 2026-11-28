// self-ntt-relaxed-convolution: relaxed (オンライン) の畳み込みを、形ごとのケースで測る (mod 998244353)。
// 各ケースの入力は「n m t fill same seed」の 1 行で、ハーネスが seed から長さ n と m の列 (f と g) を作り、
// t 回「f の 1 個を変えて (計測の外)、新しい Solver を作り (計測の外)、run() を測る」をくり返す。
// same = 1 は f と g を同じ値にする (n = m)。出力は t 回分の答えの列全部の hash。
//
// 提出は struct Solver に次を実装する。
//   Solver(const std::vector<u32>& f, const std::vector<u32>& g);  // f と g を全部受ける (構築は計測の外)
//   void run();                                                   // 係数を 1 つずつ渡して積の係数を作る。ここだけ測る
//   const std::vector<u32>& answer() const;                       // n + m - 1 個。run() のあとに読む
// run() の中では i = 0 .. n + m - 2 について、f_i (i < n のとき) と g_i (i < m のとき) を渡し (範囲の外は 0)、
// 返り値を answer[i] に置く。Solver は t 回作るので、確保はそのたびに起きて計測の中に入る。
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
 if (n < 1 || m < 1) {
  fprintf(stderr, "n と m は 1 以上が要ります\n");
  return 1;
 }

 constexpr u32 M= 998244353;
 const vector<u32> base_a= ntt_common::make_values(n, seed * 3 + 0, fill_mode, M);
 const vector<u32> base_b= same ? base_a : ntt_common::make_values(m, seed * 3 + 1, fill_mode, M);
 vector<u32> fa= base_a, ga= base_b;
 long long total= 0;
 unsigned long long h= 0x100000001b3ull;
 for (int q= 0; q < t; ++q) {
  // 回ごとに入力を変える。同じ計算をコンパイラがループの外へ出すのを防ぐ。
  // same = 1 のときは ga が fa の写しなので、両方に同じ tweak を掛けて f と g を同じ値に保つ。
  ntt_common::tweak(fa, (unsigned long long)q, M);
  ntt_common::tweak(ga, (unsigned long long)q, M);

  Solver solver(fa, ga);  // 構築は計測の外
  auto s0= chrono::steady_clock::now();
  solver.run();
  auto s1= chrono::steady_clock::now();

  total+= (long long)(s1 - s0).count();
  h= h * 0x100000001b3ull + ntt_common::hash_values(solver.answer());
 }
 printf("%llu\n", h);
 report_metrics(total);
}
