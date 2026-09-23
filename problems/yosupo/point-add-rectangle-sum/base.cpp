// https://judge.yosupo.jp/problem/point_add_rectangle_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<array<i64, 3>> &points,   // 最初の N 点 {x, y, w}
//            const vector<array<int, 2>> &spots);   // 後から足される点の座標
//     void add(int x, int y, i64 w);
//     i64 rect_sum(int l, int d, int r, int u);     // [l, r) x [d, u)
//   };
//
// このハーネスだけは、これから足される点の座標を構築へ渡す。座標の集合を先に
// 固定しないと載らない実装しか無いためで、オンラインの実装は相手にしていない。
// 他の問題では先の入力を渡さないようにしているので、ここは例外になる。
//
// 構築は計測区間に入れる。座標の表を作って木を組むところが実装で違う。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-2d.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 3>> points(n);
  for (auto &p : points) must_scan(scanf("%lld %lld %lld", &p[0], &p[1], &p[2]), 3);

  // 0 x y w は 4 項目、1 l d r u は 5 項目。
  vector<array<i64, 5>> qs(q);
  vector<array<int, 2>> spots;
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0) {
      must_scan(scanf("%lld %lld %lld", &e[1], &e[2], &e[3]), 3);
      spots.push_back({(int)e[1], (int)e[2]});
    } else {
      must_scan(scanf("%lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4]), 4);
      ++gets;
    }
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(points, spots);
  for (auto &e : qs) {
    if (e[0] == 0) s.add((int)e[1], (int)e[2], e[3]);
    else ans.push_back(s.rect_sum((int)e[1], (int)e[2], (int)e[3], (int)e[4]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
