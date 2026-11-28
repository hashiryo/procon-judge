// https://judge.yosupo.jp/problem/cycle_detection_undirected
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<int> u, vector<int> v);  // 辺 i は u[i] と v[i] を結ぶ。構築。計測区間の外。
//     void run();                                   // ここだけ測る。
//     const vector<int> &vertex() const;             // 閉路の頂点列 (長さ L)
//     const vector<int> &edge() const;              // 閉路の辺番号 (長さ L)。edge[i] が vertex[i] と vertex[i+1] を結ぶ。
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

  const vector<int> &vs = sol.vertex();
  string out;
  out.reserve(vs.size() * 16 + 8);
  if (vs.empty()) {
    out += "-1\n";
  } else {
    out += to_string(vs.size());
    out += '\n';
    for (size_t i = 0; i < vs.size(); ++i) {
      if (i) out += ' ';
      out += to_string(vs[i]);
    }
    out += '\n';
    for (size_t i = 0; i < vs.size(); ++i) {
      if (i) out += ' ';
      out += to_string(sol.edge()[i]);
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
