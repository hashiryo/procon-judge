// 列への追加・削除・変更と区間 k 番目 (自作)。CodeChef ARRQRS を元に、位置を 0-indexed に直したもの。
//
// 入力:
//   Q
//   クエリ (Q 行):
//     0 val    : 末尾に val を追加
//     1 x      : 位置 x を削除 (0 <= x < 現在の長さ)
//     2 x val  : 位置 x の値を val に変更 (0 <= x < 現在の長さ)
//     3 x y k  : [x, y) の k 番目に小さい値 (0 <= x < y <= 現在の長さ, 0 <= k < y-x)
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver();                       // 空の列
//     void push_back(int val);
//     void erase(int x);
//     void set(int x, int val);
//     int kth(int x, int y, int k) const;
//   };
// クエリは計測区間の中で 1 つずつ渡すので、先のクエリを読んで並べ替える実装は書けない。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 4>> qs(q);
  for (auto &e : qs) {
    e = {0, 0, 0, 0};
    must_scan(scanf("%d", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%d", &e[1]), 1);
    else if (e[0] == 1) must_scan(scanf("%d", &e[1]), 1);
    else if (e[0] == 2) must_scan(scanf("%d %d", &e[1], &e[2]), 2);
    else must_scan(scanf("%d %d %d", &e[1], &e[2], &e[3]), 3);
  }

  Solver s;
  vector<int> ans;

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.push_back(e[1]);
    else if (e[0] == 1) s.erase(e[1]);
    else if (e[0] == 2) s.set(e[1], e[2]);
    else ans.push_back(s.kth(e[1], e[2], e[3]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
