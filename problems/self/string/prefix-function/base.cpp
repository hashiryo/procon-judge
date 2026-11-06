// 自作の問題。文字列の接頭辞関数 (各接頭辞の最長の border の長さ、KMP の失敗関数) を、文字列の族で測る。KMP と border の
// 使い方の代表として作った。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する (yosupo-zalgorithm と同じ 3 段の形)。
//   struct Solver {
//     explicit Solver(const string &s);   // 構築。計測区間の外。
//     void run();                         // 接頭辞関数を求める。ここだけ測る。
//     const vector<int> &answer() const;  // pi[i] は s[0, i] の、自分より短い最長の border の長さ (pi[0] = 0)
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに、文字列の作り方 (_shared/string/_common.hpp) を 1 行書く。文字列はハーネスが
// 計測の前に作る。文字は英小文字で、長さは 1 以上 10^6 以下。ブロックごとに run() を測って足す。s は計測区間のあとまで
// 生きているので、提出は参照を持ってよい。出力はブロックごとに答えを 32 個ずつ 1 行に並べる。
#include "pj.hpp"
#include "_shared/string/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<string> ss(t);
  for (auto &s : ss) {
    s = string_family::read_string();
    if (s.empty()) {
      fprintf(stderr, "empty string\n");
      return 1;
    }
  }

  long long ns = 0;
  string out;
  for (int i = 0; i < t; ++i) {
    Solver sol(ss[i]);
    auto t0 = chrono::steady_clock::now();
    sol.run();
    auto t1 = chrono::steady_clock::now();
    ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    const vector<int> &pi = sol.answer();
    if (pi.size() != ss[i].size()) {
      fprintf(stderr, "answer size %zu, expected %zu\n", pi.size(), ss[i].size());
      return 1;
    }
    for (size_t k = 0; k < pi.size(); ++k) {
      out += to_string(pi[k]);
      out += k % 32 == 31 || k + 1 == pi.size() ? '\n' : ' ';
    }
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics(ns);
}
