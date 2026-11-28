// https://judge.yosupo.jp/problem/wildcard_pattern_matching
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const string &s, const string &t);  // 構築。3 本の畳み込みの係数を作る。計測区間の外。
//     void run();                                // 3 本の畳み込み。ここだけ測る。
//     const string &answer() const;              // 長さ |S|-|T|+1。S[i,i+|T|) が T と一致する位置が '1'。
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  const string s = read_token();
  const string t = read_token();

  Solver sol(s, t);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  // 出力は 0/1 を区切り無しで並べた 1 トークン。整形は計測区間の外。
  string out = sol.answer();
  out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
