// 直線の集合の最大を時刻を進めながら問う (自作)。kinetic tournament tree の速さを測る。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a, const vector<i64> &b);  // 直線 y = a_i t + b_i
//     void set(int i, i64 a, i64 b);  // 直線 i を置き換える
//     i64 query(i64 t);               // 時刻 t の最大。t は前の query の t 以上
//   };
//
// N と Q は 2×10^5 以下、|a_i|, |b_i| <= 10^9、0 <= t <= 10^9。t = 10^9 でも a t + b は 1 桁 18 に収まる。
// 最初の N 本を入れるところも計測区間に入れてある (yosupo-line-add-get-min と同じ考え)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a(n), b(n);
  for (int i = 0; i < n; ++i) must_scan(scanf("%lld %lld", &a[i], &b[i]), 2);

  // 0 i a b は 4 項目、1 t は 2 項目。
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%lld %lld %lld", &e[1], &e[2], &e[3]), 3);
    else must_scan(scanf("%lld", &e[1]), 1), ++gets;
  }

  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  Solver s(a, b);
  for (auto &e : qs) {
    if (e[0] == 0) s.set((int)e[1], e[2], e[3]);
    else ans.push_back(s.query(e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
