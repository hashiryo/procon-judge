// 相異なる n 点 (x_i, y_i) を通る n-1 次以下の多項式の係数 c_0, ..., c_{n-1} (mod 998244353) を、ケースごとに求める。
// n は 5000 以下で、FFT を使わない O(n^2) で収まる大きさ。点の少ないケースを大量に聞く入力もある。
//
// 提出は次を実装する。
//   vector<u32> run(const vector<u32> &x, const vector<u32> &y);  // 長さ n
// 計測は全ケースの run の時間の和。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<vector<u32>> xs(t), ys(t);
  for (int q = 0; q < t; ++q) {
    int n;
    must_scan(scanf("%d", &n), 1);
    xs[q].resize(n), ys[q].resize(n);
    for (auto &v : xs[q]) must_scan(scanf("%u", &v), 1);
    for (auto &v : ys[q]) must_scan(scanf("%u", &v), 1);
  }
  vector<vector<u32>> cs(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) cs[q] = run(xs[q], ys[q]);
  auto t1 = chrono::steady_clock::now();
  string out;
  for (auto &c : cs) {
    for (size_t i = 0; i < c.size(); ++i) {
      out += to_string(c[i]);
      out += i + 1 == c.size() ? '\n' : ' ';
    }
  }
  fputs(out.c_str(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
