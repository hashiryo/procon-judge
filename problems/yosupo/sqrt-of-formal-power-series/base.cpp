// https://judge.yosupo.jp/problem/sqrt_of_formal_power_series
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<i64> &a);  // 構築。計測区間の外。
//     void run();                            // 平方根を作る。ここだけ測る。
//     vector<i64> answer() const;            // 係数 N 個。平方根が無ければ空。
//   };
//
// 剰余は 998244353 で、入出力は素の整数で渡す。内部表現を提出が選べるようにする
// ためで、理由はハーネスの節に書いてある。変換は O(N)、計算は O(N log N) なので、
// 一緒に測ると表現の選び方で乗り方が変わる。構築と取り出しを分けてあるのはそのため。
// 平方根は一意でない (最下位の次数が 0 でないとき、g^2 = f (mod x^N) を満たす g は複数ある) ので、
// 判定は checker (g^2 の先頭 N 項が f と一致するか) に任せる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-fps-sqrt.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n;
  must_scan(scanf("%d", &n), 1);
  vector<i64> a = read_ints(n);

  Solver sol(a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<i64> ans = sol.answer();
  if (ans.empty()) {
    puts("-1");
  } else {
    print_all(ans, ' ');
  }

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
