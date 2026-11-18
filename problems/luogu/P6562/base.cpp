// 洛谷 P6562 ([SBCOI2020] 归家之路)。a ⊆ c ⊆ b の c への加算と和の問い (2^32 で割った余り)。
//
// 提出は次を実装する。操作は {種類, a, b, k} で、種類 2 の k は 0。
//   struct Solver {
//     Solver(int n, const vector<u32> &a, const vector<array<u32, 4>> &ops);  // 構築。計測区間の外。
//     void run();                                                             // ここだけ測る。
//     vector<u32> answer() const;                                             // 種類 2 の答えを順に
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<u32> a(size_t(1) << n);
  for (auto &x : a) must_scan(scanf("%u", &x), 1);
  vector<array<u32, 4>> ops(q);
  for (auto &o : ops) {
    must_scan(scanf("%u %u %u", &o[0], &o[1], &o[2]), 3);
    o[3] = 0;
    if (o[0] == 1) must_scan(scanf("%u", &o[3]), 1);
  }

  Solver sol(n, a, ops);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<u32> r = sol.answer();
  string out;
  out.reserve(r.size() * 11);
  char buf[16];
  for (u32 x : r) {
    const int len = snprintf(buf, sizeof(buf), "%u\n", x);
    out.append(buf, len);
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
