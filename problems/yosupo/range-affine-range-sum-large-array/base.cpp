// https://judge.yosupo.jp/problem/range_affine_range_sum_large_array
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);                    // a[i] = 0 で埋めた長さ n の列。計測区間の中。
//     void affine(int l, int r, i64 b, i64 c);   // i in [l, r) に a[i] = b a[i] + c
//     i64 sum(int l, int r);                     // (a[l] + ... + a[r-1]) mod 998244353
//   };
//
// N は 10^9 以下、Q は 10^5 以下。添字の空間が大きいので、触った所だけ節を作る木などで持つ。
// クエリは 1 つずつ渡し、その場で答えさせる。Library Checker の首位は、先にクエリを全部読んで区間の端を
// 座標圧縮するオフラインの解き方だが、ライブラリとして使う形ではないのでここでは比べない
// (associative-array と同じ考え)。クエリの数も構築に渡さない。節の置き場の伸ばし方の差を測りに乗せるため。
//
// 値は 998244353 未満の整数でやりとりする。剰余の型を決めないのは range-affine-range-sum と同じ理由で、
// ハーネスがライブラリに依存しないようにするため。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-dynamic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);

  // 0 l r b c と 1 l r で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 0) must_scan(scanf("%lld %lld", &e[3], &e[4]), 2);
    else ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n);
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
