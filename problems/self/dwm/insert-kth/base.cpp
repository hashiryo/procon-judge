// 列への挿入・値の変更と区間 k 番目 (自作)。Luogu P4278 (BZOJ 3065) を元に、位置を 0-indexed に直したもの。
//
// 入力:
//   N Q
//   A_0 ... A_{N-1}
//   クエリ (Q 行):
//     0 x val  : 位置 x の前に val を挿入 (0 <= x <= 現在の長さ)
//     1 x val  : 位置 x の値を val に変更 (0 <= x < 現在の長さ)
//     2 x y k  : [x, y) の k 番目に小さい値 (0 <= x < y <= 現在の長さ, 0 <= k < y-x)
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);  // 構築。計測区間の外。a だけを受け取る。
//     void insert(int x, int val);             // 位置 x の前に挿入
//     void set(int x, int val);                // 位置 x を val に変更
//     int kth(int x, int y, int k) const;      // [x, y) の k 番目 (0-indexed)
//   };
// クエリは計測区間の中で 1 つずつ渡すので、先のクエリを読んで並べ替える実装は書けない。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);

  vector<array<int, 4>> qs(q);
  for (auto &e : qs) {
    e = {0, 0, 0, 0};
    must_scan(scanf("%d", &e[0]), 1);
    if (e[0] == 0 || e[0] == 1) must_scan(scanf("%d %d", &e[1], &e[2]), 2);
    else must_scan(scanf("%d %d %d", &e[1], &e[2], &e[3]), 3);
  }

  Solver s(a);
  vector<int> ans;

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.insert(e[1], e[2]);
    else if (e[0] == 1) s.set(e[1], e[2]);
    else ans.push_back(s.kth(e[1], e[2], e[3]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
