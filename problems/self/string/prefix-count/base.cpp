// 自作の問題。文字列の多重集合への追加と削除と、ある文字列で始まる要素の数を答えるクエリを、文字列の作り方と操作の列で測る。
// トライの使い方の代表として作った。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver();
//     void insert(const string &s);       // s を 1 つ足す
//     void erase(const string &s);        // s を 1 つ除く。s は今の多重集合にある
//     int count_prefix(const string &p);  // 今の多重集合の要素のうち、p で始まるものの数 (重複も数える)。p は空のこともある
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに「作り方 ops seed σ lo hi」を 1 行書き、ハーネスが計測の前に操作の列を作る。
// 文字は英小文字の先頭の σ 種類。操作はそれぞれ次から選ぶ。
//   5/10: 新しい文字列を作って足す
//   1/10: 今の多重集合から一様に選んだ要素を除く (空なら新しい文字列を足す)
//   4/10: count_prefix。半分は、これまでに足した文字列から一様に選んだものの、長さを 0 からその長さまで一様に選んだ接頭辞。
//         残りの半分は新しく作った文字列の、同じように選んだ接頭辞
// 新しい文字列の作り方は次の 2 つ。どちらも長さは lo 以上 hi 以下で一様に選ぶ。
//   words  文字を一様に選ぶ
//   tree   これまでに足した文字列から一様に選んだものの、一様な長さの接頭辞を頭に置き、残りの文字を一様に選ぶ
//          (接頭辞が選んだ長さより長ければ切る)。最初の文字列は words と同じ
//
// ブロックごとに、構築とすべての操作を測って足す。出力はブロックごとに count_prefix の答えを 32 個ずつ 1 行に並べる
// (count_prefix が無いブロックは -)。
#include "pj.hpp"
#include "_shared/string/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace {
struct Op {
  int type;  // 0: insert, 1: erase, 2: count_prefix
  string s;
};

vector<Op> make_ops() {
  char mode[16];
  unsigned long long ops, seed, sigma, lo, hi;
  must_scan(scanf("%15s %llu %llu %llu %llu %llu", mode, &ops, &seed, &sigma, &lo, &hi), 6);
  const bool tree = !strcmp(mode, "tree");
  if ((!tree && strcmp(mode, "words")) || sigma < 1 || sigma > 26 || lo < 1 || lo > hi) {
    fprintf(stderr, "bad block spec\n");
    exit(1);
  }
  string_family::SplitMix64 rng(seed);
  vector<string> hist;     // これまでに足した文字列
  vector<int> present;     // 今の多重集合の要素 (hist の番号)
  auto fresh = [&]() {
    const size_t len = lo + rng.below(hi - lo + 1);
    string w;
    if (tree && !hist.empty()) {
      const string &b = hist[rng.below(hist.size())];
      w = b.substr(0, min(len, (size_t)rng.below(b.size() + 1)));
    }
    while (w.size() < len) w += char('a' + rng.below(sigma));
    return w;
  };
  vector<Op> res(ops);
  for (auto &op : res) {
    const u64 r = rng.below(10);
    if (r < 5 || (r == 5 && present.empty())) {
      op = {0, fresh()};
      present.push_back((int)hist.size());
      hist.push_back(op.s);
    } else if (r == 5) {
      const size_t k = rng.below(present.size());
      op = {1, hist[present[k]]};
      present[k] = present.back(), present.pop_back();
    } else {
      string w = !hist.empty() && rng.below(2) ? hist[rng.below(hist.size())] : fresh();
      w.resize(rng.below(w.size() + 1));
      op = {2, w};
    }
  }
  return res;
}
}  // namespace

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<vector<Op>> blocks(t);
  for (auto &b : blocks) b = make_ops();

  vector<vector<int>> ans(t);
  for (int i = 0; i < t; ++i) {
    size_t q = 0;
    for (auto &op : blocks[i]) q += op.type == 2;
    ans[i].assign(q, 0);
  }
  long long ns = 0;
  for (int i = 0; i < t; ++i) {
    int *out = ans[i].data();
    auto t0 = chrono::steady_clock::now();
    {
      Solver sol;
      for (const auto &op : blocks[i]) {
        if (op.type == 0) sol.insert(op.s);
        else if (op.type == 1) sol.erase(op.s);
        else *out++ = sol.count_prefix(op.s);
      }
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
