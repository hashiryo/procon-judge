// https://judge.yosupo.jp/problem/exp_of_formal_power_series_sparse
//
// 提出は次を実装する。
//   vector<u32> run(int n, const vector<pair<int, u32>> &f);
// f は (次数, 係数) の組を次数の昇順に並べた疎な冪級数 (K ≤ 10 項、係数は 0 でない、次数 < n ≤ 10^6、mod 998244353)。
// exp(f) の先頭 n 項を返す。f の定数項は 0 (i_0 > 0)。K = 0 もある。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int n, k;
  must_scan(scanf("%d %d", &n, &k), 2);
  vector<pair<int, u32>> f(k);
  for (auto &[i, a] : f) must_scan(scanf("%d %u", &i, &a), 2);
  auto t0 = chrono::steady_clock::now();
  vector<u32> g = run(n, f);
  auto t1 = chrono::steady_clock::now();
  if (g.empty()) puts("-1");
  else print_all(g, ' ');
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
