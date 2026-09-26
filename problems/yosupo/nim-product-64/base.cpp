// harness: 各提出が定義する run(as, bs) を計測する。
// yosupo "Nim Product (F_{2^64})" 形式の I/O。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int T;
 cin >> T;
 vector<u64> as(T), bs(T);
 for (int i = 0; i < T; ++i) cin >> as[i] >> bs[i];

 constexpr int REPEAT = 1;
 uint64_t best_ns = ~uint64_t(0);
 vector<u64> result;

 for (int rep = 0; rep < REPEAT; ++rep) {
  auto t0 = chrono::steady_clock::now();
  auto r = run(as, bs);
  auto t1 = chrono::steady_clock::now();
  result = std::move(r);
  auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
  if (ns < best_ns) best_ns = ns;
 }

 // 高速 stdout: 改行区切りで T 行
 for (auto x : result) cout << x << '\n';
 report_metrics((long long)best_ns);
 return 0;
}
