// https://judge.yosupo.jp/problem/minimum_spanning_tree
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<int> a, vector<int> b, vector<long long> c);  // 構築。計測区間の外。
//     void run();                       // ここだけ測る。
//     long long weight() const;         // 全域木の重みの総和
//     const vector<int> &edges() const;  // 木に含まれる辺の番号 (N-1 本)
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
  vector<long long> c(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d %lld", &a[i], &b[i], &c[i]), 3);

  Solver sol(n, a, b, c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(m) * 8 + 32);
  out += to_string(sol.weight());
  out += '\n';
  const auto &es = sol.edges();
  for (size_t i = 0; i < es.size(); ++i) {
    if (i) out += ' ';
    out += to_string(es[i]);
  }
  out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
