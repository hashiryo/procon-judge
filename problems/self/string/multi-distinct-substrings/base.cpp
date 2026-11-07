// 自作の問題。複数の文字列のどれかに現れる、異なる (空でない) 部分文字列の数を答える (洛谷 P6139 と同じ問い方)。広義の接尾辞
// オートマトンの使い方の代表として作った。区切り文字でつないだ文字列の接尾辞配列でも解ける。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する (yosupo-zalgorithm と同じ 3 段の形)。
//   struct Solver {
//     explicit Solver(const vector<string> &ss);  // 構築。計測区間の外。
//     void run();                                  // 数える。ここだけ測る。
//     u64 answer() const;
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに「作り方 k seed σ lo hi」を 1 行書き、ハーネスが計測の前に k 本の文字列を作る。文字は
// 英小文字の先頭の σ 種類で、どの文字列も長さは lo 以上 hi 以下で一様に選ぶ (1 <= lo)。作り方は次の 3 つ。
//   words  文字を一様に選ぶ
//   tree   これまでに作った文字列から一様に選んだものの、一様な長さの接頭辞を頭に置き、残りの文字を一様に選ぶ (最初の文字列は words と同じ)
//   same   a だけの文字列 (σ は使わない)
// ブロックごとに run() を測って足す。ss は計測区間のあとまで生きているので、提出は参照を持ってよい。出力はブロックごとに答えを 1 行。
#include "pj.hpp"
#include "_shared/string/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace {
vector<string> make_strings() {
  char mode[16];
  unsigned long long k, seed, sigma, lo, hi;
  must_scan(scanf("%15s %llu %llu %llu %llu %llu", mode, &k, &seed, &sigma, &lo, &hi), 6);
  const bool tree = !strcmp(mode, "tree"), same = !strcmp(mode, "same");
  if ((!tree && !same && strcmp(mode, "words")) || sigma < 1 || sigma > 26 || lo < 1 || lo > hi) {
    fprintf(stderr, "bad block spec\n");
    exit(1);
  }
  string_family::SplitMix64 rng(seed);
  vector<string> ss(k);
  for (size_t i = 0; i < k; ++i) {
    const size_t len = lo + rng.below(hi - lo + 1);
    string w;
    if (same) w.assign(len, 'a');
    else {
      if (tree && i > 0) {
        const string &b = ss[rng.below(i)];
        w = b.substr(0, min(len, (size_t)rng.below(b.size() + 1)));
      }
      while (w.size() < len) w += char('a' + rng.below(sigma));
    }
    ss[i] = move(w);
  }
  return ss;
}
}  // namespace

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<vector<string>> blocks(t);
  for (auto &b : blocks) b = make_strings();

  long long ns = 0;
  string out;
  for (int i = 0; i < t; ++i) {
    Solver sol(blocks[i]);
    auto t0 = chrono::steady_clock::now();
    sol.run();
    auto t1 = chrono::steady_clock::now();
    ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    out += to_string(sol.answer()) + '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics(ns);
}
