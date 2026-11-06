// 自作の問題。1 つのテキストの中の、多数のパターンそれぞれの出現回数を答えるクエリを、テキストの族とパターンの組で測る。
// 接尾辞オートマトンの使い方の代表として作った。接尾辞配列の上の二分探索や FM-index も同じ土俵に乗る。設計は algo-notes の
// notes/string-basics.md。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);  // 前処理
//     int count(const string &p);        // s[i, i + |p|) = p となる i の数。p は空でない
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに、テキストの作り方 (_shared/string/_common.hpp) と「mix q seed L σ」を 1 行ずつ
// 書く。文字列はハーネスが計測の前に作る。文字は英小文字で、テキストの長さ n は 1 以上 5 * 10^5 以下。q 個のパターンの長さは、
// e を 0 から bit_width(L) - 1 で一様に選んで、[2^e, 2^(e + 1)) から一様に選ぶ (L を超えたら L にする)。パターンは次から選ぶ。
//   1/2: テキストの一様な位置から始まる部分文字列 (長さが n を超えたら n にする)
//   1/4: 先頭の σ 種類の英小文字を一様に並べたもの
//   1/4: 1 つ目と同じ部分文字列の、一様な 1 か所を先頭の σ 種類の英小文字の一様な 1 文字に替えたもの
//
// ブロックごとに、構築と q 個のクエリを測って足す。s は計測区間のあとまで生きているので、提出は参照を持ってよい。出力は
// ブロックごとに答えを 32 個ずつ 1 行に並べる。
#include "pj.hpp"
#include "_shared/string/_common.hpp"
#include <bit>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace {
vector<string> make_patterns(const string &s) {
  char mode[16];
  unsigned long long q, seed, L, sigma;
  must_scan(scanf("%15s %llu %llu %llu %llu", mode, &q, &seed, &L, &sigma), 5);
  if (strcmp(mode, "mix") || L < 1 || sigma < 1 || sigma > 26) {
    fprintf(stderr, "bad query spec\n");
    exit(1);
  }
  string_family::SplitMix64 rng(seed);
  const size_t n = s.size();
  vector<string> ps(q);
  for (auto &p : ps) {
    const int e = (int)rng.below(std::bit_width(L));
    size_t len = std::min<size_t>(L, ((size_t)1 << e) + rng.below((u64)1 << e));
    const u64 r = rng.below(4);
    if (r == 2) {
      p.resize(len);
      for (auto &c : p) c = char('a' + rng.below(sigma));
    } else {
      len = std::min(len, n);
      p = s.substr(rng.below(n - len + 1), len);
      if (r == 3) p[rng.below(len)] = char('a' + rng.below(sigma));
    }
  }
  return ps;
}
}  // namespace

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<string> ss(t);
  vector<vector<string>> pss(t);
  for (int i = 0; i < t; ++i) {
    ss[i] = string_family::read_string();
    if (ss[i].empty()) {
      fprintf(stderr, "empty string\n");
      return 1;
    }
    pss[i] = make_patterns(ss[i]);
  }

  vector<vector<int>> ans(t);
  for (int i = 0; i < t; ++i) ans[i].assign(pss[i].size(), 0);
  long long ns = 0;
  for (int i = 0; i < t; ++i) {
    const auto &ps = pss[i];
    int *out = ans[i].data();
    auto t0 = chrono::steady_clock::now();
    {
      Solver sol(ss[i]);
      for (size_t k = 0; k < ps.size(); ++k) out[k] = sol.count(ps[k]);
      auto t1 = chrono::steady_clock::now();
      ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    }
  }

  string out;
  for (int i = 0; i < t; ++i) {
    for (size_t k = 0; k < ans[i].size(); ++k) {
      out += to_string(ans[i][k]);
      out += k % 32 == 31 || k + 1 == ans[i].size() ? '\n' : ' ';
    }
    if (ans[i].empty()) out += "-\n";
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics(ns);
}
