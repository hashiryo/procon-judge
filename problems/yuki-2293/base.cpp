// https://yukicoder.me/problems/no/2293
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);    // 構築。計測区間の外。
//     i64 same(int u, int v);    // x_u = x_v を足して、今の解の個数 (mod 998244353) を返す
//     i64 differ(int u, int v);  // x_u != x_v を足して、同じく返す
//     i64 reset();               // 制約を全部消して、同じく返す
//   };
//
// 解の個数は、矛盾が無ければ 2^(成分数)、あれば 0。リセットのたびに作り直すと
// O(N) がリセットの回数ぶん積み上がるので、巻き戻せる Union-Find で戻す。
// 頂点を 2N 個に倍化する手と、xor を重みに持つ手を比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find-undoable.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);

  // 1 u v と 2 u v と 3 で項目数が違うので、種別を読んでから残りを読む。
  vector<array<int, 3>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%d", &e[0]), 1);
    if (e[0] != 3) {
      must_scan(scanf("%d %d", &e[1], &e[2]), 2);
      --e[1], --e[2];
    }
  }

  Solver s(n);
  vector<i64> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 1) ans.push_back(s.same(e[1], e[2]));
    else if (e[0] == 2) ans.push_back(s.differ(e[1], e[2]));
    else ans.push_back(s.reset());
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
