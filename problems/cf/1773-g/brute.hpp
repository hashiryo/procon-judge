#pragma once
// 愚直解 (pj testdata crosscheck 用)。質問の順列を全部試して、定義のとおりにゲームを進める (n <= 7)。
#include <algorithm>
#include <numeric>
#include "pj.hpp"
struct Solver {
  int n, m;
  vector<u32> q;
  double ans = 0;
  Solver(int n_, int m_, const vector<u32> &q_) : n(n_), m(m_), q(q_) {}
  void run() {
    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    const u32 full = (1u << m) - 1;
    long long win = 0, tot = 0;
    do {
      u32 s = full;
      for (int i : p) {
        const u32 c = s & q[i];
        if (c == s || c == 0) continue;
        s = c;  // 不正解の人が抜ける
      }
      win += s & 1;
      ++tot;
    } while (next_permutation(p.begin(), p.end()));
    ans = double(win) / double(tot);
  }
  double answer() const { return ans; }
};
