// 列 a_0..a_{N-1} (mod 998244353) を生む最短の線形漸化式 a_i = Σ_{j=1}^{d} c_j a_{i-j} を、長さごとに分けたケースで求める。
// 各ケースの入力は「N T seed mode」の 1 行で、ハーネスが seed から長さ N の列を T 本作り (計測の外)、提出の run を T 回呼ぶ。
// mode = 0 なら半分は乱数の列、半分は次数 N/4 の漸化式の列。mode = 1 (愚直解との突き合わせ用) はすべて漸化式の列で、
// 最短の漸化式が 1 つに決まる (N ≥ 2 × 次数)。計測は T 回の和、出力は答え (長さと係数) 全部の hash を 1 つ。素朴な実装と
// SIMD の実装を、長さごとのケースの時間で比べるための問題。Berlekamp–Massey の答えは処理の順で決まるので、どの提出も同じ
// 係数を返す前提 (最短の漸化式が 1 つに決まらない列もある)。
//
// 提出は次を実装する。
//   vector<u32> run(const vector<u32> &a);  // c_1..c_d
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
  int n, t, mode;
  u64 seed;
  must_scan(scanf("%d %d %llu %d", &n, &t, &seed, &mode), 4);
  constexpr u64 M = 998244353;
  vector<vector<u32>> as(t, vector<u32>(n)), cs(t);
  for (int q = 0; q < t; ++q) {
    if ((q & 1) || mode == 1) {  // 次数 n/4 (1 以上) の漸化式の列
      const int d = max(1, n / 4);
      vector<u64> c(d);
      for (auto &x : c) x = sm_next(seed) % M;
      for (int i = 0; i < n; ++i) {
        if (i < d) as[q][i] = u32(sm_next(seed) % M);
        else {
          u64 v = 0;
          for (int j = 1; j <= d; ++j) v = (v + c[j - 1] * as[q][i - j]) % M;
          as[q][i] = u32(v);
        }
      }
    } else
      for (auto &x : as[q]) x = u32(sm_next(seed) % M);
  }
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) cs[q] = run(as[q]);
  auto t1 = chrono::steady_clock::now();
  u64 h = 0;
  for (auto &c : cs) {
    h = h * 1000003 + c.size();
    for (u32 x : c) h = h * 0x100000001b3ull + x;
  }
  printf("%llu\n", h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
