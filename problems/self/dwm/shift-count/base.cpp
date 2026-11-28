// 区間の巡回シフトと値の個数 (自作)。Codeforces 455D を元に、位置を 0-indexed の半開区間に直したもの。
//
// 入力:
//   N Q
//   A_0 ... A_{N-1}
//   クエリ (Q 行):
//     0 l r    : [l, r) を右に 1 つ巡回シフト (0 <= l < r <= N)
//     1 l r k  : [l, r) の k の個数 (0 <= l < r <= N)
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);
//     void shift(int l, int r);          // [l, r) を右に 1 つ巡回シフト
//     int count(int l, int r, int k) const;
//   };
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
    if (e[0] == 0) must_scan(scanf("%d %d", &e[1], &e[2]), 2);
    else must_scan(scanf("%d %d %d", &e[1], &e[2], &e[3]), 3);
  }

  Solver s(a);
  vector<int> ans;

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.shift(e[1], e[2]);
    else ans.push_back(s.count(e[1], e[2], e[3]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
