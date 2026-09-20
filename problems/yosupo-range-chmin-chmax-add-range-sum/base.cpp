// https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void chmin(int l, int r, i64 b);        // a[i] = min(a[i], b)
//     void chmax(int l, int r, i64 b);        // a[i] = max(a[i], b)
//     void add(int l, int r, i64 b);          // a[i] += b
//     i64 sum(int l, int r);                  // a[l] + ... + a[r-1]
//   };
//
// 構築はどちらの実装も配列から O(N) で組むので、計測区間の外に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-beats.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);

  // 0 l r b / 1 l r b / 2 l r b は 4 項目、3 l r は 3 項目。
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 3) ++gets;
    else must_scan(scanf("%lld", &e[3]), 1);
  }

  Solver s(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    int l = (int)e[1], r = (int)e[2];
    if (e[0] == 0) s.chmin(l, r, e[3]);
    else if (e[0] == 1) s.chmax(l, r, e[3]);
    else if (e[0] == 2) s.add(l, r, e[3]);
    else ans.push_back(s.sum(l, r));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
