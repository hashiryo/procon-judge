// https://judge.yosupo.jp/problem/dirichlet_convolution_and_prefix_sums
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(u64 n, vector<u32> f, vector<u32> g);  // Q_N 昇順の累積和。構築。計測区間の外。
//     void run();                                   // ここだけ測る。
//     const vector<u32> &answer() const;            // h = f * g の累積和 (Q_N 昇順)
//   };
//
// Q_N の要素の個数は 2 sqrt(N) ほど。1 ファイルに T 個のケースが入る。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);

  vector<Solver> sols;
  sols.reserve(t);
  for (int tc = 0; tc < t; ++tc) {
    u64 n;
    must_scan(scanf("%llu", &n), 1);
    const Quotients q = make_quotients(n);
    vector<u32> f(q.len()), g(q.len());
    for (auto &x : f) must_scan(scanf("%u", &x), 1);
    for (auto &x : g) must_scan(scanf("%u", &x), 1);
    sols.emplace_back(n, std::move(f), std::move(g));
  }

  auto t0 = chrono::steady_clock::now();
  for (auto &s : sols) s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(4) << 20);
  for (auto &s : sols) {
    for (u32 v : s.answer()) {
      out += to_string(v);
      out += ' ';
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
