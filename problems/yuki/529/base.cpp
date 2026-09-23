// https://yukicoder.me/problems/no/529
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 街と道。二重辺連結成分への縮約を含む
//     void add(int u, i64 w);       // 街 u に価値 w の獲物が出る
//     i64 take(int s, int t);       // s から t へ帰省する途中で捕まえられる最大の価値。
//                                   // 無ければ -1。捕まえた獲物は消える
//   };
//
// 同じ道を 2 度使わずに行ける範囲は二重辺連結成分の中なら自由なので、成分に
// 縮約して橋だけの木にすると、木の上の道の最大を取って消す問題になる。縮約と
// 成分ごとの獲物の持ち方は common.hpp に置き、木の上の道の最大を HLD + セグメント
// 木で取るか Link-Cut 木で取るかを比べる。
//
// 構築を計測区間に入れてある。HLD は木を列に潰し、Link-Cut 木は link を積むので、
// 構築そのものが比較の対象になる。縮約はどちらも同じぶんだけ入る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-segtree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, q;
  must_scan(scanf("%d %d %d", &n, &m, &q), 3);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2), --e[0], --e[1];

  // 1 U W も 2 S T も 3 項目なので、まとめて読む。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 2) ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, edges);
  for (auto &e : qs) {
    if (e[0] == 1) s.add((int)e[1] - 1, e[2]);
    else ans.push_back(s.take((int)e[1] - 1, (int)e[2] - 1));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
