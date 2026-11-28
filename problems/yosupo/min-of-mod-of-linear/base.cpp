// https://judge.yosupo.jp/problem/min_of_mod_of_linear
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(u64 n, u64 m, u64 a, u64 b);  // 構築。計測区間の外。
//     void run();                          // ここだけ測る。
//     u64 answer() const;                  // min{ (a x + b) mod m | 0 <= x < n }
//   };
//
// 1 ファイルに T 個のケースが入る。構築を全部済ませてから、run だけを続けて測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<u64> ns(t), ms(t), as(t), bs(t);
  for (int i = 0; i < t; ++i) {
    must_scan(scanf("%llu %llu %llu %llu", &ns[i], &ms[i], &as[i], &bs[i]), 4);
  }

  vector<Solver> sols;
  sols.reserve(t);
  for (int i = 0; i < t; ++i) sols.emplace_back(ns[i], ms[i], as[i], bs[i]);

  auto t0 = chrono::steady_clock::now();
  for (auto &s : sols) s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(t) * 11);
  for (auto &s : sols) {
    out += to_string(s.answer());
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
