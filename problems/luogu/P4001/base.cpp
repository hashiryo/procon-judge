// https://www.luogu.com.cn/problem/P4001
//
// 提出は次を実装する。
//   struct Solver {
//     // h[i * (m - 1) + j] は (i, j)-(i, j + 1)、v[i * m + j] は (i, j)-(i + 1, j)、d[i * (m - 1) + j] は
//     // (i, j)-(i + 1, j + 1) の無向の道の重み (頂点は 0 始まり)
//     Solver(int n, int m, const vector<int> &h, const vector<int> &v, const vector<int> &d);
//     void run();
//     long long answer() const;  // 左上 (0, 0) と右下 (n - 1, m - 1) を分ける道の重みの和の最小
//   };
//
// 3 ≤ N, M ≤ 1000、重みは 1 以上 10^6 以下。想定解は平面グラフの双対の最短路で、この問題を置いたのは、頂点 10^6、無向の辺
// 3 × 10^6 の格子を最大流で解く力ずくの時間とメモリを測るため (algo-notes の network-flow-brute-force.md)。構築と run を測る。
// h、v、d は計測区間のあとまで生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  auto read = [](size_t k) {
    vector<int> a(k);
    for (auto &x : a) must_scan(scanf("%d", &x), 1);
    return a;
  };
  const vector<int> h = read((size_t)n * (m - 1));
  const vector<int> v = read((size_t)(n - 1) * m);
  const vector<int> d = read((size_t)(n - 1) * (m - 1));

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, m, h, v, d);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
