// https://onlinejudge.u-aizu.ac.jp/courses/lesson/8/ITP2/all/ITP2_2_D
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);        // 空の列を n 本。計測区間の外。
//     void push_back(int t, i64 x);  // t 番目の末尾に x を足す
//     vector<i64> dump(int t);       // t 番目の中身を返す
//     void concat(int s, int t);     // t 番目の後ろに s 番目を繋いで s を空にする
//   };
//
// dump は出力するクエリの本体なので計測区間の中に置く。構築は空の列を並べる
// だけなので外。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-rbst.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);

  // 0 t x は 3 項目、1 t は 2 項目、2 s t は 3 項目。
  vector<array<i64, 3>> qs(q);
  int gets = 0;
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld", &e[0], &e[1]), 2);
    if (e[0] == 1) ++gets;
    else must_scan(scanf("%lld", &e[2]), 1);
  }

  Solver s(n);
  vector<vector<i64>> outs;
  outs.reserve(gets);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e[0] == 0) s.push_back((int)e[1], e[2]);
    else if (e[0] == 1) outs.push_back(s.dump((int)e[1]));
    else s.concat((int)e[1], (int)e[2]);
  }
  auto t1 = chrono::steady_clock::now();

  // 空の列は空行になるので、print_all ではなく自分で組む。
  string out;
  for (auto &v : outs) {
    for (size_t i = 0; i < v.size(); ++i) {
      if (i) out += ' ';
      out += to_string(v[i]);
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
