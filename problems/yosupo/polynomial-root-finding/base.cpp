// https://judge.yosupo.jp/problem/polynomial_root_finding
//
// 提出は次を実装する。
//   vector<u32> run(const vector<u32> &f);
// 次数 N ≤ 4000 の多項式 f = Σ f_i x^i (f_N ≠ 0、mod 998244353) の根を、重複なしで任意の順に返す。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<u32> f(n + 1);
  for (auto &x : f) must_scan(scanf("%u", &x), 1);
  auto t0 = chrono::steady_clock::now();
  vector<u32> roots = run(f);
  auto t1 = chrono::steady_clock::now();
  printf("%d\n", (int)roots.size());
  if (roots.empty()) puts("");
  print_all(roots, ' ');
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
