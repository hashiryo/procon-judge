// https://judge.yosupo.jp/problem/range_affine_range_sum
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);    // 構築。計測区間の外。
//     void affine(int l, int r, i64 b, i64 c);  // i in [l,r) に a[i] = b a[i] + c
//     i64 sum(int l, int r);                    // (a[l] + ... + a[r-1]) mod 998244353
//   };
//
// 値のやりとりは 998244353 未満の整数で行う。剰余の型はハーネスが決めない。
// ここで ModInt を使うとハーネスがライブラリに依存して、ライブラリを取れない回に
// この問題の提出が全部落ちる。ModInt を直したときに、それを使わない実装まで
// 測り直しになるのも困る。提出の側で選ばせておけばどちらも起きない。
//
// i64 から実装の内部表現への変換はクエリごとに O(1) で、どの実装にも等しく
// 乗るので、比較は歪まない。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);

  // 0 l r b c と 1 l r で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 0) must_scan(scanf("%lld %lld", &e[3], &e[4]), 2);
    else ++gets;
  }

  Solver s(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.affine((int)e[1], (int)e[2], e[3], e[4]);
    else ans.push_back(s.sum((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
