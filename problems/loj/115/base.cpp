// https://loj.ac/p/115
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 4>> &edges);  // {u, v, 下限, 上限}
//     void run();                  // 下限と上限を守り、どの頂点でも釣り合う流れを 1 つ求める
//     bool feasible() const;       // 流れがあるか
//     const vector<i64> &flows() const;  // あれば辺ごとの流量 (入力の順)
//   };
//
// 頂点は 0-indexed にして渡す。n は 200 以下、辺は 10200 本以下で、0 <= 下限 <= 上限 < 3000。流れはどれでもよいので、
// checker.cpp が、YES と NO が期待出力と一致することと、YES なら流量が下限と上限の間にあり、どの頂点でも入る量と出る量
// が等しいことを確かめる。構築と run を測る。辺ごとの流量を取り出す分も run に入れる。edges は計測区間のあとまで生きて
// いるので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 4>> edges(m);
  for (auto &e : edges) {
    must_scan(scanf("%d %d %d %d", &e[0], &e[1], &e[2], &e[3]), 4);
    --e[0], --e[1];
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  if (!sol.feasible()) {
    puts("NO");
  } else {
    string out = "YES\n";
    for (i64 f : sol.flows()) out += to_string(f), out += '\n';
    fwrite(out.data(), 1, out.size(), stdout);
  }

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
