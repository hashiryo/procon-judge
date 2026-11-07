// 区間代入と区間最小 (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void assign(int l, int r, i64 x);       // i in [l, r) に a[i] = x
//     i64 minimum(int l, int r);        // min(a[l], ..., a[r-1])
//   };
//
// 値は初めも代入も絶対値 10^9 以下。取得の名前を min にしないのは、提出の中で std::min を呼べなくなるのを避けるため。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);
  // 0 l r x と 1 l r で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 4>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 0) must_scan(scanf("%lld", &e[3]), 1);
    else ++gets;
  }

  Solver s(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.assign((int)e[1], (int)e[2], e[3]);
    else ans.push_back(s.minimum((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
