// 素朴な畳み込み c_k = Σ_{i+j=k} a_i b_j (mod 998244353) を、大きさごとに分けたケースで測る。各ケースの入力は
// 「n m T seed」の 1 行で、ハーネスが seed から長さ n と m の列の組を T 個作り (計測の外)、提出の run を T 回呼ぶ。
// 計測は T 回の和、出力は答えの列全部の hash を 1 つ。小さい大きさは T を大きくして 1 回の手間を、大きい大きさは T = 1
// にして核の速さを見る。FFT を使わない素朴な実装と SIMD の実装を、大きさごとのケースの時間で比べるための問題。
//
// 提出は次を実装する。
//   vector<u32> run(const vector<u32> &a, const vector<u32> &b);  // 長さ n + m - 1
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
  int n, m, t;
  u64 seed;
  must_scan(scanf("%d %d %d %llu", &n, &m, &t, &seed), 4);
  vector<vector<u32>> as(t, vector<u32>(n)), bs(t, vector<u32>(m)), cs(t);
  for (int q = 0; q < t; ++q) {
    for (auto &x : as[q]) x = u32(sm_next(seed) % 998244353);
    for (auto &x : bs[q]) x = u32(sm_next(seed) % 998244353);
  }
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) cs[q] = run(as[q], bs[q]);
  auto t1 = chrono::steady_clock::now();
  u64 h = 0;
  for (auto &c : cs) {
    h = h * 1000003 + c.size();
    for (u32 x : c) h = h * 0x100000001b3ull + x;
  }
  printf("%llu\n", h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
