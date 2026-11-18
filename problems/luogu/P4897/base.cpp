// 洛谷 P4897【模板】最小割树（Gomory-Hu Tree）。頂点 0 から n の n + 1 頂点、m 辺の無向連通グラフで、2 頂点の最小カット
// を Q 回答える (n ≤ 500、m ≤ 1500、Q ≤ 10^5、容量 ≤ 10^4)。
//
// 提出は self-flow-gomory-hu と同じものを実装する。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges);  // 無向辺 {u, v, 容量}。頂点は 0 から n - 1
//     void run();                                        // Gomory-Hu 木を作る
//     const vector<array<i64, 3>> &tree() const;         // 木の辺 {u, v, 重み} を n - 1 本
//   };
// 入力を読んだあと、構築と run を測る。問い合わせは計測区間の外で、木の上で全組の道の上の最小を求めてから答える。
#include "pj.hpp"
#include <algorithm>
#include <limits>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  ++n;
  vector<array<i64, 3>> edges(m);
  for (auto &e : edges) must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();
  vector<vector<pair<int, i64>>> adj(n);
  for (auto &[u, v, w] : sol.tree()) adj[u].push_back({(int)v, w}), adj[v].push_back({(int)u, w});
  vector<i64> mn((size_t)n * n, -1);
  vector<int> st;
  for (int s = 0; s < n; ++s) {
    i64 *row = &mn[(size_t)s * n];
    row[s] = numeric_limits<i64>::max(), st.assign(1, s);
    while (!st.empty()) {
      const int u = st.back();
      st.pop_back();
      for (auto [v, w] : adj[u])
        if (row[v] < 0) row[v] = min(row[u], w), st.push_back(v);
    }
  }
  int q;
  must_scan(scanf("%d", &q), 1);
  string out;
  for (int i = 0; i < q; ++i) {
    int u, v;
    must_scan(scanf("%d %d", &u, &v), 2);
    out += to_string(mn[(size_t)u * n + v]), out += '\n';
  }
  fputs(out.c_str(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
