// 永続セグメント木 (点更新・区間和) のハーネス。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a, int q);
//     void set(int v, int i, i64 x);  // 版 v から新しい版を作り、a[i] = x にする
//     i64 sum(int v, int l, int r);   // 版 v の a[l] + ... + a[r-1]
//   };
//
// 版 0 は初期の列。type 0 のクエリは、現れる順に版 1, 2, ... を作る。v は 0 以上、その時点の版の数未満。
// 構築と置き場の reserve は計測区間の外に置く。クエリは 1 つずつ渡し、その場で答えさせる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a(n);
  for (auto &x : a) must_scan(scanf("%lld", &x), 1);

  struct Op {
    int type, v;
    long long p, r;
  };
  vector<Op> ops(q);
  for (auto &op : ops) {
    int type;
    long long v, p, r;
    must_scan(scanf("%d %lld %lld %lld", &type, &v, &p, &r), 4);
    op = Op{type, int(v), p, r};
  }

  vector<i64> ans;
  ans.reserve(q);

  Solver s(a, q);

  auto t0 = chrono::steady_clock::now();
  for (auto &op : ops) {
    if (op.type == 0) s.set(op.v, int(op.p), op.r);
    else ans.push_back(s.sum(op.v, int(op.p), int(op.r)));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
