// 永続 Li Chao 木 (線分追加・最小) のハーネス。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int q);
//     void insert(int v, i64 l, i64 r, i64 a, i64 b);  // 版 v から新しい版を作り、[l,r) に y = a x + b を足す
//     i64 query(int v, i64 x);                         // 版 v の x での最小。無ければ LLONG_MAX
//   };
//
// 版 0 は空。type 0 のクエリは、現れる順に版 1, 2, ... を作る。v は 0 以上、その時点の版の数未満です。
// 座標は -1000000 以上 1000000 以下、a, b は -10^9 以上 10^9 以下。
// 構築は計測区間の外に置く。クエリは 1 つずつ渡し、その場で答えさせる。
#include <climits>
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-neo.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  vector<int> type(q);
  vector<array<i64, 6>> ops(q);
  for (int i = 0; i < q; ++i) {
    int t;
    must_scan(scanf("%d", &t), 1);
    type[i] = t;
    if (t == 0) {
      i64 v, l, r, a, b;
      must_scan(scanf("%lld %lld %lld %lld %lld", &v, &l, &r, &a, &b), 5);
      ops[i] = {v, l, r, a, b, 0};
    } else {
      i64 v, x;
      must_scan(scanf("%lld %lld", &v, &x), 2);
      ops[i] = {v, x, 0, 0, 0, 0};
    }
  }

  vector<i64> ans;
  ans.reserve(q);

  Solver s(q);

  auto t0 = chrono::steady_clock::now();
  for (int i = 0; i < q; ++i) {
    if (type[i] == 0) s.insert(int(ops[i][0]), ops[i][1], ops[i][2], ops[i][3], ops[i][4]);
    else ans.push_back(s.query(int(ops[i][0]), ops[i][1]));
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

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
