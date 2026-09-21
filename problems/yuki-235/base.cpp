// https://yukicoder.me/problems/no/235
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &s, const vector<i64> &c,
//            const vector<array<int, 2>> &edges);  // 宿泊費 s、インフレ係数 c
//     void parade(int x, int y, i64 z);   // 道の上の街 w について s[w] += c[w] * z
//     i64 travel(int x, int y);           // 道の上の街の s の総和 (mod 1e9+7)
//   };
//
// 剰余は 1e9+7 で、入出力は素の整数で渡す。道の上の頂点への作用と総和なので、
// 作用素 (係数の和を持っておいて s に c * z を足す) は common.hpp で 1 つに揃え、
// 木を HLD + 遅延セグメント木で持つか Link-Cut 木で持つかを比べる。
//
// 構築を計測区間に入れてある。HLD は木を列に潰し、Link-Cut 木は N 回の link を
// 積むので、構築そのものが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-segtree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> s = read_ints(n);
  vector<i64> c = read_ints(n);
  vector<array<int, 2>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2), --e[0], --e[1];
  int q;
  must_scan(scanf("%d", &q), 1);

  // 0 X Y Z は 4 項目、1 X Y は 3 項目。種別を読んでから残りを読む。
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 0) must_scan(scanf("%lld", &e[3]), 1);
    else ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, s, c, edges);
  for (auto &e : qs) {
    if (e[0] == 0) sol.parade((int)e[1] - 1, (int)e[2] - 1, e[3]);
    else ans.push_back(sol.travel((int)e[1] - 1, (int)e[2] - 1));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
