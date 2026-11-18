// 多項式 a (長さ n) を b (長さ m、最高次の係数は 0 でない) で割った商と余り (mod 998244353) を、大きさごとに分けたケースで
// 測る。各ケースの入力は「n m T seed」の 1 行で、ハーネスが seed から a と b の組を T 個作り (計測の外)、提出の run を T 回
// 呼ぶ。計測は T 回の和、出力は答え全部の hash を 1 つ。素朴な実装と SIMD の実装を、大きさごとのケースの時間で比べるための問題。
//
// 提出は次を実装する。商は長さ n - m + 1、余りは長さ m - 1 (末尾の 0 も含めて) で返す (n ≥ m ≥ 1)。
//   pair<vector<u32>, vector<u32>> run(const vector<u32> &a, const vector<u32> &b);
#include <utility>
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
  vector<vector<u32>> as(t, vector<u32>(n)), bs(t, vector<u32>(m));
  for (int q = 0; q < t; ++q) {
    for (auto &x : as[q]) x = u32(sm_next(seed) % 998244353);
    for (auto &x : bs[q]) x = u32(sm_next(seed) % 998244353);
    while (bs[q][m - 1] == 0) bs[q][m - 1] = u32(sm_next(seed) % 998244353);
  }
  vector<pair<vector<u32>, vector<u32>>> res(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) res[q] = run(as[q], bs[q]);
  auto t1 = chrono::steady_clock::now();
  u64 h = 0;
  for (auto &[qq, rr] : res) {
    h = h * 1000003 + qq.size() * 7 + rr.size();
    for (u32 x : qq) h = h * 0x100000001b3ull + x;
    for (u32 x : rr) h = h * 0x100000001b3ull + x;
  }
  printf("%llu\n", h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
