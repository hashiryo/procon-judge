#pragma once
#include "pj.hpp"
#include "mylib/number_theory/tables.hpp"

// f[a] = floor(N / a^3) (a^3 で割り切れる数) の表に倍数メビウス変換をかけると、
// 最大の立方因子がちょうど a^3 の数になる。a >= 2 の和が答え。テストケースごとに
// 変換をやり直す。
struct Solver {
  vector<i64> ns, ans;

  explicit Solver(const vector<i64> &ns) : ns(ns) {}

  void run() {
    vector<i64> f(1'000'010);
    ans.clear();
    for (i64 n : ns) {
      i64 a = 2, cnt = 0;
      for (; a * a * a <= n; a++) f[a] = n / (a * a * a);
      vector<i64> g(f.begin(), f.begin() + a);
      multiple_mobius(g);
      for (; --a >= 2;) cnt += g[a];
      ans.push_back(cnt);
    }
  }

  const vector<i64> &answer() const { return ans; }
};
