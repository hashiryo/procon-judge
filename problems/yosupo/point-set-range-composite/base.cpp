// https://judge.yosupo.jp/problem/point_set_range_composite
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<i64, 2>> &f);  // 構築。計測区間の外。f[i] = {a, b} は x -> a x + b
//     void set(int p, i64 c, i64 d);                     // f[p] を x -> c x + d に置き換える
//     i64 query(int l, int r, i64 x);                    // f[r-1](...f[l](x)...) mod 998244353
//   };
//
// 値は 998244353 未満の整数でやりとりする。剰余の型を決めないのは range-affine-range-sum と
// 同じ理由で、ハーネスがライブラリに依存しないようにするため。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 2>> f(n);
  for (auto &e : f) must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);

  // 0 p c d と 1 l r x はどちらも 4 項目。
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3]), 4);
    gets += e[0] == 1;
  }

  Solver s(f);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.set((int)e[1], e[2], e[3]);
    else ans.push_back(s.query((int)e[1], (int)e[2], e[3]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
