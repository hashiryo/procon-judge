// https://judge.yosupo.jp/problem/min_cost_b_flow
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<i64> &b, const vector<array<i64, 5>> &edges);  // 辺は {s, t, 下限, 上限, 費用}
//     void run();                             // 最小費用の b-flow を求める
//     bool feasible() const;                  // b-flow があるか
//     __int128 cost() const;                  // 費用の和
//     const vector<i64> &potential() const;   // 頂点ごとのポテンシャル p
//     const vector<i64> &flow() const;        // 辺ごとの流量
//   };
//
// 頂点 v の b_v は湧き出し (正) か吸い込み (負)。ポテンシャルは、縮約費用 c + p_s - p_t が正の辺は下限まで、負の辺は
// 上限まで流れているという相補性を満たし、絶対値は 10^15 以下にする。辺には自己ループもあり、費用の和は 2^64 を
// 超えうる。答えは一意でないので判定はチェッカに任せる。構築と run を測る。b と edges は計測区間のあとまで
// 生きているので、提出は参照を持ってよい。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-network-simplex.hpp"
#endif
#include SUBMISSION_HPP

static string i128_to_string(__int128 v) {
  if (v == 0) return "0";
  const bool neg = v < 0;
  unsigned __int128 x = neg ? -(unsigned __int128)v : (unsigned __int128)v;
  string s;
  while (x) s += char('0' + (int)(x % 10)), x /= 10;
  if (neg) s += '-';
  return string(s.rbegin(), s.rend());
}

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<i64> b = read_ints(n);
  vector<array<i64, 5>> edges(m);
  for (auto &e : edges) must_scan(scanf("%lld %lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3], &e[4]), 5);

  auto t0 = chrono::steady_clock::now();
  Solver sol(n, b, edges);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  if (!sol.feasible()) {
    out = "infeasible\n";
  } else {
    out = i128_to_string(sol.cost());
    out += '\n';
    for (i64 p : sol.potential()) out += to_string(p), out += '\n';
    for (i64 f : sol.flow()) out += to_string(f), out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
