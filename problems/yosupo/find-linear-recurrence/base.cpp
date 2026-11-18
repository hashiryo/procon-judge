// https://judge.yosupo.jp/problem/find_linear_recurrence
//
// 提出は次を実装する。
//   vector<u32> run(const vector<u32> &a);
// a_i = Σ_{j=1}^{d} c_j a_{i-j} (mod 998244353, d ≤ i < N) を満たす最短の c_1, ..., c_d を返す。
// 0 ≤ N ≤ 10^4。最短の答えが複数あればどれでもよい (チェッカで判定する)。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<u32> a(n);
  for (auto &x : a) must_scan(scanf("%u", &x), 1);
  auto t0 = chrono::steady_clock::now();
  vector<u32> c = run(a);
  auto t1 = chrono::steady_clock::now();
  printf("%d\n", (int)c.size());
  if (c.empty()) puts("");
  print_all(c, ' ');
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
