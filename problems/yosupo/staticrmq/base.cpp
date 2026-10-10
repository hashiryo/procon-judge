// https://judge.yosupo.jp/problem/staticrmq
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<u32> &a);  // 前処理。計測区間の中。
//     u32 query(int l, int r);                // min(a[l], ..., a[r - 1])。0 <= l < r <= N
//   };
//
// 前処理とクエリの両方を計測区間に入れる。比べたいのは前処理の重さとクエリの速さの兼ね合いで、前処理を外に出すと
// 表を大きく持つ実装ほど得をする。クエリは 1 つずつ渡し、その場で答えさせる。LCA や LCP 配列への問い合わせで
// 使うのもこの形なので、先のクエリを読んでまとめて答える実装はここでは比べない。値は 10^9 以下なので u32 で渡す。
// 設計は algo-notes の notes/rmq.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-sparse-table.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<u32> a(n);
  for (auto &x : a) must_scan(scanf("%u", &x), 1);
  vector<array<int, 2>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  vector<u32> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(a);
  for (auto &e : qs) ans.push_back(s.query(e[0], e[1]));
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
