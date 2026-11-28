// https://judge.yosupo.jp/problem/division_of_big_integers
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const string &a, const string &b);  // 10 進の文字列。構築。計測区間の外。
//     void run();                                // q = a / b と r = a - q b を作る。ここだけ測る。
//     string quotient() const;                   // 商の 10 進。整形は計測区間の外。
//     string remainder() const;                  // 余りの 10 進。
//   };
//
// 入出力は 10 進の文字列で渡す。内部表現を提出が選べるようにするためで、変換は O(桁) なので
// 一緒に測ると基数の選び方で乗り方が変わる。構築と取り出しを分けてあるのはそのため。
// テストは 1 ファイルに T 個の組が入る (総文字数が 4,000,002 まで)。構築を全部済ませてから、
// 割り算だけを続けて測る。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<string> as(t), bs(t);
  for (int i = 0; i < t; ++i) as[i] = read_token(), bs[i] = read_token();

  vector<Solver> sols;
  sols.reserve(t);
  for (int i = 0; i < t; ++i) sols.emplace_back(as[i], bs[i]);

  auto t0 = chrono::steady_clock::now();
  for (auto &s : sols) s.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(size_t(4) << 20);  // 入力と同じくらい (総文字数 400 万まで)
  for (auto &s : sols) {
    out += s.quotient();
    out += ' ';
    out += s.remainder();
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
