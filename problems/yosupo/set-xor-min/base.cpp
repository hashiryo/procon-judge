// https://judge.yosupo.jp/problem/set_xor_min
//
// 提出は次を実装する。
//   struct Solver {
//     Solver();                 // 構築。計測区間の外。
//     void insert(int x);       // 既にあれば何もしない
//     void erase(int x);        // 無ければ何もしない
//     int xor_min(int x);       // 集合の要素 y について min(y xor x)
//   };
//
// 値は 0 以上 2^30 未満。集合は空から始まるので、構築に測るものが無い。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-segtree-patricia.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<array<int, 2>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    if (e[0] == 2) ++gets;
  }

  Solver s;
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.insert(e[1]);
    else if (e[0] == 1) s.erase(e[1]);
    else ans.push_back(s.xor_min(e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
