// https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);                              // 構築。計測区間の外。
//     int unite(int u, int v, const array<i64, 4> &x);     // a[u] = a[v] x。
//                                                          // 矛盾しなければ 1
//     bool diff(int u, int v, array<i64, 4> &out);         // a[v]^-1 a[u]。
//                                                          // 定まれば true
//   };
//
// 行列は {x00, x01, x10, x11} の順に並べる。剰余は 998244353 で、行列式は 1 が
// 保証されている。
//
// 出力の項目数がクエリの種別で変わるので、計測区間では値だけを溜めて、行の
// 組み立ては外でやる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-union-find.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);

  // 0 u v x00 x01 x10 x11 と 1 u v で項目数が違う。
  vector<array<i64, 7>> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%lld %lld %lld", &e[0], &e[1], &e[2]), 3);
    if (e[0] == 0)
      must_scan(scanf("%lld %lld %lld %lld", &e[3], &e[4], &e[5], &e[6]), 4);
  }

  Solver s(n);
  vector<array<i64, 4>> vals(q);
  vector<int> width(q);  // その行に出す項目数 (1 か 4)

  auto t0 = chrono::steady_clock::now();
  for (int i = 0; i < q; ++i) {
    auto &e = qs[i];
    if (e[0] == 0) {
      array<i64, 4> x{e[3], e[4], e[5], e[6]};
      vals[i][0] = s.unite((int)e[1], (int)e[2], x);
      width[i] = 1;
    } else if (s.diff((int)e[1], (int)e[2], vals[i])) {
      width[i] = 4;
    } else {
      vals[i][0] = -1;
      width[i] = 1;
    }
  }
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve((size_t)q * 16);
  for (int i = 0; i < q; ++i) {
    for (int j = 0; j < width[i]; ++j) {
      if (j) out += ' ';
      out += to_string(vals[i][j]);
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
