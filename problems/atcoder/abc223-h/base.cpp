// https://atcoder.jp/contests/abc223/tasks/abc223_h
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(const vector<u64> &a);  // 前処理。計測区間の中
//     bool query(int l, int r, u64 x);         // a[l..r) (0 始まり、半開区間) の元の xor で x を作れるか。x ≥ 1
//   };
//
// 解き方の型は、接頭辞ごとに「bit ごとに最も右の元を残す」xor 基底を持ち、問い合わせでは位置が l 以上の元だけで x を消す
// もの。区間版の xor 基底をライブラリに置くときの持ち方 (接頭辞ごとの写しを全部持つか、間引くか、変更の履歴を持つか) を
// 比べる。前処理とクエリの両方を計測区間に入れ、クエリは 1 つずつ渡してその場で答えさせる (先読みはさせない)。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, q;
  must_scan(scanf("%d %d", &n, &q), 2);
  vector<u64> a(n);
  for (auto &x : a) must_scan(scanf("%llu", &x), 1);
  struct Q {
    int l, r;
    u64 x;
  };
  vector<Q> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%d %d %llu", &e.l, &e.r, &e.x), 3);
    --e.l;
  }
  vector<char> ans(q);

  auto t0 = chrono::steady_clock::now();
  Solver sol(a);
  for (int i = 0; i < q; ++i) ans[i] = sol.query(qs[i].l, qs[i].r, qs[i].x);
  auto t1 = chrono::steady_clock::now();

  string out;
  out.reserve((size_t) q * 4);
  for (char c : ans) out += c ? "Yes\n" : "No\n";
  fputs(out.c_str(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
