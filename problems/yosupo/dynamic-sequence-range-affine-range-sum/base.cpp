// https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);   // 構築。計測区間の外。
//     void insert(int i, i64 x);               // 位置 i に x を挿む
//     void erase(int i);                       // 位置 i を取り除く
//     void reverse(int l, int r);              // [l, r) を反転
//     void affine(int l, int r, i64 b, i64 c); // a[i] = b a[i] + c
//     i64 sum(int l, int r);                   // (a[l] + ... + a[r-1]) mod 998244353
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。列の長さが変わるので、
// range-affine-range-sum と違って平衡二分探索木しか載らない。
//
// 構築はどれも配列から O(N) で組むので、計測区間の外に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-wbt.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);

  // 種別ごとに項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
    else if (e[0] == 1) must_scan(scanf("%lld", &e[1]), 1);
    else if (e[0] == 3)
      must_scan(scanf("%lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4]), 4);
    else must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
    if (e[0] == 4) ++gets;
  }

  Solver s(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    int l = (int)e[1], r = (int)e[2];
    if (e[0] == 0) s.insert(l, e[2]);
    else if (e[0] == 1) s.erase(l);
    else if (e[0] == 2) s.reverse(l, r);
    else if (e[0] == 3) s.affine(l, r, e[3], e[4]);
    else ans.push_back(s.sum(l, r));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
