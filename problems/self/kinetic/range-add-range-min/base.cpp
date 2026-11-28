// 要素ごとの係数の区間線形加算と区間最小 (自作)。kinetic segment tree の最小側を、最大側と同じ形で測る。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &x, const vector<i64> &y);  // 各要素の直線 (x_i, y_i)
//     void add(int l, int r, i64 a, i64 b);  // 0 <= a。i in [l, r) に y_i += a x_i + b
//     void set(int i, i64 x, i64 y);         // (x_i, y_i) = (x, y)
//     i64 minimum(int l, int r);             // min(y_l, ..., y_{r-1})
//   };
//
// N と Q は 10^5 以下、|x_i| <= 10^6、|y_i| <= 10^9、0 <= a <= 10^6、|b| <= 10^9。1 つの要素に足される値は
// 2×10^17 ほどまで動くので、y は long long で持てる。取得の名前を min にしないのは、提出の中で std::min を
// 呼べなくなるのを避けるため。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> x = read_ints(n), y = read_ints(n);

  // 0 l r a b は 5 項目、1 l r は 3 項目、2 i x y は 4 項目。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld", &e[0]), 1);
    if (e[0] == 0) must_scan(scanf("%lld %lld %lld %lld", &e[1], &e[2], &e[3], &e[4]), 4);
    else if (e[0] == 1) must_scan(scanf("%lld %lld", &e[1], &e[2]), 2), ++gets;
    else must_scan(scanf("%lld %lld %lld", &e[1], &e[2], &e[3]), 3);
  }

  Solver s(x, y);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.add((int)e[1], (int)e[2], e[3], e[4]);
    else if (e[0] == 1) ans.push_back(s.minimum((int)e[1], (int)e[2]));
    else s.set((int)e[1], e[2], e[3]);
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
