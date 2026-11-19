// https://judge.yosupo.jp/problem/line_add_get_min
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<i64, 2>> &lines);  // {a, b} で y = ax + b
//     void insert(i64 a, i64 b);
//     i64 query(i64 p);
//   };
//
// 問いの x を先に使う提出は、Solver(lines, xs) のコンストラクタも持てる。xs は 1 p の p を問いの順に
// 並べたもので、持っていればハーネスはそちらを呼ぶ。座標を詰めた Li Chao 木 (LiChaoPoints) のように、
// 問いの点を前もって知って速くする形と、知らずに解く形を同じ表で比べるため。
//
// 最初の N 本を入れるところも計測区間に入れてある。クエリの `0 a b` と同じ
// 操作なので、外に出すと追加の半分だけを測らないことになる。入力の解析だけを
// 外に出す。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-convex-hull-trick.hpp"
#endif
#include SUBMISSION_HPP

template <class S, class L, class X> S make_solver(const L &lines, const X &xs) {
  if constexpr (std::is_constructible_v<S, const L &, const X &>) return S(lines, xs);
  else return S(lines);
}

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 2>> lines(n);
  for (auto &e : lines) must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);

  // 0 a b と 1 p で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
    if (e[0] == 0) must_scan(scanf("%lld", &e[2]), 1);
    else ++gets;
  }

  vector<i64> ans, xs;
  ans.reserve(gets), xs.reserve(gets);
  for (auto &e : qs)
    if (e[0] == 1) xs.push_back(e[1]);

  auto t0 = chrono::steady_clock::now();
  Solver s = make_solver<Solver>(lines, xs);
  for (auto &e : qs) {
    if (e[0] == 0) s.insert(e[1], e[2]);
    else ans.push_back(s.query(e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
