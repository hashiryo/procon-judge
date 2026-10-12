// https://judge.yosupo.jp/problem/eertree
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);         // 構築。文字列を写すだけで、計測区間の外。
//     void run();                               // 回文木を求める。ここだけ測る。
//     const EertreeAnswer &answer() const;      // 回文の頂点の数 n = par.size() - 1、親 par[v] と suffix link link[v] (1 <= v <= n)、
//   };                                          // 各接頭辞の最長の回文の接尾辞の頂点 last[i] (0 <= i < |S|)。整形は外。
//
// 頂点の番号は問題文と同じく、ODD が -1、EVEN が 0、回文の頂点は初めて現れて終わる位置の順に 1 から (回文木を左から作るときに
// できる順と同じ)。par[0] と link[0] は使わない。1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける
// (yosupo-zalgorithm と同じ形)。設計は algo-notes の notes/eertree.md。
#include "pj.hpp"

struct EertreeAnswer {
  vector<int> par, link, last;
};

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/eer_basic.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  string s = read_token();

  Solver sol(s);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const EertreeAnswer &a = sol.answer();
  const int n = int(a.par.size()) - 1;
  string out = to_string(n) + '\n';
  out.reserve(size_t(n) * 16 + a.last.size() * 8 + 16);
  for (int v = 1; v <= n; ++v) out += to_string(a.par[v]) + ' ' + to_string(a.link[v]) + '\n';
  for (size_t i = 0; i < a.last.size(); ++i) out += to_string(a.last[i]) + (i + 1 == a.last.size() ? '\n' : ' ');
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
