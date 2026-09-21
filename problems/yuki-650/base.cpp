// https://yukicoder.me/problems/no/650
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 辺 i は edges[i]。行列は単位行列で始まる
//     void set(int e, const array<i64, 4> &x);            // 辺 e の行列を (x0 x1; x2 x3) にする
//     array<i64, 4> product(int u, int v);                // u (先祖) から v への道の辺の行列の積。根側が左
//   };
//
// 剰余は 1e9+7 で、入出力は素の整数で渡す。積は可換でないので、根側を左にして
// 掛ける。根は 0。辺に載った値を扱うので、HLD は子側の頂点に持たせ、Link-Cut 木は
// 辺を頂点にして間に挟む。
//
// 構築を計測区間に入れてある。HLD は木を列に潰し、Link-Cut 木は 2(N-1) 回の link
// を積むので、構築そのものが比較の対象になる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-hld-segtree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<array<int, 2>> edges(n - 1);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);
  int q;
  must_scan(scanf("%d", &q), 1);

  // x i a b c d は 6 項目、g i j は 3 項目。種別を読んでから残りを読む。
  struct Query {
    bool get;
    array<i64, 6> v;
  };
  vector<Query> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    string t = read_token();
    e.get = t == "g";
    if (e.get) must_scan(scanf("%lld %lld", &e.v[0], &e.v[1]), 2), ++gets;
    else must_scan(scanf("%lld %lld %lld %lld %lld", &e.v[0], &e.v[1], &e.v[2], &e.v[3], &e.v[4]), 5);
  }

  vector<array<i64, 4>> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, edges);
  for (auto &e : qs) {
    if (e.get) ans.push_back(s.product((int)e.v[0], (int)e.v[1]));
    else s.set((int)e.v[0], {e.v[1], e.v[2], e.v[3], e.v[4]});
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(ans.size() * 44);
  for (auto &a : ans) {
    for (int i = 0; i < 4; ++i) {
      out += to_string(a[i]);
      out += i == 3 ? '\n' : ' ';
    }
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
