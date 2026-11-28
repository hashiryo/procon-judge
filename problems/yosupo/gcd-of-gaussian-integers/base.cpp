// https://judge.yosupo.jp/problem/gcd_of_gaussian_integers
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(i64 a1, i64 b1, i64 a2, i64 b2);  // a1 + b1 i と a2 + b2 i。構築。計測区間の外。
//     void run();                              // gcd を求める。ここだけ測る。
//     i64 x() const, y() const;                // gcd を x + y i で返す。
//   };
//
// 答えは単位 (±1, ±i) 倍の不定性があるので checker で判定する。
// 1 ファイルに T 個のケースが入る。構築を全部済ませてから、run だけを続けて測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<i64> a1(t), b1(t), a2(t), b2(t);
  for (int i = 0; i < t; ++i) {
    must_scan(scanf("%lld %lld %lld %lld", &a1[i], &b1[i], &a2[i], &b2[i]), 4);
  }

  vector<Solver> sols;
  sols.reserve(t);
  for (int i = 0; i < t; ++i) sols.emplace_back(a1[i], b1[i], a2[i], b2[i]);

  auto t0 = chrono::steady_clock::now();
  for (auto &s : sols) s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(t) * 26);
  for (auto &s : sols) {
    out += to_string(s.x());
    out += ' ';
    out += to_string(s.y());
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
