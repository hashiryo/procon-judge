// https://codeforces.com/contest/1609/problem/E
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);  // s は 'a'、'b'、'c' の列。計測区間の中
//     int update(int pos, char c);       // s[pos] (0 始まり) を c にしてから、答えを返す
//   };
//
// 解き方の型は、"abc" の何文字目まで部分列として現れたかを状態にした min-plus の 3 × 3 の行列を、点更新のセグメント木に
// 載せて全体の積を読むもの (動的 DP)。行列を載せる型の持ち方と、半環の表し方を比べる。構築とクエリを計測区間に入れる。
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
  vector<int> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  Solver sol(s);
  for (auto [x, c] : qs) ans.push_back(sol.update(x, c));
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
