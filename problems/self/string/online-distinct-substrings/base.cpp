// 自作の問題。空の文字列に 1 文字ずつ足していき、足すたびにその時点の異なる (空でない) 部分文字列の数を答える。文字を足しながら
// 答える使い方の代表として作った (接尾辞オートマトンや Ukkonen の接尾辞木が向く。接尾辞配列のように全体を見てから作る構造は
// 使えない)。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver();
//     u64 push(char c);  // c を末尾に足し、今の文字列の異なる部分文字列の数を返す
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに、文字列の作り方 (_shared/string/_common.hpp) を 1 行書く。文字列はハーネスが計測の
// 前に作る。文字は英小文字で、長さ n は 1 以上 5 * 10^5 以下。ブロックごとに、構築とすべての push を測って足す。
// 出力はブロックごとに 1 行で、n、すべての答えを h <- h * K + x (mod 2^64、K = 0x9E3779B97F4A7C15) で畳んだ値、
// 足した文字の数が 2 の冪のときと最後の答えを並べる。
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

  vector<vector<u64>> ans(t);
  for (int i = 0; i < t; ++i) ans[i].assign(ss[i].size(), 0);
  long long ns = 0;
  for (int i = 0; i < t; ++i) {
    const string &s = ss[i];
    u64 *out = ans[i].data();
    auto t0 = chrono::steady_clock::now();
    {
      Solver sol;
      for (size_t k = 0; k < s.size(); ++k) out[k] = sol.push(s[k]);
      auto t1 = chrono::steady_clock::now();
      ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    }
  }

  string out;
  for (int i = 0; i < t; ++i) {
    const auto &a = ans[i];
    u64 h = 0;
    for (u64 x : a) h = h * 0x9E3779B97F4A7C15ull + x;
    out += to_string(a.size()) + ' ' + to_string(h);
    for (size_t k = 1; k <= a.size(); k *= 2) out += ' ' + to_string(a[k - 1]);
    out += ' ' + to_string(a.back()) + '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics(ns);
}
