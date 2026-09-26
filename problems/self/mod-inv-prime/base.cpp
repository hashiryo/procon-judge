// harness: 各提出が定義する run(p, queries) を計測する。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive_fermat.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 u32 p;
 int n;
 cin >> p >> n;
 vector<u32> qs(n);
 for (auto& a : qs) cin >> a;

 uint64_t best_ns = ~uint64_t(0);
 vector<u32> result;
 for (int rep = 0; rep < 1; ++rep) {
  auto t0 = chrono::steady_clock::now();
  auto r = run(p, qs);
  auto t1 = chrono::steady_clock::now();
  result = std::move(r);
  auto ns = (uint64_t) chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
  if (ns < best_ns) best_ns = ns;
 }
 // u32(-1) は "逆元なし" sentinel として "-1" として出力
 for (auto x : result) cout << (int32_t) x << '\n';
 report_metrics((long long)best_ns);
 return 0;
}
