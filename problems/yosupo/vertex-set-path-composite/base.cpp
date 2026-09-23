// https://judge.yosupo.jp/problem/vertex_set_path_composite
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 2>> &f,
//            const vector<array<int, 2>> &edges);  // f[i] = {a_i, b_i}
//     void set(int p, i64 c, i64 d);               // f[p] を cx + d にする
//     i64 composite(int u, int v, i64 x);          // u から v の順に適用した値
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。合成は順序を持つので、u から v
// への向きで適用する。辺は無向で、根は提出が決める。
//
// 構築を計測区間に入れてある。HLD は木を列に潰して向きの違う 2 本の木を持ち、
// Link-Cut 木は N 回の link を積むので、構築そのものが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-segtree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 2>> f(n);
  for (auto &e : f) must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
  vector<array<int, 2>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  // 0 p c d も 1 u v x も 4 項目なので、まとめて読む。
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3]), 4);
    if (e[0]) ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, f, edges);
  for (auto &e : qs) {
    if (e[0] == 0) s.set((int)e[1], e[2], e[3]);
    else ans.push_back(s.composite((int)e[1], (int)e[2], e[3]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
