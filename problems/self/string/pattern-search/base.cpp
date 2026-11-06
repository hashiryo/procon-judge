// 自作の問題。テキストの中のパターンの出現位置をすべて求める速さを、テキストとパターンの組で測る。KMP、Z、Boyer–Moore 系、
// 2 方向の照合 (two-way)、SIMD の候補の絞り込みなどが同じ土俵に乗る。設計は algo-notes の notes/string-basics.md。
//
// 提出は次を実装する (aoj-ALDS1_14_B と同じ形)。
//   struct Solver {
//     Solver(const string &t, const string &p);  // 構築。計測区間の外。
//     void run();                                // 探索。ここだけ測る。
//     const vector<int> &answer() const;         // t[i, i + |p|) = p となる i を昇順に
//   };
//
// 入力の 1 行目はブロックの数 T。ブロックごとに、テキストの作り方 (_shared/string/_common.hpp) とパターンの作り方を 1 行ずつ
// 書く。文字列はハーネスが計測の前に作る。文字は英小文字で、テキストの長さは 1 以上 10^6 以下、パターンの長さは 1 以上で
// テキスト以下。パターンの作り方は次の 3 つ。
//
//   族 m seed x y z  テキストと同じ作り方の、長さ m の文字列
//   sub pos len      テキストの t[pos, pos + len)
//   subx pos len     テキストの t[pos, pos + len) の最後の文字を、英小文字の次の文字 (z の次は a) に替えたもの
//
// ブロックごとに run() を測って足す。t と p は計測区間のあとまで生きているので、提出は参照を持ってよい。出力はブロックごとに、
// 出現の数の行と、出現位置を 32 個ずつ並べた行。
#include "pj.hpp"
#include "_shared/string/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace {
string make_pattern(const string &t) {
  char kind[16];
  must_scan(scanf("%15s", kind), 1);
  string p;
  if (!strcmp(kind, "sub") || !strcmp(kind, "subx")) {
    unsigned long long pos, len;
    must_scan(scanf("%llu %llu", &pos, &len), 2);
    if (pos + len > t.size()) {
      fprintf(stderr, "pattern out of range\n");
      exit(1);
    }
    p = t.substr(pos, len);
    if (kind[3] == 'x' && !p.empty()) p.back() = char('a' + (p.back() - 'a' + 1) % 26);
  } else {
    unsigned long long m, seed, x, y, z;
    must_scan(scanf("%llu %llu %llu %llu %llu", &m, &seed, &x, &y, &z), 5);
    p = string_family::make_string(kind, (size_t)m, seed, x, y, z);
  }
  if (p.empty() || p.size() > t.size()) {
    fprintf(stderr, "bad pattern length %zu\n", p.size());
    exit(1);
  }
  return p;
}
}  // namespace

signed main() {
  int nb;
  must_scan(scanf("%d", &nb), 1);
  vector<string> ts(nb), ps(nb);
  for (int i = 0; i < nb; ++i) {
    ts[i] = string_family::read_string();
    ps[i] = make_pattern(ts[i]);
  }

  long long ns = 0;
  string out;
  for (int i = 0; i < nb; ++i) {
    Solver sol(ts[i], ps[i]);
    auto t0 = chrono::steady_clock::now();
    sol.run();
    auto t1 = chrono::steady_clock::now();
    ns += chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    const vector<int> &pos = sol.answer();
    out += to_string(pos.size());
    out += '\n';
    for (size_t k = 0; k < pos.size(); ++k) {
      out += to_string(pos[k]);
      out += k % 32 == 31 || k + 1 == pos.size() ? '\n' : ' ';
    }
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics(ns);
}
