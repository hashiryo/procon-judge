// 次数 D の多項式 f (係数は全部 0 でない) の m 乗の先頭 N 項 (mod 998244353) を、大きさごとに分けたケースで測る。
// 疎な冪級数の O(ND) の漸化式を、項の数 D と長さ N を変えて素朴な実装と SIMD の実装で比べるための問題。
// 各ケースの入力は「N D T seed」の 1 行で、ハーネスが seed から f と m の組を T 個作り (計測の外)、提出の run を T 回呼ぶ。
// 計測は T 回の和、出力は答え全部の hash を 1 つ。
//
// 提出は次を実装する (yosupo-pow-of-formal-power-series-sparse と同じ口)。f は (次数, 係数) の組を次数の昇順に並べたもの
// (f(0) は 0 でなく、次数は n 未満)、0 ≤ m ≤ 10^18。
//   vector<u32> run(int n, u64 m, const vector<pair<int, u32>> &f);
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
  int n, d, t;
  u64 seed;
  must_scan(scanf("%d %d %d %llu", &n, &d, &t, &seed), 4);
  vector<vector<pair<int, u32>>> fs(t, vector<pair<int, u32>>(d + 1));
  vector<u64> ms(t);
  for (int q = 0; q < t; ++q) {
    for (int i = 0; i <= d; ++i) {
      u32 a = 0;
      while (a == 0) a = u32(sm_next(seed) % 998244353);
      fs[q][i] = {i, a};
    }
    const u64 r = sm_next(seed);
    const int s = int(sm_next(seed) % 64);
    ms[q] = (r >> s) % 1000000000000000001ull;  // 0 ≤ m ≤ 10^18 (小さい m が多い)
  }
  vector<vector<u32>> res(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) res[q] = run(n, ms[q], fs[q]);
  auto t1 = chrono::steady_clock::now();
  u64 h = 0;
  for (auto &g : res) {
    h = h * 1000003 + g.size();
    for (u32 x : g) h = h * 0x100000001b3ull + x;
  }
  printf("%llu\n", h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
