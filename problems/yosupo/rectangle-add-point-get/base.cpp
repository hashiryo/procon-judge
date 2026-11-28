// https://judge.yosupo.jp/problem/rectangle_add_point_get
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。
//     Solver(const vector<array<i64, 5>> &rect,   // 初期の N 個の長方形 {l, d, r, u, w}
//            const vector<array<i64, 6>> &query);  // Q 個のクエリ
//     void run();                              // ここだけ測る。
//     const vector<i64> &answers() const;      // タイプ 1 のクエリの答え
//   };
//
// query[i] は {0, l, d, r, u, w} (長方形の追加) か {1, x, y, 0, 0, 0} (点の取得)。
// どちらのクエリも先に全部読めるので、Solver にはまとめて渡す。時間順はベクタの
// 順そのままで、初期の長方形はすべてクエリより先にある。
//
// 圧縮も木の構築も実装の側の仕事なので、構築を計測区間の外に出さず run() で測る
// 提出にしてよい。このハーネスは構築を外に置くだけで、中で何をするかは決めない。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 5>> rect(n);
  for (auto &e : rect)
    must_scan(scanf("%lld %lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3], &e[4]),
               5);
  vector<array<i64, 6>> query(q);
  for (auto &e : query) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0)
      must_scan(
          scanf("%lld %lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4], &e[5]), 5);
    else
      must_scan(scanf("%lld %lld", &e[1], &e[2]), 2);
  }

  Solver sol(rect, query);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answers());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
