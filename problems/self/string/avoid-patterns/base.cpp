// 自作の問題。パターンの集合と長さ n を受け取り、英小文字の先頭の σ 種類で作る長さ n の文字列のうち、どのパターンも部分文字列として
// 含まないものの数を 998244353 で割った余りで答える。オートマトンの上の DP (Aho–Corasick のオートマトンで状態を持ち、1 文字ずつ伸ばす)
// の使い方の代表として作った。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<string> &ps, int sigma);  // ps はパターンの列 (どれも空でなく、先頭の sigma 種類の英小文字だけ)
//     u32 count(int n);                             // 長さ n の文字列のうち、どのパターンも含まないものの数 mod 998244353
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに「n σ G」を 1 行書き、続く G 行にパターンの作り方を 1 行ずつ書く。パターンはハーネスが
// 計測の前に作り、G 行の分を順につなげた列を ps にする。1 <= n、1 <= σ <= 26。作り方の行は次の 4 つで、どれも k 本のパターンを作り、
// 長さは lo 以上 hi 以下で一様に選ぶ (1 <= lo <= hi)。
//   words k seed lo hi                     先頭の σ 種類の英小文字を一様に並べたもの
//   tree k seed lo hi                      この行で作ったパターンから一様に選んだものの、一様な長さの接頭辞を頭に置き、残りを words と
//                                          同じく選んだもの (1 本目は words と同じ)
//   sub k seed lo hi 族 N seed x y z       テキスト (_shared/string/_common.hpp の作り方の 1 行) の一様な位置から始まる部分文字列 (長さが
//                                          テキストの長さを超えたらテキストの長さにする)
//   subx k seed lo hi 族 N seed x y z      sub と同じ部分文字列の、一様な 1 か所を先頭の σ 種類の英小文字の一様な 1 文字に替えたもの
// sub と subx のテキストは空でなく、σ 種類に入らない文字を含まないものにする。
//
// ブロックごとに、Solver の構築と count(n) と後始末を測って足す。ps は計測区間のあとまで生きているので、提出は参照を持ってよい。出力は
// ブロックごとに答えを 1 行。
#include "pj.hpp"
#include "_shared/string/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace {
struct Block {
  int n, sigma;
  vector<string> ps;
};

[[noreturn]] void bad_spec(const char *what) {
  fprintf(stderr, "bad block spec: %s\n", what);
  exit(1);
}

void add_group(Block &b) {
  char mode[16];
  unsigned long long k, seed, lo, hi;
  must_scan(scanf("%15s %llu %llu %llu %llu", mode, &k, &seed, &lo, &hi), 5);
  const bool words = !strcmp(mode, "words"), tree = !strcmp(mode, "tree"), sub = !strcmp(mode, "sub"), subx = !strcmp(mode, "subx");
  if (!words && !tree && !sub && !subx) bad_spec("mode");
  if (lo < 1 || lo > hi) bad_spec("lo, hi");
  string text;
  if (sub || subx) {
    text = string_family::read_string();
    if (text.empty()) bad_spec("empty text");
  }
  string_family::SplitMix64 rng(seed);
  const size_t first = b.ps.size();
  for (size_t i = 0; i < k; ++i) {
    size_t len = lo + rng.below(hi - lo + 1);
    string p;
    if (sub || subx) {
      len = min(len, text.size());
      p = text.substr(rng.below(text.size() - len + 1), len);
      if (subx) p[rng.below(len)] = char('a' + rng.below(b.sigma));
    } else {
      if (tree && i > 0) {
        const string &base = b.ps[first + rng.below(i)];
        p = base.substr(0, min(len, (size_t)rng.below(base.size() + 1)));
      }
      while (p.size() < len) p += char('a' + rng.below(b.sigma));
    }
    b.ps.push_back(std::move(p));
  }
}

Block read_block() {
  Block b;
  int g;
  must_scan(scanf("%d %d %d", &b.n, &b.sigma, &g), 3);
  if (b.n < 1 || b.sigma < 1 || b.sigma > 26 || g < 1) bad_spec("n, sigma, G");
  for (int i = 0; i < g; ++i) add_group(b);
  for (const auto &p : b.ps)
    for (const char c : p)
      if (c < 'a' || c >= 'a' + b.sigma) bad_spec("pattern letter out of sigma");
  return b;
}
}  // namespace

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<Block> blocks(t);
  for (auto &b : blocks) b = read_block();

  vector<u32> ans(t);
  long long ns = 0;
  for (int i = 0; i < t; ++i) {
    auto t0 = chrono::steady_clock::now();
    {
      Solver sol(blocks[i].ps, blocks[i].sigma);
      ans[i] = sol.count(blocks[i].n);
    }
    auto t1 = chrono::steady_clock::now();
    ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
  }
  print_all(ans);
  report_metrics(ns);
}
