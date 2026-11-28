#pragma once
#include "pj.hpp"

// 想定解。直線 y = f_j X + dp[j] を傾きの減る順に両端キューへ積み、問い s_i を増える順に引く。
// j = 0 は「まだ 1 体も倒していない」状態で、f_0 = x、dp[0] = 0 とする。そうすると
// dp[i] = min_{0 <= j < i} (dp[j] + s_i f_j) の 1 本の形になり、答えは dp[n]。
// 期待出力を作る側なので、Library を include しない。
struct Solver {
  int n;
  i64 x;
  vector<i64> s, f;
  vector<i64> a, b;  // 包絡に残っている直線 y = a X + b
  int head = 0, tail = 0;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f), a(n + 2), b(n + 2) {}

  void run() {
    add_line(x, 0);
    i64 last = 0;
    for (int i = 0; i < n; ++i) {
      last = query(s[i]);
      add_line(f[i], last);
    }
    ans = last;  // dp[n]
  }

  i64 answer() const { return ans; }

private:
  // 傾きが a1 >= a2 >= a3 の 3 本で、l2 が包絡に出ないか。交点 x12 >= x23 と同じ。
  static bool hidden(i64 a1, i64 b1, i64 a2, i64 b2, i64 a3, i64 b3) {
    return (__int128)(b3 - b1) * (a1 - a2) <= (__int128)(b2 - b1) * (a1 - a3);
  }

  void add_line(i64 na, i64 nb) {
    if (tail > head && a[tail - 1] == na) {
      if (b[tail - 1] <= nb) return;
      --tail;
    }
    while (tail - head >= 2 && hidden(a[tail - 2], b[tail - 2], a[tail - 1], b[tail - 1], na, nb)) --tail;
    a[tail] = na, b[tail] = nb, ++tail;
  }

  // 問い X は前の問い以上。包絡の先頭から進めて最小を引く。
  i64 query(i64 X) {
    while (head + 1 < tail && a[head + 1] * X + b[head + 1] <= a[head] * X + b[head]) ++head;
    return a[head] * X + b[head];
  }
};
