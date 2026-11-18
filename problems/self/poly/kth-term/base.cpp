// 定数係数の線形漸化式 a_n = Σ_{j=1}^{d} c_j a_{n-j} (mod 998244353) の第 k 項を、次数ごとに分けたケースで測る。各ケースの
// 入力は「d T kmax seed」の 1 行で、ハーネスが seed から初項 a_0..a_{d-1}、係数 c_1..c_d、k (kmax 未満) の組を T 個作り
// (計測の外)、提出の run を T 回呼ぶ。計測は T 回の和、出力は答え全部の hash を 1 つ。FFT を使わない素朴な実装と SIMD の
// 実装を、次数ごとのケースの時間で比べるための問題。
//
// 提出は次を実装する。
//   u32 run(const vector<u32> &a, const vector<u32> &c, u64 k);  // a = a_0..a_{d-1}、c = c_1..c_d
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP
inline u64 sm_next(u64 &s) {
  u64 z = (s += 0x9e3779b97f4a7c15ull);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
  return z ^ (z >> 31);
}
signed main() {
  int d, t;
  u64 kmax, seed;
  must_scan(scanf("%d %d %llu %llu", &d, &t, &kmax, &seed), 4);
  vector<vector<u32>> as(t, vector<u32>(d)), cs(t, vector<u32>(d));
  vector<u64> ks(t);
  for (int q = 0; q < t; ++q) {
    for (auto &x : as[q]) x = u32(sm_next(seed) % 998244353);
    for (auto &x : cs[q]) x = u32(sm_next(seed) % 998244353);
    ks[q] = sm_next(seed) % kmax;
  }
  vector<u32> ans(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) ans[q] = run(as[q], cs[q], ks[q]);
  auto t1 = chrono::steady_clock::now();
  u64 h = 0;
  for (u32 x : ans) h = h * 0x100000001b3ull + x;
  printf("%llu\n", h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
