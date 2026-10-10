// https://judge.yosupo.jp/problem/predecessor_problem
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<u64> &bits);  // 0 以上 n 未満の整数の集合を作る。計測区間の中。
//     void insert(int k);                       // k を入れる。既にあれば何もしない
//     void erase(int k);                        // k を除く。無ければ何もしない
//     bool contains(int k);
//     int next(int k);                          // k 以上で最小の要素。無ければ -1
//     int prev(int k);                          // k 以下で最大の要素。無ければ -1
//   };
//
// 初期集合は bits の i / 64 番目の下から i % 64 番目の bit が 1 なら i を含む。n 以上の位置の bit は 0。入力の文字列を
// bit に直すのは入力の解析なので、計測区間の外で行う。初期集合の構築とクエリの両方を計測区間に入れる。クエリは
// 1 つずつ渡し、その場で答えさせる。先のクエリを読んでまとめて処理する実装はここでは比べない。n は 10^7 以下。
// 設計は algo-notes の notes/associative-containers.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/std_set.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  string t = read_token();
  if ((int)t.size() != n) must_scan(0, 1);
  vector<u64> bits((n + 63) / 64);
  for (int i = 0; i < n; ++i) bits[i >> 6] |= u64(t[i] == '1') << (i & 63);
  vector<array<int, 2>> qs(q);
  int nout = 0;
  for (auto &e : qs) {
    must_scan(scanf("%d %d", &e[0], &e[1]), 2);
    nout += e[0] >= 2;
  }

  vector<int> ans;
  ans.reserve(nout);

  auto t0 = chrono::steady_clock::now();
  Solver s(n, bits);
  for (auto &[c, k] : qs) {
    switch (c) {
      case 0: s.insert(k); break;
      case 1: s.erase(k); break;
      case 2: ans.push_back(s.contains(k)); break;
      case 3: ans.push_back(s.next(k)); break;
      default: ans.push_back(s.prev(k)); break;
    }
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
