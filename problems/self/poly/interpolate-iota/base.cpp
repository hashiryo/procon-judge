// y_i = f(i) (0 ≤ i < K) を満たす K-1 次以下の多項式 f の f(x) (mod 998244353) を、点の数ごとに分けたケースで測る。各ケースの
// 入力は「K T seed」の 1 行で、ハーネスが seed から y と x (K 以上 2^60 + K 未満) の組を T 個作り (計測の外)、提出の run を
// T 回呼ぶ。計測は T 回の和、出力は答え全部の hash を 1 つ。「答えが n の多項式になる」形の補間を、素朴な実装と SIMD の実装で
// 比べるための問題。
//
// 提出は次を実装する。
//   u32 run(const vector<u32> &y, u64 x);
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
  int k, t;
  u64 seed;
  must_scan(scanf("%d %d %llu", &k, &t, &seed), 3);
  vector<vector<u32>> ys(t, vector<u32>(k));
  vector<u64> xs(t);
  for (int q = 0; q < t; ++q) {
    for (auto &v : ys[q]) v = u32(sm_next(seed) % 998244353);
    xs[q] = u64(k) + (sm_next(seed) >> 4);
  }
  vector<u32> ans(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) ans[q] = run(ys[q], xs[q]);
  auto t1 = chrono::steady_clock::now();
  u64 h = 0;
  for (u32 v : ans) h = h * 0x100000001b3ull + v;
  printf("%llu\n", h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
