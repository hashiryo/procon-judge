// https://judge.yosupo.jp/problem/pow_of_formal_power_series_sparse
//
// 提出は次を実装する。
//   vector<u32> run(int n, u64 m, const vector<pair<int, u32>> &f);
// f は (次数, 係数) の組を次数の昇順に並べた疎な冪級数 (K ≤ 10 項、係数は 0 でない、次数 < n ≤ 10^6、mod 998244353)。
// f^m (0 ≤ m ≤ 10^18) の先頭 n 項を返す。K = 0 (f = 0) もあり、0^0 = 1 とする。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int n, k;
  u64 m;
  must_scan(scanf("%d %d %llu", &n, &k, &m), 3);
  vector<pair<int, u32>> f(k);
  for (auto &[i, a] : f) must_scan(scanf("%d %u", &i, &a), 2);
  auto t0 = chrono::steady_clock::now();
  vector<u32> g = run(n, m, f);
  auto t1 = chrono::steady_clock::now();
  print_all(g, ' ');
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
