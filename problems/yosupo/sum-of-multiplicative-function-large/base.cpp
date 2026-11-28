// https://judge.yosupo.jp/problem/sum_of_multiplicative_function_large
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(u64 n, u64 a, u64 b);  // 構築。計測区間の外。
//     void run();                   // 和を求める。ここだけ測る。
//     u64 answer() const;           // sum f(i) (i = 1..n) mod 469762049
//   };
//
// f は乗法的で f(p^e) = a e + b p。法は 469762049 (素数)。
// 1 ファイルに T 個のケースが入る。構築を全部済ませてから、run だけを続けて測る。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<u64> ns(t), as(t), bs(t);
  for (int i = 0; i < t; ++i) {
    must_scan(scanf("%llu %llu %llu", &ns[i], &as[i], &bs[i]), 3);
  }

  vector<Solver> sols;
  sols.reserve(t);
  for (int i = 0; i < t; ++i) sols.emplace_back(ns[i], as[i], bs[i]);

  auto t0 = chrono::steady_clock::now();
  for (auto &s : sols) s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(t) * 10);
  for (auto &s : sols) {
    out += to_string(s.answer());
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
