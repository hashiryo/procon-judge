// https://judge.yosupo.jp/problem/persistent_range_affine_range_sum
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a, int q);
//     void apply(int i, int k, int l, int r, int b, int c);  // A_i を A_k の写しにして [l,r) を x -> b x + c
//     void copy(int i, int k, int s, int l, int r);           // A_i を A_k の写しにして [l,r) を A_s から写す
//     int sum(int k, int l, int r) const;                     // A_k の [l,r) の和
//   };
//
// k, s = -1 は初期の列 A_{-1}。i は 0 始まりのクエリ番号で、type 0/1 のクエリが A_i を作る。
// 構築と reserve は計測区間の外に置く。クエリは 1 つずつ渡し、その場で答えさせる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);

  vector<int> ans;
  ans.reserve(q);

  Solver s(a, q);

  auto t0 = chrono::steady_clock::now();
  for (int i = 0; i < q; ++i) {
    int type;
    must_scan(scanf("%d", &type), 1);
    if (type == 0) {
      int k, l, r, b, c;
      must_scan(scanf("%d %d %d %d %d", &k, &l, &r, &b, &c), 5);
      s.apply(i, k, l, r, b, c);
    } else if (type == 1) {
      int k, ss, l, r;
      must_scan(scanf("%d %d %d %d", &k, &ss, &l, &r), 4);
      s.copy(i, k, ss, l, r);
    } else {
      int k, l, r;
      must_scan(scanf("%d %d %d", &k, &l, &r), 3);
      ans.push_back(s.sum(k, l, r));
    }
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
