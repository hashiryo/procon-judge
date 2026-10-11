// https://judge.yosupo.jp/problem/runenumerate
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const string &s);             // 構築。文字列を写すだけで、計測区間の外。
//     void run();                                   // すべての run を求める。ここだけ測る。
//     const vector<array<int, 3>> &answer() const;  // (最小の周期 t, 始まり l, 終わり r) の昇順の列。整形は外。
//   };
//
// 1 回の計算で答えが出る問題なので、構築 / 計算 / 取り出しの 3 段に分ける (yosupo-suffixarray と同じ形)。run が見つかる順は方法に
// よって違い、並べ方も実装の一部なので、並べる処理も run の中に置く。設計は algo-notes の notes/runs.md。
#include <array>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  string s = read_token();

  Solver sol(s);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<array<int, 3>> &r = sol.answer();
  string out = to_string(r.size()) + '\n';
  for (const auto &[t, l, rr] : r) out += to_string(t) + ' ' + to_string(l) + ' ' + to_string(rr) + '\n';
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
