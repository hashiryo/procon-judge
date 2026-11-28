// https://judge.yosupo.jp/problem/two_square_sum
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(u64 n);                          // 構築。計測区間の外。
//     void run();                             // ここだけ測る。
//     const vector<pair<u64, u64>> &answer() const;  // n = a^2 + b^2 の (a, b) を全部
//   };
//
// 出力はケースごとに、組の数 n の行と、そのあとに n 行の "a b"。組の順は任意 (checker)。
// 1 ファイルに Q 個のケースが入る。構築を全部済ませてから、run だけを続けて測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<u64> ns(q);
  for (int i = 0; i < q; ++i) must_scan(scanf("%llu", &ns[i]), 1);

  vector<Solver> sols;
  sols.reserve(q);
  for (int i = 0; i < q; ++i) sols.emplace_back(ns[i]);

  auto t0 = chrono::steady_clock::now();
  for (auto &s : sols) s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(q) * 64);
  for (auto &s : sols) {
    const auto &v = s.answer();
    out += to_string(v.size());
    out += '\n';
    for (auto &[a, b] : v) {
      out += to_string(a);
      out += ' ';
      out += to_string(b);
      out += '\n';
    }
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
