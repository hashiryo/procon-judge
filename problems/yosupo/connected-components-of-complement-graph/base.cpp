// https://judge.yosupo.jp/problem/connected_components_of_complement_graph
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<int> a, vector<int> b);  // 辺 (a[i], b[i]) の補グラフ。構築。計測区間の外。
//     void run();                                   // ここだけ測る。
//     const vector<vector<int>> &components() const;  // 補グラフの連結成分 (頂点番号の列)
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> a(m), b(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d", &a[i], &b[i]), 2);

  Solver sol(n, a, b);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const auto &cs = sol.components();
  string out;
  out.reserve(size_t(n) * 8 + 32);
  out += to_string(cs.size());
  out += '\n';
  for (auto &c : cs) {
    out += to_string(c.size());
    for (int v : c) out += ' ', out += to_string(v);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
