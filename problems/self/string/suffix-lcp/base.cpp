// 自作の問題。1 つの文字列の 2 つの接尾辞の最長共通接頭辞 (LCP) の長さを答えるクエリを、文字列の族とクエリの組で測る。
// 部分文字列の LCP と大小の比べを答える構造の代表として作った。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);  // 前処理
//     int lcp(int i, int j);             // s[i, n) と s[j, n) の LCP の長さ
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに、文字列の作り方 (_shared/string/_common.hpp) とクエリの作り方を 1 行ずつ書く。
// 文字列とクエリはハーネスが計測の前に作る。文字は英小文字で、長さ n は 1 以上 10^6 以下。クエリは 0 <= i, j < n を満たし、
// i = j のこともある。クエリの作り方は次の 2 つ。
//
//   mix q seed p      q 個のクエリを、それぞれ次の 3 つから選んで作る。どれも i と j を入れ替えることがある。
//                     3/8: i と j を一様に選ぶ
//                     1/8: i を一様に選び、j = i とする
//                     1/2: p > 0 なら j - i を p の倍数にする。p = 0 なら i と j を一様に選ぶ
//   aligned q seed k  i と j を 2^k の倍数から一様に選ぶ
//
// ブロックごとに、構築と q 個のクエリを測って足す。s は計測区間のあとまで生きているので、提出は参照を持ってよい。出力は
// ブロックごとに答えを 32 個ずつ 1 行に並べる。
#include "pj.hpp"
#include "_shared/string/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace {
struct Query {
  int i, j;
};

vector<Query> make_queries(int n) {
  char mode[16];
  unsigned long long q, seed, p;
  must_scan(scanf("%15s %llu %llu %llu", mode, &q, &seed, &p), 4);
  string_family::SplitMix64 rng(seed);
  vector<Query> qs(q);
  if (!strcmp(mode, "mix")) {
    for (auto &e : qs) {
      const u64 r = rng.below(8);
      int i, j;
      if (r == 3) {
        i = j = (int)rng.below(n);
      } else if (r < 3 || p == 0 || p >= (u64)n) {
        i = (int)rng.below(n), j = (int)rng.below(n);
      } else {
        const int d = (int)p * (1 + (int)rng.below((n - 1) / p));
        i = (int)rng.below(n - d), j = i + d;
      }
      if (rng.below(2)) swap(i, j);
      e = {i, j};
    }
  } else if (!strcmp(mode, "aligned")) {
    const int nb = (n - 1) >> p;
    for (auto &e : qs) e = {(int)rng.below(nb + 1) << p, (int)rng.below(nb + 1) << p};
  } else {
    fprintf(stderr, "unknown query mode %s\n", mode);
    exit(1);
  }
  return qs;
}
}  // namespace

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<string> ss(t);
  vector<vector<Query>> qss(t);
  for (int i = 0; i < t; ++i) {
    ss[i] = string_family::read_string();
    if (ss[i].empty()) {
      fprintf(stderr, "empty string\n");
      return 1;
    }
    qss[i] = make_queries((int)ss[i].size());
  }

  vector<vector<int>> ans(t);
  for (int i = 0; i < t; ++i) ans[i].assign(qss[i].size(), 0);
  long long ns = 0;
  for (int i = 0; i < t; ++i) {
    const auto &qs = qss[i];
    int *out = ans[i].data();
    auto t0 = chrono::steady_clock::now();
    {
      Solver sol(ss[i]);
      for (size_t k = 0; k < qs.size(); ++k) out[k] = sol.lcp(qs[k].i, qs[k].j);
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
