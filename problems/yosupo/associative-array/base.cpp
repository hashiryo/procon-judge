// https://judge.yosupo.jp/problem/associative_array
//
// 提出は次を実装する。
//   struct Solver {
//     Solver();                // 空の表を作る。計測区間の中。
//     void set(u64 k, u64 v);  // a[k] = v
//     u64 get(u64 k);          // a[k]。set したことの無い k なら 0
//   };
//
// 表を作るところとクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。ライブラリとして
// 使うのもこの形なので、先のクエリを読んでまとめて処理する実装 (キーを先に集めて座標圧縮するなど) はここでは
// 比べない。クエリの数も構築に渡さない。表の大きさを先に決められると、表の伸ばし方の差が測りに乗らないため。
// k と v は 10^18 以下。設計は algo-notes の notes/associative-containers.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/std_umap.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  // 1 クエリを 16 byte で持つ。v は 10^18 以下なので、get は v = ~0 で表す。
  vector<array<u64, 2>> qs(q);
  int ngets = 0;
  for (auto &e : qs) {
    int t;
    must_scan(scanf("%d %llu", &t, &e[0]), 2);
    if (t == 0) must_scan(scanf("%llu", &e[1]), 1);
    else e[1] = ~0ull, ++ngets;
  }

  vector<u64> ans;
  ans.reserve(ngets);

  auto t0 = chrono::steady_clock::now();
  Solver s;
  for (auto &e : qs) {
    if (e[1] == ~0ull) ans.push_back(s.get(e[0]));
    else s.set(e[0], e[1]);
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
