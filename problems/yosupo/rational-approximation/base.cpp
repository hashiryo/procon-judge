// harness: 各提出が定義する run(queries) を計測する。
// I/O 変換は計測外で行う。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/fastest.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);

 int T;
 std::cin >> T;
 std::vector<std::tuple<i64, i64, i64>> queries(T);
 for (auto& [n, x, y] : queries) std::cin >> n >> x >> y;

 std::vector<std::tuple<i64, i64, i64, i64>> result;
 uint64_t best_ns = ~uint64_t(0);
 for (int rep = 0; rep < 1; ++rep) {
  auto t0 = chrono::steady_clock::now();
  result = run(queries);
  auto t1 = chrono::steady_clock::now();
  auto ns = (uint64_t) chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
  if (ns < best_ns) best_ns = ns;
 }

 for (auto& [a, b, c, d] : result) std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
 report_metrics((long long)best_ns);
 return 0;
}
