// https://onlinejudge.u-aizu.ac.jp/challenges/sources/VPC/HUPC/2842
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int h, int w, const vector<array<int, 5>> &events);
//     void run();                                   // ここだけ測る。
//     const vector<array<i64, 2>> &answer() const;  // 数えるたびに {焼けた, 焼き中}
//   };
//
// events は {種別, h1, w1, h2, w2} で、時刻の昇順に並べてある。種別は
//   -1 = 焼き上がる, 0 = 焼き始める, 1 = 食べる, 2 = 数える。
// 座標は入力のまま 1-indexed で渡す。
//
// 焼き上がりを T 後の出来事として足して時刻で並べ替えるところは、どの実装
// でも同じ入力の整理なので、ハーネスでやる。
// 提出が何を include するかに頼らないよう、ハーネスが使うものは自分で引く。
#include <algorithm>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-bit-2d.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int h, w, tt, q;
  must_scan(scanf("%d %d %d %d", &h, &w, &tt, &q), 4);

  // 並べ替えの鍵に時刻を持つので、いったん 6 項目で持つ。
  vector<array<int, 6>> raw;
  raw.reserve((size_t)q * 2);
  for (int i = 0; i < q; ++i) {
    int t, c, h1, w1, h2 = 0, w2 = 0;
    must_scan(scanf("%d %d %d %d", &t, &c, &h1, &w1), 4);
    if (c == 2) must_scan(scanf("%d %d", &h2, &w2), 2);
    raw.push_back({t, c, h1, w1, h2, w2});
    if (c == 0) raw.push_back({t + tt, -1, h1, w1, h2, w2});
  }
  std::sort(raw.begin(), raw.end());

  vector<array<int, 5>> events;
  events.reserve(raw.size());
  int gets = 0;
  for (auto &e : raw) {
    events.push_back({e[1], e[2], e[3], e[4], e[5]});
    if (e[1] == 2) ++gets;
  }

  Solver sol(h, w, events);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve((size_t)gets * 14);
  for (auto &a : sol.answer()) {
    out += to_string(a[0]);
    out += ' ';
    out += to_string(a[1]);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
