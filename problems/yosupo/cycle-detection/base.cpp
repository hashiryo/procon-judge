// https://judge.yosupo.jp/problem/cycle_detection
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<int> u, vector<int> v);  // 辺 i は u[i] -> v[i]。構築。計測区間の外。
//     void run();                                   // ここだけ測る。
//     const vector<int> &cycle() const;             // 閉路の辺番号 (この順に通ると閉じる)。無ければ空。
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> u(m), v(m);
  for (int i = 0; i < m; ++i) must_scan(scanf("%d %d", &u[i], &v[i]), 2);

  Solver sol(n, u, v);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> &c = sol.cycle();
  string out;
  out.reserve(c.size() * 8 + 8);
  if (c.empty()) {
    out += "-1\n";
  } else {
    out += to_string(c.size());
    out += '\n';
    for (int e : c) {
      out += to_string(e);
      out += '\n';
    }
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
