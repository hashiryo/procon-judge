// 点更新と区間の大きい方から和が k 以上になる最小個数 (自作)。AtCoder ABC 467 G を元に、制約を小さくしたもの。
//
// 入力:
//   N Q
//   A_0 ... A_{N-1}
//   クエリ (Q 行): c x l r k  (1-indexed)
//     A_c を x に更新してから、[l, r] のお菓子を大きい方から選んで和が k 以上になる最小個数を出す。
//     どのように選んでも和が k 以上にならないときは -1。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);
//     void set(int c, int x);                 // 0-indexed
//     int query(int l, int r, long long k) const;  // [l, r) 0-indexed、-1 あり
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

  vector<array<long long, 5>> qs(q);
  for (auto &e : qs)
    must_scan(scanf("%lld %lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3], &e[4]), 5);

  Solver s(a);
  vector<int> ans;

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    s.set((int)e[0] - 1, (int)e[1]);
    ans.push_back(s.query((int)e[2] - 1, (int)e[3], e[4]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
