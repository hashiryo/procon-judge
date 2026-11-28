// https://judge.yosupo.jp/problem/vertex_get_range_contour_add_on_tree
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。queries は Q 個のクエリ。
//     Solver(int n, const vector<i64> &a, const vector<pair<int, int>> &edges,
//            const vector<array<i64, 5>> &queries);
//     void run();                          // ここだけ測る。
//     const vector<i64> &answers() const;  // タイプ 1 の答えを聞かれた順に
//   };
//
// queries[i] は e[0] が種別で、続きは入力の順に詰める。使わない項は 0。
//   0 p l r x -> e[1] = p, e[2] = l, e[3] = r, e[4] = x (距離が [l, r) の頂点に x を足す)
//   1 p       -> e[1] = p (a_p を出す)
// edges は n - 1 本の辺。木は頂点 0 を根とする。
//
// クエリ列を先に全部読んでから渡すので、重心分解などの構築は計測区間の外に置ける。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  vector<pair<int, int>> edges(size_t(n > 1 ? n - 1 : 0));
  for (auto &[u, v] : edges) must_scan(scanf("%d %d", &u, &v), 2);

  // 0 p l r x と 1 p で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  for (auto &e : qs) {
    e = {0, 0, 0, 0, 0};
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0)
      must_scan(scanf("%lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4]), 4);
    else
      must_scan(scanf("%lld", &e[1]), 1);
  }

  Solver sol(n, a, edges, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
