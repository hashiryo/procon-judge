// 多項式係数の線形漸化式 Σ_{i=0}^{d} p_i(n) a_{n+i} = 0 の第 N 項を、階数 d と係数の次数 e のケースで測る。
// 各ケースの入力は「d e t nmax kind seed」の 1 行で、ハーネスが seed から p_0..p_d、a_0..a_{d-1}、N を t 組
// 作り (計測の外)、提出の run を t 回呼ぶ。計測は t 回の和、出力は答え全部の hash。
// kind 0 は p_d = 1、1 は p_d = 0 でない定数、2 は p_d = n + 1 (nmax は小さい)。
//
// 提出は次を実装する。
//   u32 run(const vector<vector<u32>> &p, const vector<u32> &a0, u64 N);
//   // p[i][j] は p_i の n^j の係数 (0 <= i <= d、0 <= j <= e)、a0 は a_0..a_{d-1}
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
 int d, e, t, kind;
 unsigned long long nmax, seed;
 must_scan(scanf("%d %d %d %llu %d %llu", &d, &e, &t, &nmax, &kind, &seed), 6);
 constexpr u32 M = 998244353;
 vector<vector<vector<u32>>> ps(t, vector<vector<u32>>(d + 1, vector<u32>(e + 1)));
 vector<vector<u32>> as(t, vector<u32>(d));
 vector<u64> ns(t);
 for (int q = 0; q < t; ++q) {
  for (int i = 0; i <= d; ++i)
   for (int j = 0; j <= e; ++j) ps[q][i][j] = u32(sm_next(seed) % M);
  for (auto &x : ps[q][d]) x = 0;
  if (kind == 0) ps[q][d][0] = 1;
  else if (kind == 1) ps[q][d][0] = u32(1 + sm_next(seed) % (M - 1));
  else ps[q][d][0] = 1, ps[q][d][1] = 1;  // n + 1
  for (auto &x : as[q]) x = u32(sm_next(seed) % M);
  const u64 r = sm_next(seed);
  // q = 0 はいちばん大きい N (ケースの主役)、1..5 は端、それ以降は乱数。
  ns[q] = q == 0 ? nmax - 1 : q == 1 ? 0 : q == 2 ? std::min<u64>(1, nmax - 1) : q == 3 ? std::min<u64>(d, nmax - 1) : q == 4 ? std::min<u64>(d + 1, nmax - 1) : q == 5 ? nmax / 2 : r % nmax;
 }
 vector<u32> ans(t);
 auto t0 = chrono::steady_clock::now();
 for (int q = 0; q < t; ++q) ans[q] = run(ps[q], as[q], ns[q]);
 auto t1 = chrono::steady_clock::now();
 u64 h = 0;
 for (u32 x : ans) h = h * 0x100000001b3ull + x;
 printf("%llu\n", h);
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
