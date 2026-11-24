// DMOPC '19 Contest 3 P6 Triple Spanning Tree。N 頂点 M 辺 (多重辺あり、連結とは限らない) のグラフから、辺素な全域木を
// 3 本作る。作れなければ -1、作れれば長さ M の文字列で、i 番目の文字は辺 i を使わないなら 0、E_1 に入れるなら 1、E_2 なら 2、
// E_3 なら 3 を出す (解は何でもよい)。2 ≤ N ≤ 500、1 ≤ M ≤ 1500。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 辺は 0 始まり
//     void run();
//     bool ok() const;                // 辺素な全域木が 3 本取れるか
//     const vector<int> &group() const;  // ok のとき、辺ごとの木の番号 (0 から 2、使わない辺は -1)
//   };
// 入力を読んだあと、構築と run を測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> e(m);
  for (auto &[u, v] : e) must_scan(scanf("%d %d", &u, &v), 2), --u, --v;
  auto t0 = chrono::steady_clock::now();
  Solver sol(n, e);
  sol.run();
  auto t1 = chrono::steady_clock::now();
  string out;
  if (!sol.ok()) out = "-1";
  else
    for (int g : sol.group()) out += char('0' + g + 1);
  out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
