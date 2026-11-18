// 長さ n の多項式 f を m 個の点 x_j で評価する (mod 998244353) のを、大きさごとに分けたケースで測る。素朴な O(nm) の
// Horner 法を、素朴な実装と SIMD の実装で比べるための問題。各ケースの入力は「n m T seed」の 1 行で、ハーネスが seed から
// f と点の組を T 個作り (計測の外)、提出の run を T 回呼ぶ。計測は T 回の和、出力は答え全部の hash を 1 つ。
//
// 提出は次を実装する。f(x_j) を j の順に返す (値は M 未満、点は重なってもよい)。
//   vector<u32> run(const vector<u32> &f, const vector<u32> &x);
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
  vector<vector<u32>> fs(t, vector<u32>(n)), xs(t, vector<u32>(m));
  for (int q = 0; q < t; ++q) {
    for (auto &a : fs[q]) a = u32(sm_next(seed) % 998244353);
    for (auto &a : xs[q]) a = u32(sm_next(seed) % 998244353);
  }
  vector<vector<u32>> res(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) res[q] = run(fs[q], xs[q]);
  auto t1 = chrono::steady_clock::now();
  u64 h = 0;
  for (auto &g : res) {
    h = h * 1000003 + g.size();
    for (u32 v : g) h = h * 0x100000001b3ull + v;
  }
  printf("%llu\n", h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
