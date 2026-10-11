// https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);         // 列から作る。計測区間の中。
//     pair<int, i64> count_sum(int l, int r, int x);  // a[l, r) のうち x 以下のものの数と和。0 <= l <= r <= N
//   };
//
// 構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。先のクエリを読んでまとめて
// 処理する実装はここでは比べない。N, Q は 5 * 10^5 以下、a_i と x は 0 以上 10^9 以下。設計は algo-notes の
// notes/wavelet-matrix.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/wm_b32.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) must_scan(scanf("%d %d %d", &e[0], &e[1], &e[2]), 3);

  vector<pair<int, i64>> ans(q);

  auto t0 = chrono::steady_clock::now();
  Solver s(a);
  for (int i = 0; i < q; ++i) ans[i] = s.count_sum(qs[i][0], qs[i][1], qs[i][2]);
  auto t1 = chrono::steady_clock::now();

  // 1 行に数と和を並べる。整形は計測区間の外。
  string out;
  out.reserve(size_t(q) * 24);
  for (auto &[c, sum] : ans) {
    out += to_string(c);
    out += ' ';
    out += to_string(sum);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
