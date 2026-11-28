// https://judge.yosupo.jp/problem/tree_diameter
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, vector<int> a, vector<int> b, vector<long long> c);  // 構築。計測区間の外。
//     void run();                    // ここだけ測る。
//     long long weight() const;      // 直径の重みの総和
//     const vector<int> &path() const;  // 直径の頂点列
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<int> a(n - 1), b(n - 1);
  vector<long long> c(n - 1);
  for (int i = 0; i < n - 1; ++i) {
    must_scan(scanf("%d %d %lld", &a[i], &b[i], &c[i]), 3);
  }

  Solver sol(n, a, b, c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> &p = sol.path();
  string out;
  out.reserve(p.size() * 8 + 32);
  out += to_string(sol.weight());
  out += ' ';
  out += to_string(p.size());
  out += '\n';
  for (size_t i = 0; i < p.size(); ++i) {
    if (i) out += ' ';
    out += to_string(p[i]);
  }
  out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
