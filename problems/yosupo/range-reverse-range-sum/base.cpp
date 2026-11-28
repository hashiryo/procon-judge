// https://judge.yosupo.jp/problem/range_reverse_range_sum
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void reverse(int l, int r);             // a[l], ..., a[r-1] を反転する。
//     i64 sum(int l, int r);                  // a[l] + ... + a[r-1]
//   };
//
// 列の長さは変わらないが、区間の反転があるので、位置で二分する木を載せる。
// 和は剰余を取らない素の値で、最悪 N * A_MAX = 2 * 10^14 なので 64 bit に収まる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);

  // 種別は t の 2 種類だけで、どのクエリも t l r の 3 項目。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 1) ++gets;
  }

  Solver sol(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (const auto &e : qs) {
    const int l = (int)e[1], r = (int)e[2];
    if (e[0] == 0) sol.reverse(l, r);
    else ans.push_back(sol.sum(l, r));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
