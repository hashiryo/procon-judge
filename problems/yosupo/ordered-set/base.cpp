// https://judge.yosupo.jp/problem/ordered_set
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<int> &a);  // a の要素からなる集合を作る。a は狭義単調増加。計測区間の中。
//     void insert(int x);                      // x を入れる。既にあれば何もしない
//     void erase(int x);                       // x を除く。無ければ何もしない
//     int kth(int k);                          // 小さい方から k 番目 (0 始まり) の要素。要素が k 個以下なら -1
//     int count_le(int x);                     // x 以下の要素の個数
//     int prev(int x);                         // x 以下で最大の要素。無ければ -1
//     int next(int x);                         // x 以上で最小の要素。無ければ -1
//   };
//
// 初期集合の構築とクエリの両方を計測区間に入れる。クエリは 1 つずつ渡し、その場で答えさせる。先のクエリを読んで
// まとめて処理する実装 (値を先に集めて座標圧縮するなど) はここでは比べない。要素と x は 0 以上 10^9 以下。入力の
// k 番目のクエリは 1 始まりなので、ハーネスが 1 を引いて渡す。設計は algo-notes の notes/associative-containers.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/pbds_tree.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  vector<array<int, 2>> qs(q);
  int nout = 0;
  for (auto &e : qs) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    nout += e[0] >= 2;
  }

  vector<int> ans;
  ans.reserve(nout);

  auto t0 = chrono::steady_clock::now();
  Solver s(a);
  for (auto &[t, x] : qs) {
    switch (t) {
      case 0: s.insert(x); break;
      case 1: s.erase(x); break;
      case 2: ans.push_back(s.kth(x - 1)); break;
      case 3: ans.push_back(s.count_le(x)); break;
      case 4: ans.push_back(s.prev(x)); break;
      default: ans.push_back(s.next(x)); break;
    }
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
