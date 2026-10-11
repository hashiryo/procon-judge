// https://atcoder.jp/contests/abc339/tasks/abc339_g
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);   // 列から作る。計測区間の中。
//     i64 sum_le(int l, int r, int x);         // a[l, r) のうち x 以下のものの和
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つ前の答えとの xor で暗号にしてあり、ハーネスが答えを受け取って
// から次のクエリを戻して渡すので、オンラインでしか答えられない。入力の L と R は 1 始まりの閉区間なので、
// [L - 1, R) にして渡す。N, Q は 2 * 10^5 以下、A_i と X は 0 以上 10^9 以下。設計は algo-notes の
// notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-seg2d.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<i64, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);

  vector<i64> ans(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(a);
  i64 b = 0;
  for (int i = 0; i < q; ++i) {
    const int l = int(qs[i][0] ^ b), r = int(qs[i][1] ^ b), x = int(qs[i][2] ^ b);
    ans[i] = b = s.sum_le(l - 1, r, x);
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
