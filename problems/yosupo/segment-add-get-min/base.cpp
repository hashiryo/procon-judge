// https://judge.yosupo.jp/problem/segment_add_get_min
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<array<i64, 4>> &segs);  // {l, r, a, b} で x in [l, r) に y = ax + b
//     void insert(i64 l, i64 r, i64 a, i64 b);
//     i64 query(i64 p);  // p を覆う線分が無ければ LLONG_MAX
//   };
//
// 問いの x を先に使う提出は、Solver(segs, xs) のコンストラクタも持てる。xs は 1 p の p を問いの順に並べた
// もので、持っていればハーネスはそちらを呼ぶ (line_add_get_min と同じ)。
//
// 最初の N 本を入れるところも計測区間に入れる。クエリの `0 l r a b` と同じ操作なので、外に出すと追加の一部
// だけを測らないことになる。入力の解析と出力の整形は外に出す。
#include <climits>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-li-chao-tree.hpp"
#endif
#include SUBMISSION_HPP

template <class S, class L, class X> S make_solver(const L &segs, const X &xs) {
  if constexpr (std::is_constructible_v<S, const L &, const X &>) return S(segs, xs);
  else return S(segs);
}

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<array<i64, 4>> segs(n);
  for (auto &e : segs) must_scan(scanf("%lld %lld %lld %lld", &e[0], &e[1], &e[2], &e[3]), 4);
  // 0 l r a b と 1 p で項目数が違うので、種別を読んでから残りを読む。
  vector<array<i64, 5>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
    if (e[0] == 0) must_scan(scanf("%lld %lld %lld", &e[2], &e[3], &e[4]), 3);
    else ++gets;
  }

  vector<i64> ans, xs;
  ans.reserve(gets), xs.reserve(gets);
  for (auto &e : qs)
    if (e[0] == 1) xs.push_back(e[1]);

  auto t0 = chrono::steady_clock::now();
  Solver s = make_solver<Solver>(segs, xs);
  for (auto &e : qs) {
    if (e[0] == 0) s.insert(e[1], e[2], e[3], e[4]);
    else ans.push_back(s.query(e[1]));
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve(ans.size() * 12);
  for (i64 v : ans) {
    if (v == LLONG_MAX) out += "INFINITY";
    else out += to_string(v);
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
