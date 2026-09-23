// https://yukicoder.me/problems/no/1625
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<array<i64, 3>> &tri,      // {x の最小, x の最大, 面積の 2 倍}
//            const vector<array<i64, 4>> &qs);      // {type, l, r, 面積の 2 倍 (追加のとき)}
//     void run();                                   // ここだけ測る。
//     const vector<i64> &answer() const;            // 質問ごとの答え。無ければ -1
//   };
//
// 三角形を (x の最小, x の最大) の点にすると、[l, r] に収まる三角形は x, y とも
// [l, r] にある点になる。三角形から点と面積を作るのは入力の解析なのでハーネスで
// 済ませる。追加される点も先に集めて構造を固めるので、1 回の計算の形で渡す。
// 比べたいのは 2 次元の点集合に対する 1 点更新と長方形の最大を取る構造で、kd 木と
// 2 次元セグメント木。
#include <algorithm>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-2d.hpp"
#endif
#include SUBMISSION_HPP

// 3 点から {x の最小, x の最大, 面積の 2 倍}。
static array<i64, 3> triangle(i64 a, i64 b, i64 c, i64 d, i64 e, i64 f) {
  i64 l = std::min({a, c, e}), r = std::max({a, c, e});
  i64 s = (c - a) * (f - b) - (d - b) * (e - a);
  return {l, r, s < 0 ? -s : s};
}

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 3>> tri(n);
  for (auto &t : tri) {
    i64 v[6];
    must_scan(scanf("%lld %lld %lld %lld %lld %lld", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]), 6);
    t = triangle(v[0], v[1], v[2], v[3], v[4], v[5]);
  }
  // 1 a b c d e f は 7 項目、2 l r は 3 項目。種別を読んでから残りを読む。
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 1) {
      i64 v[6];
      must_scan(scanf("%lld %lld %lld %lld %lld %lld", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]), 6);
      auto t = triangle(v[0], v[1], v[2], v[3], v[4], v[5]);
      e[1] = t[0], e[2] = t[1], e[3] = t[2];
    } else {
      must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
      e[3] = 0, ++gets;
    }
  }

  Solver sol(tri, qs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
