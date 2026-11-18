// 洛谷 P13843 (【模板】集合幂级数 exp（非素数模数）)。集合冪級数の exp(F) (F[∅] = 0) を 2^64 で割った余り。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<u64> &f);  // 構築。計測区間の外。
//     void run();                           // ここだけ測る。
//     vector<u64> answer() const;           // 2^n 個
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<u64> f(size_t(1) << n);
  for (auto &x : f) must_scan(scanf("%llu", (unsigned long long *)&x), 1);

  Solver sol(n, f);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<u64> r = sol.answer();
  string out;
  out.reserve(r.size() * 21);
  char buf[24];
  for (size_t i = 0; i < r.size(); ++i) {
    const int len = snprintf(buf, sizeof(buf), "%llu", (unsigned long long)r[i]);
    out.append(buf, len);
    out.push_back(i + 1 == r.size() ? '\n' : ' ');
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
