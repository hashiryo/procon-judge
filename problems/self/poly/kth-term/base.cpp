// 定数係数の線形漸化式 a_n = Σ_{j=1}^{d} c_j a_{n-j} (mod 998244353) の第 k 項 (0 ≤ k ≤ 10^18) を、ケースごとに求める。
// 次数 d は 1 以上 3000 以下で、FFT を使わない範囲 (O(d^2 log k)) の大きさ。次数の小さいケースを大量に聞く入力もある。
//
// 提出は次を実装する。
//   u32 run(const vector<u32> &a, const vector<u32> &c, u64 k);  // a = a_0..a_{d-1}、c = c_1..c_d
// 計測は全ケースの run の時間の和。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<vector<u32>> as(t), cs(t);
  vector<u64> ks(t);
  for (int q = 0; q < t; ++q) {
    int d;
    must_scan(scanf("%d %llu", &d, &ks[q]), 2);
    as[q].resize(d), cs[q].resize(d);
    for (auto &x : as[q]) must_scan(scanf("%u", &x), 1);
    for (auto &x : cs[q]) must_scan(scanf("%u", &x), 1);
  }
  vector<u32> ans(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) ans[q] = run(as[q], cs[q], ks[q]);
  auto t1 = chrono::steady_clock::now();
  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
