#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// 問いの x は s_i しかないので、先に全部渡して座標を詰めた LiChaoTreePoints で引く。
// 木の深さが問いの数で決まり、節に載っている直線を葉から根へ上りながら見る。
struct Solver {
  int n;
  i64 x;
  vector<i64> s, f;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f) {}

  void run() {
    LiChaoTreePoints<i64> lct(s);  // s をコピーして、重複を除いて並べた座標にする
    lct.add_line(x, 0);            // まだ 1 体も倒していない状態
    i64 last = 0;
    for (int i = 0; i < n; ++i) {
      last = lct.query(s[i]);
      lct.add_line(f[i], last);
    }
    ans = last;  // dp[n]
  }

  i64 answer() const { return ans; }
};
