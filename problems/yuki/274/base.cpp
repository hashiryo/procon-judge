// https://yukicoder.me/problems/no/274
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int m, const vector<i64> &l, const vector<i64> &r);  // 構築。計測区間の外。
//     void run();           // ここだけ測る。
//     bool answer() const;  // 良い壁が作れるなら true
//   };
//
// ブロック i を裏返すかどうかを x_i とすると、2 つのブロックの位置関係から
// x_i = x_j か x_i != x_j の制約が出る。全対の制約に矛盾が無いかを判定する問題で、
// 有向 2-SAT (強連結成分分解) と、経路圧縮あり / なしのポテンシャル付き
// Union-Find を比べる。
//
// 全対の走査 O(N^2) はどの実装も同じだが、矛盾を見つけた時点で止められるかどうか
// が実装で違うので、走査ごと計測区間に入れる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<i64> l(n), r(n);
  for (int i = 0; i < n; ++i) must_scan(scanf("%lld %lld", &l[i], &r[i]), 2);

  Solver sol(m, l, r);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  puts(sol.answer() ? "YES" : "NO");

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
