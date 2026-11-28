// https://judge.yosupo.jp/problem/shortest_path
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int s, int t, vector<int> a, vector<int> b, vector<long long> c);
//     void run();                       // ここだけ測る。
//     bool ok() const;                  // t に届くか
//     long long weight() const;         // 最短距離
//     const vector<int> &path() const;  // 通る頂点の列 (s から t まで)
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, s, t;
  must_scan(scanf("%d %d %d %d", &n, &m, &s, &t), 4);
  vector<int> a(m), b(m);
  vector<long long> c(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d %lld", &a[i], &b[i], &c[i]), 3);

  Solver sol(n, s, t, a, b, c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(m) * 12 + 32);
  if (!sol.ok()) {
    out += "-1\n";
  } else {
    const auto &p = sol.path();
    out += to_string(sol.weight());
    out += ' ';
    out += to_string(p.size() - 1);
    out += '\n';
    for (size_t i = 0; i + 1 < p.size(); ++i) {
      out += to_string(p[i]);
      out += ' ';
      out += to_string(p[i + 1]);
      out += '\n';
    }
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
