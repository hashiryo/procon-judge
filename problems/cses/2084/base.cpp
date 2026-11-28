// https://cses.fi/problemset/task/2084
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &s, const vector<i64> &f, i64 x);  // 構築。計測区間の外。
//     void run();            // ここだけ測る。
//     i64 answer() const;    // 最小の合計時間
//   };
//
// n 体のモンスターを順に見て、倒すか逃げるかを選ぶ。最後のモンスターは必ず倒す。
// 倒すと s_i × 今の skill factor の時間がかかり、skill factor が f_i になる。
// s は広義単調増加、f は広義単調減少 (x >= f_1 >= … >= f_n) なので、遷移
// dp[i] = min_j (dp[j] + s_i f_j) は傾き f_j が単調な両端キューの CHT で引ける。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  i64 x;
  must_scan(scanf("%d %lld", &n, &x), 2);
  vector<i64> s = read_ints(n);
  vector<i64> f = read_ints(n);

  Solver sol(s, f, x);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", (long long)sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
