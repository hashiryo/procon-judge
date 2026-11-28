// https://judge.yosupo.jp/problem/chordal_graph_recognition
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 辺を覚えるだけ (計測区間の外)
//     void run();                                         // ここだけ測る
//     bool chordal() const;
//     const vector<int> &order() const;                   // chordal のとき PEO (長さ n)
//     const vector<int> &cycle() const;                   // そうでないとき長さ 4 以上の誘導閉路 (巡回順、始点は繰り返さない)
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/mcs_set.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver s(n, edges);

  auto t0 = chrono::steady_clock::now();
  s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  if (s.chordal()) {
    const vector<int> &o = s.order();
    out.reserve(size_t(n) * 8 + 8);
    out += "YES\n";
    for (int i = 0; i < n; ++i) {
      if (i) out += ' ';
      out += to_string(o[i]);
    }
    out += '\n';
  } else {
    const vector<int> &c = s.cycle();
    out.reserve(c.size() * 8 + 24);
    out += "NO\n";
    out += to_string(c.size());
    out += '\n';
    for (size_t i = 0; i < c.size(); ++i) {
      if (i) out += ' ';
      out += to_string(c[i]);
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
