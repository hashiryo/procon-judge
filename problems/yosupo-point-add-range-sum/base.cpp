// https://judge.yosupo.jp/problem/point_add_range_sum
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void add(int p, i64 x);                 // a[p] += x
//     i64 sum(int l, int r);                  // a[l] + ... + a[r-1]
//   };
//
// 計測区間にはクエリの処理だけを残す。入力の解析、Solver の構築、答えの整形は
// すべて外に出してある。構築は O(N) で、クエリ列 O(Q log N) に対して無視できる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a(n);
  for (auto &x : a) must_scan(scanf("%lld", &x), 1);
  vector<array<i64, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  Solver s(a);
  vector<i64> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.add((int)e[1], e[2]);
    else ans.push_back(s.sum((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(ans.size() * 20);
  for (i64 v : ans) {
    out += to_string(v);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
