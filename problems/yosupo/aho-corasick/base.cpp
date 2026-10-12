// https://judge.yosupo.jp/problem/aho_corasick
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<string> &ss);   // 構築。文字列の列を写すだけで、計測区間の外。
//     void run();                                  // トライと suffix link を求める。ここだけ測る。
//     const AhoCorasickAnswer &answer() const;     // 頂点の数 n = par.size()、親 par[v] と suffix link link[v] (1 <= v < n)、
//   };                                             // 各文字列の頂点 word[i]。整形は外。
//
// 頂点の番号は、文字列を 0 番から順にトライへ入れたときに頂点ができる順 (問題文の (i, j) の辞書順と同じ)。par[0] と link[0] は使わない。
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-zalgorithm と同じ形)。設計は algo-notes の
// notes/aho-corasick.md。
#include "pj.hpp"

struct AhoCorasickAnswer {
  vector<int> par, link, word;
};

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  const int N = int(read_ints(1)[0]);
  vector<string> ss(N);
  for (auto &s : ss) s = read_token();

  Solver sol(ss);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const AhoCorasickAnswer &a = sol.answer();
  const int n = int(a.par.size());
  string out = to_string(n) + '\n';
  out.reserve(size_t(n) * 16 + a.word.size() * 8 + 16);
  for (int v = 1; v < n; ++v) out += to_string(a.par[v]) + ' ' + to_string(a.link[v]) + '\n';
  for (size_t i = 0; i < a.word.size(); ++i) out += to_string(a.word[i]) + (i + 1 == a.word.size() ? '\n' : ' ');
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
