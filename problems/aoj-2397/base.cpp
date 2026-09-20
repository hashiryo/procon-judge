// https://onlinejudge.u-aizu.ac.jp/challenges/sources/JAG/Spring/2397
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int w, i64 h, const vector<array<i64, 2>> &obstacles);
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 1e9+9 での値
//   };
//
// obstacles は {行, 列} を 0-indexed にして行の昇順に並べたもの。行は最大
// 10^18 になるので、遷移行列の冪乗で飛ぶ。
//
// 1 つの入力に複数のデータセットが入っているので、ハーネスが全部読んでから
// データセットごとに Solver を作る。計測区間は全部の合計。出力の "Case k:"
// もハーネスが付ける。
#include <algorithm>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-matrix.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  vector<int> ws;
  vector<i64> hs;
  vector<vector<array<i64, 2>>> obss;
  for (;;) {
    i64 w, h, n;
    must_scan(scanf("%lld %lld %lld", &w, &h, &n), 3);
    if (w == 0) break;
    vector<array<i64, 2>> obs(n);
    for (auto &o : obs) {
      i64 x, y;
      must_scan(scanf("%lld %lld", &x, &y), 2);
      o = {y - 1, x - 1};
    }
    std::sort(obs.begin(), obs.end());
    ws.push_back((int)w), hs.push_back(h), obss.push_back(std::move(obs));
  }

  vector<i64> ans;
  ans.reserve(ws.size());

  auto t0 = chrono::steady_clock::now();
  for (size_t i = 0; i < ws.size(); ++i) {
    Solver s(ws[i], hs[i], obss[i]);
    s.run();
    ans.push_back(s.answer());
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  for (size_t i = 0; i < ans.size(); ++i) {
    out += "Case ";
    out += to_string(i + 1);
    out += ": ";
    out += to_string(ans[i]);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
