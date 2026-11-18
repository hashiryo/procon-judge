// https://judge.yosupo.jp/problem/factorization_of_polynomials
//
// 提出は次を実装する。
//   vector<pair<int, vector<u32>>> run(u32 p, const vector<u32> &a);
// 素数 p ≤ 998244353 と、次数 N ≤ 100 の monic な多項式 a = Σ a_i x^i を、monic な既約多項式の積に分解する。
// 返すのは (重複度, 因子の係数 b_0, ..., b_d (b_d = 1)) の組で、順は任意。a = 1 なら空。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int n;
  u32 p;
  must_scan(scanf("%d %u", &n, &p), 2);
  vector<u32> a(n + 1);
  for (auto &x : a) must_scan(scanf("%u", &x), 1);
  auto t0 = chrono::steady_clock::now();
  auto fs = run(p, a);
  auto t1 = chrono::steady_clock::now();
  string out = to_string(fs.size()) + "\n";
  for (auto &[e, b] : fs) {
    out += to_string(e) + " " + to_string(b.size() - 1);
    for (u32 x : b) out += " " + to_string(x);
    out += "\n";
  }
  fputs(out.c_str(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
