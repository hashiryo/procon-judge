// https://onlinejudge.u-aizu.ac.jp/challenges/sources/UOA/UAPC/3024
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &a, const vector<i64> &b);  // 列を 2 本
//     void set(int k, int i, i64 v);          // k 番の列の i 番目を v にする
//     i64 min_of(int k, int l, int r);        // k 番の列の [l, r) の最小値
//     void assign(int dst, int src);          // dst 番の列を src 番の中身にする
//   };
//
// 入力は 1-indexed だが提出には 0-indexed で渡す。区間は半開にそろえる。
//
// assign が要なので、どの実装も永続化して使う。中身を複製すると O(N) かかり、
// クエリの数だけ積み上がる。構築は配列から組むだけなので計測区間の外に置く。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-dynamic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(n);
  vector<i64> b = read_ints(n);
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 3 || e[0] == 4) ++gets;
  }

  Solver s(a, b);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    int x = (int)e[0], y = (int)e[1], z = (int)e[2];
    if (x <= 2) s.set(x - 1, y - 1, z);
    else if (x <= 4) ans.push_back(s.min_of(x - 3, y - 1, z));
    else s.assign(x - 5, 1 - (x - 5));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
