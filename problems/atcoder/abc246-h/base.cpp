// https://atcoder.jp/contests/abc246/tasks/abc246_h
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);  // s は '0'、'1'、'?' の列。計測区間の中
//     u32 update(int x, char c);         // s[x] (0 始まり) を c にしてから、答え (mod 998244353) を返す
//   };
//
// 解き方の型は、文字ごとの 3 × 3 の行列 (0 で終わる部分列の数、1 で終わる部分列の数、1 の組に掛かるもの) を
// セグメント木に載せ、点更新のたびに全体の積を読むもの。行列を載せる型の持ち方 (大きさを型で決めるか、
// 実行時に決めるか) と、積の還元の仕方を比べる。構築とクエリを計測区間に入れる。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  string s = read_token();
  vector<pair<int, char>> qs(q);
  for (auto &[x, c] : qs) {
    must_scan(scanf("%d", &x), 1);
    string t = read_token();
    --x, c = t[0];
  }
  vector<u32> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  Solver sol(s);
  for (auto [x, c] : qs) ans.push_back(sol.update(x, c));
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
