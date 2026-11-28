// 法 998244353 固定のテトレーション (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver { unsigned solve(unsigned a, unsigned b); };  // a ↑↑ b mod 998244353
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int n;
 must_scan(scanf("%d", &n), 1);
 vector<u32> as(n), bs(n);
 for (int i = 0; i < n; ++i) must_scan(scanf("%u %u", &as[i], &bs[i]), 2);

 Solver sol;
 vector<u32> ans;
 ans.reserve(n);

 auto t0 = chrono::steady_clock::now();
 for (int i = 0; i < n; ++i) ans.push_back(sol.solve(as[i], bs[i]));
 auto t1 = chrono::steady_clock::now();

 print_all(ans);
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
