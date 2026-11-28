// https://judge.yosupo.jp/problem/eulerian_trail_directed
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<int> u, vector<int> v);  // 辺 i は u[i] -> v[i]。構築。計測区間の外。
//     void run();                                   // ここだけ測る。
//     bool ok() const;                              // オイラー路があるか
//     const vector<int> &vertex() const;             // 頂点列 (長さ M+1)
//     const vector<int> &edge() const;              // 辺番号 (長さ M)。edge[i] が vertex[i] から vertex[i+1]。
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);

  vector<Solver> sols;
  sols.reserve(t);
  for (int tc = 0; tc < t; ++tc) {
    int n, m;
    must_scan(scanf("%d %d", &n, &m), 2);
    vector<int> u(m), v(m);
    for (int i = 0; i < m; ++i) must_scan(scanf("%d %d", &u[i], &v[i]), 2);
    sols.emplace_back(n, std::move(u), std::move(v));
  }

  auto t0 = chrono::steady_clock::now();
  for (auto &s : sols) s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(4) << 20);
  for (auto &s : sols) {
    if (!s.ok()) {
      out += "No\n";
      continue;
    }
    out += "Yes\n";
    const auto &vs = s.vertex();
    for (size_t i = 0; i < vs.size(); ++i) {
      if (i) out += ' ';
      out += to_string(vs[i]);
    }
    out += '\n';
    const auto &es = s.edge();
    for (size_t i = 0; i < es.size(); ++i) {
      if (i) out += ' ';
      out += to_string(es[i]);
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
