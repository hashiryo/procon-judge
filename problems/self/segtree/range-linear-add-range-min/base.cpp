// 区間に 1 次関数を足して区間最小 (自作)。入力は gen.py の説明のとおりで、Library Checker の range_linear_add_range_min と
// 同じ形式と制約。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);   // 構築。計測区間の外。
//     void add(int l, int r, i64 b, i64 c);    // i in [l, r) に a[i] += b i + c
//     i64 minimum(int l, int r);               // min(a[l], ..., a[r-1])
//   };
//
// N と Q は 10^5 以下、|a_i| < 10^9、|b| < 100、|c| < 10^7。10^5 回の加算で値は 2 * 10^12 ほどまで動くので
// i64 で渡す。取得の名前を min にしないのは range-add-range-min と同じく、提出の中で std::min を呼べなく
// なるのを避けるため。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<i64> a = read_ints(n);

  // 0 l r b c と 1 l r で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 0) must_scan(scanf("%lld %lld", &e[3], &e[4]), 2);
    else ++gets;
  }

  Solver s(a);
  vector<i64> ans;
  ans.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.add((int)e[1], (int)e[2], e[3], e[4]);
    else ans.push_back(s.minimum((int)e[1], (int)e[2]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
