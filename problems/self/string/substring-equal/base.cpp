// 自作の問題。1 つの文字列の 2 つの部分文字列が等しいかを答えるクエリを、文字列の族とクエリの組で測る。ローリングハッシュの
// 使い方の代表として作った。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);   // 前処理
//     bool equal(int a, int b, int len);  // s[a, a + len) と s[b, b + len) が等しいか
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに、文字列の作り方 (_shared/string/_common.hpp) とクエリの作り方を 1 行ずつ書く。
// 文字列とクエリはハーネスが計測の前に作る。文字は英小文字で、長さ n は 10^6 以下。クエリは 0 <= a, b、0 <= len、
// a + len <= n、b + len <= n を満たす。クエリの作り方は次の 2 つ。
//
//   mix q seed p      q 個のクエリを、それぞれ次の 3 つから選んで作る。どれも a と b を入れ替えることがある。
//                     1/4: a、b、len を一様に選ぶ (len は 0 から n - max(a, b))
//                     1/4: a と b を一様に選び、len を 64 以下にする
//                     1/2: p > 0 なら b - a を p の倍数にし、len を 2 の冪ごとにほぼ一様な大きさで選ぶ。p = 0 なら
//                          a と b を一様に選び、len を同じように選ぶ
//   aligned q seed k  長さ 2^k の塊の境から始まる 2 つの塊を比べる (a と b は 2^k の倍数、len = 2^k)。Thue–Morse 列では、
//                     長さ 2^k の塊は T_k かその a と b を入れ替えたものなので、半分ほどが等しくない組になる。k >= 11 なら、
//                     法 2^64 の多項式のハッシュはどの基数でもこの組を区別できない
//
// ブロックごとに、構築と q 個のクエリを測って足す。s は計測区間のあとまで生きているので、提出は参照を持ってよい。出力は
// ブロックごとに答えを 0 と 1 の文字で並べ、64 個ごとに改行する。
#include "pj.hpp"
#include "_shared/string/_common.hpp"
#include <algorithm>
#include <bit>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace {
struct Query {
  int a, b, len;
};

// 0 以上 m 以下の長さを、2 の冪ごとにほぼ一様な大きさで選ぶ。
int log_len(string_family::SplitMix64 &rng, int m) {
  const int e = (int)rng.below(std::bit_width((unsigned)m) + 1);
  return (int)rng.below((u64)std::min(m, (1 << e) - 1) + 1);
}

vector<Query> make_queries(int n) {
  char mode[16];
  unsigned long long q, seed, p;
  must_scan(scanf("%15s %llu %llu %llu", mode, &q, &seed, &p), 4);
  string_family::SplitMix64 rng(seed);
  vector<Query> qs(q);
  if (!strcmp(mode, "mix")) {
    for (auto &e : qs) {
      const u64 r = rng.below(4);
      int a, b, len;
      if (r < 2 || p == 0 || p >= (u64)n) {
        a = (int)rng.below(n + 1), b = (int)rng.below(n + 1);
        const int m = n - std::max(a, b);
        len = r == 0 ? (int)rng.below(m + 1) : r == 1 ? (int)rng.below(std::min(m, 64) + 1) : log_len(rng, m);
      } else {
        const int d = (int)p * (1 + (int)rng.below((n - 1) / p));
        a = (int)rng.below(n - d + 1), b = a + d;
        len = log_len(rng, n - b);
      }
      if (rng.below(2)) swap(a, b);
      e = {a, b, len};
    }
  } else if (!strcmp(mode, "aligned")) {
    const int w = 1 << p, nb = n >> p;
    for (auto &e : qs) e = nb ? Query{w * (int)rng.below(nb), w * (int)rng.below(nb), w} : Query{0, 0, 0};
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
    qss[i] = make_queries((int)ss[i].size());
  }

  vector<vector<char>> ans(t);
  for (int i = 0; i < t; ++i) ans[i].assign(qss[i].size(), 0);
  long long ns = 0;
  for (int i = 0; i < t; ++i) {
    const auto &qs = qss[i];
    char *out = ans[i].data();
    auto t0 = chrono::steady_clock::now();
    {
      Solver sol(ss[i]);
      for (size_t k = 0; k < qs.size(); ++k) out[k] = sol.equal(qs[k].a, qs[k].b, qs[k].len);
      auto t1 = chrono::steady_clock::now();
      ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    }
  }

  string out;
  for (int i = 0; i < t; ++i) {
    for (size_t k = 0; k < ans[i].size(); ++k) {
      out += char('0' + ans[i][k]);
      if (k % 64 == 63 || k + 1 == ans[i].size()) out += '\n';
    }
    if (ans[i].empty()) out += "-\n";
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics(ns);
}
