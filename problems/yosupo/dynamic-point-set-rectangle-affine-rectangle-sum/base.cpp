// https://judge.yosupo.jp/problem/dynamic_point_set_rectangle_affine_rectangle_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<array<i64, 3>> &points,    // 最初の N 点 {x, y, w}
//            const vector<array<i64, 2>> &spots,     // タイプ 0 で後から足す点の座標 (出現順)
//            const vector<array<i64, 7>> &queries);  // Q 個のクエリ
//     void run();                                    // ここだけ測る。
//     const vector<i64> &answers() const;            // タイプ 2 の答えを順に
//   };
//
// 後から足す点の座標を構築に渡すのは、座標を先に固定しないと木に載らない実装があるため
// (yosupo-point-add-rectangle-sum と同じ)。点の番号は、最初の N 点が 0..N-1、タイプ 0 の
// 点が現れた順に N, N+1, ... とする。
//
// queries[i] は e[0] が種別で、続きは入力の順に詰める。使わない項は 0。
//   0 x y w      -> e[1] = x, e[2] = y, e[3] = w
//   1 x w        -> e[1] = x, e[2] = w
//   2 l d r u    -> e[1] = l, e[2] = d, e[3] = r, e[4] = u
//   3 l d r u a b -> e[1] = l, e[2] = d, e[3] = r, e[4] = u, e[5] = a, e[6] = b
// 座標と重みは i64 で渡す。剰余は 998244353。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 3>> points(n);
  for (auto &p : points) must_scan(scanf("%lld %lld %lld", &p[0], &p[1], &p[2]), 3);

  // 種別ごとに項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 7>> queries(q);
  vector<array<i64, 2>> spots;
  for (auto &e : queries) {
    e.fill(0);
    must_scan(scanf("%lld", &e[0]), 1);
    const int kind = (int)e[0];
    if (kind == 0) {
      must_scan(scanf("%lld %lld %lld", &e[1], &e[2], &e[3]), 3);
      spots.push_back({e[1], e[2]});
    } else if (kind == 1) {
      must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
    } else if (kind == 2) {
      must_scan(scanf("%lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4]), 4);
    } else {
      must_scan(scanf("%lld %lld %lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4], &e[5], &e[6]), 6);
    }
  }

  Solver sol(points, spots, queries);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
