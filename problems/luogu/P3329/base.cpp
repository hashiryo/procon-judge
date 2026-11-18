// 洛谷 P3329 [ZJOI2011] 最小割。T 個の無向グラフ (n ≤ 150、m ≤ 3000、多重辺あり、容量 ≤ 10^6) のそれぞれで、最小カットの
// 値が x 以下の頂点の組の数を q 回 (q ≤ 30) 答える。答えはグラフごとに q 行で、グラフの間に空行を挟む。
//
// 提出は self-flow-gomory-hu と同じものを実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges);  // 無向辺 {u, v, 容量}。頂点は 0 から n - 1
//     void run();                                        // Gomory-Hu 木を作る
//     const vector<array<i64, 3>> &tree() const;         // 木の辺 {u, v, 重み} を n - 1 本
//   };
// グラフごとに、入力を読んだあとの構築と run を測り、T 個の和を出す。組を数えるのは計測区間の外で、木の辺を重みの大きい
// 順に Union-Find でつなぎ、x より大きい辺でつながる組を全体から引く。
#include "pj.hpp"
#include <algorithm>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int T;
  must_scan(scanf("%d", &T), 1);
  long long total_ns = 0;
  string out;
  for (int tc = 0; tc < T; ++tc) {
    int n, m;
    must_scan(scanf("%d %d", &n, &m), 2);
    vector<array<i64, 3>> edges(m);
    for (auto &e : edges) {
      must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
      --e[0], --e[1];
    }
    auto t0 = chrono::steady_clock::now();
    Solver sol(n, edges);
    sol.run();
    auto t1 = chrono::steady_clock::now();
    total_ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    vector<array<i64, 3>> es(sol.tree());
    sort(es.begin(), es.end(), [](auto &a, auto &b) { return a[2] > b[2]; });
    int q;
    must_scan(scanf("%d", &q), 1);
    if (tc > 0) out += '\n';
    for (int i = 0; i < q; ++i) {
      long long x;
      must_scan(scanf("%lld", &x), 1);
      vector<int> uf(n, -1);
      auto find = [&](int v) {
        while (uf[v] >= 0) v = uf[v];
        return v;
      };
      long long above = 0;  // λ > x の組の数
      for (auto &[u, v, w] : es) {
        if (w <= x) break;
        int a = find((int)u), b = find((int)v);
        above += (long long)uf[a] * uf[b];
        if (uf[a] > uf[b]) swap(a, b);
        uf[a] += uf[b], uf[b] = a;
      }
      out += to_string((long long)n * (n - 1) / 2 - above), out += '\n';
    }
  }
  fputs(out.c_str(), stdout);
  report_metrics(total_ns);
}
