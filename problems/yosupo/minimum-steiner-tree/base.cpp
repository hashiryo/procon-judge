// https://judge.yosupo.jp/problem/minimum_steiner_tree
//
// 提出は次を実装する。
//   struct Solver {
//     // 構築。計測区間の外。辺 i は u[i] と v[i] を結び、重みは w[i]。
//     // terminals は結ぶべき頂点 X (K 個)。
//     Solver(int n, const vector<int> &u, const vector<int> &v,
//            const vector<long long> &w, const vector<int> &terminals);
//     void run();                        // ここだけ測る。
//     long long weight() const;          // 選んだ辺の重みの総和 Y
//     const vector<int> &edges() const;  // 選んだ辺の番号 (Z 本, K-1 <= Z <= N-1)
//   };
//
// 出力は "Y Z" の 1 行と、辺の番号を並べた 1 行。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> u(m), v(m);
  vector<long long> w(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d %lld", &u[i], &v[i], &w[i]), 3);
  int k;
  must_scan(scanf("%d", &k), 1);
  vector<int> xs(k);
  for (int i = 0; i < k; ++i) must_scan(scanf("%d", &xs[i]), 1);

  Solver sol(n, u, v, w, xs);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const auto &es = sol.edges();
  string out;
  out.reserve(es.size() * 6 + 64);
  out += to_string(sol.weight());
  out += ' ';
  out += to_string(es.size());
  out += '\n';
  for (size_t i = 0; i < es.size(); ++i) {
    if (i) out += ' ';
    out += to_string(es[i]);
  }
  out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
