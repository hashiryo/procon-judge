// https://judge.yosupo.jp/problem/common_interval_decomposition_tree
//
// 提出は次を実装する。
//   struct Solver {
//     struct Node { int parent; int l; int r; bool linear; };  // 区間は半開 [l, r)
//     Solver(int n, const vector<int> &p);   // 順列を写すだけ (計測区間の外)
//     void run();                            // 木を求める。ここだけ測る
//     const vector<Node> &answer() const;    // 根は parent == -1
//   };
//
// 出力は問題文の形 (p_i r_i - 1 S_i) にハーネスが直す。頂点の番号は構築順で、
// 問題文の p_i < i は満たさない (判定サイトの checker は番号を問わない)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo-rmq.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<int> p(n);
  for (auto &x : p) must_scan(scanf("%d", &x), 1);

  Solver sol(n, p);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<Solver::Node> &nodes = sol.answer();
  string out = to_string(nodes.size()) + '\n';
  out.reserve(nodes.size() * 24 + 16);
  for (const Solver::Node &v : nodes) {
    out += to_string(v.parent);
    out += ' ';
    out += to_string(v.l);
    out += ' ';
    out += to_string(v.r - 1);
    out += ' ';
    out += v.linear ? "linear\n" : "prime\n";
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
