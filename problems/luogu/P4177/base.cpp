// https://www.luogu.com.cn/problem/P4177
//
// 提出は次を実装する。
//   struct Solver {
//     // 仕事 i の収入は x[i]、工程は proc[off[i]] から proc[off[i + 1] - 1] で、proc[k] = {機械 (0 始まり), 借りる費用}。
//     // 機械 j を買う費用は y[j]
//     Solver(int n, int m, const vector<int> &x, const vector<int> &off, const vector<array<int, 2>> &proc,
//            const vector<int> &y);
//     void run();
//     long long answer() const;  // 利益の最大
//   };
//
// 1 ≤ N, M ≤ 1200、1 ≤ x_i ≤ 5000、借りる費用と買う費用は 20000 以下。工程の数の合計は最大 N M = 1.44 × 10^6 になる。
// 想定解は最大重み閉包の最小カット (s → 仕事に収入、仕事 → 機械に借りる費用、機械 → t に買う費用) で、この問題を置いたのは、
// 頂点が少なく辺の多い網を最大流で解く時間とメモリを測るため (algo-notes の network-flow-brute-force.md)。構築と run を測る。
// x、off、proc、y は計測区間のあとまで生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<int> x(n), off(n + 1), y(m);
  vector<array<int, 2>> proc;
  for (int i = 0; i < n; ++i) {
    int t;
    must_scan(scanf("%d %d", &x[i], &t), 2);
    for (int k = 0; k < t; ++k) {
      int a, b;
      must_scan(scanf("%d %d", &a, &b), 2);
      proc.push_back({a - 1, b});
    }
    off[i + 1] = proc.size();
  }
  for (auto &c : y) must_scan(scanf("%d", &c), 1);

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, m, x, off, proc, y);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
