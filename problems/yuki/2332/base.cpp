// https://yukicoder.me/problems/no/2332
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<i64> &a, const vector<i64> &b, const vector<i64> &c);  // 計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 作れなければ -1
//   };
//
// B を左から埋めていく DP で、位置 l から A の接頭辞を k 個足せるのは k が
// 「A と B の l 文字目以降の最長共通接頭辞」以下のとき。費用 k * C_l は x = l + k の
// 1 次式なので、区間つきで直線を入れる Li Chao 木で DP を回す。DP は common.hpp に
// 置き、最長共通接頭辞をローリングハッシュの二分探索で取るか Z アルゴリズムで
// 取るかを比べる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-z-algorithm.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<i64> a = read_ints(n);
  vector<i64> b = read_ints(m);
  vector<i64> c = read_ints(m);

  Solver sol(a, b, c);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
