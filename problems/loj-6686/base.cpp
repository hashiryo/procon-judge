// https://loj.ac/p/6686
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(unsigned __int128 n);  // n <= 1e30。計測区間の外。
//     void run();          // ここだけ測る。
//     i64 answer() const;  // mod 998244353
//   };
//
// sum_{i<=n} gcd(floor(cbrt(i)), i)。立方根が最大の区間は約数ごとに直接数え、
// それより手前は乗法的関数の和に直す。前処理は common.hpp に置き、乗法的関数の
// 和を Dirichlet 級数の積で出すか、素数冪の和からの乗法的関数の総和で出すかを
// 比べる。n は long long を超えるので、文字列から 128 bit に直して渡す。
// テストデータは LOJ から手で取り込む (source = manual)。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-multiplicative-sum.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  string s = read_token();
  unsigned __int128 n = 0;
  for (char c : s) n = n * 10 + (c - '0');

  Solver sol(n);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
