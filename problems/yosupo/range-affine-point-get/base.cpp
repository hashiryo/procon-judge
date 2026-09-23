// https://judge.yosupo.jp/problem/range_affine_point_get
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);    // 構築。計測区間の外。
//     void affine(int l, int r, i64 b, i64 c);  // i in [l,r) に a[i] = b a[i] + c
//     i64 get(int i);                           // a[i] mod 998244353
//   };
//
// 剰余の型を決めないのは range-affine-range-sum と同じ理由。値は 998244353
// 未満の整数でやりとりする。
//
// 区間の作用と 1 点の取得だけなので、作用素は双対だけあればよく、値の側の
// モノイドは要らない。載せるコンテナにとっては range-affine-range-sum より
// 軽い仕事になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);

  // 0 l r b c と 1 i で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4]), 4);
    else must_scan(scanf("%lld", &e[1]), 1), ++gets;
  }

  Solver s(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.affine((int)e[1], (int)e[2], e[3], e[4]);
    else ans.push_back(s.get((int)e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
