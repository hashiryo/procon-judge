// https://judge.yosupo.jp/problem/two_sat
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &clauses);  // 節 (a, b) は a ∨ b。リテラルは入力と同じく、変数 i (1 から n) の
//                                                           // 肯定が i、否定が -i。clauses を覚えるだけにする。clauses は Solver より長く生きる
//     void run();                                           // ここだけ測る。
//     bool satisfiable() const;
//     bool value(int i) const;                              // 変数 i (1 から n) の値。satisfiable() が真のときだけ呼ぶ
//   };
//
// 含意グラフを組むところと作業領域の確保も run() の中に置く (yosupo-scc と同じ)。satisfiable() と value() は run() の結果から
// O(1) で返す。出力の整形は計測区間の外でハーネスが行う。設計は algo-notes の notes/graph-basics.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  read_token(), read_token();  // "p" と "cnf"
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> clauses(m);
  for (auto &c : clauses) {
    int zero;
    must_scan(scanf("%d %d %d", &c[0], &c[1], &zero), 3);
  }

  Solver s(n, clauses);

  auto t0 = chrono::steady_clock::now();
  s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  if (s.satisfiable()) {
    out.reserve(size_t(n) * 8 + 32);
    out += "s SATISFIABLE\nv";
    for (int i = 1; i <= n; ++i) {
      out += ' ';
      out += to_string(s.value(i) ? i : -i);
    }
    out += " 0\n";
  } else {
    out = "s UNSATISFIABLE\n";
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
