#pragma once
// KMP。パターンの失敗関数を作ってから、テキストを 1 回走査する。答えの列は先に最大の長さを reserve しておく (触るまでページは
// 割り当てられない)。
#include "pj.hpp"

struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() {
    const int n = (int)t.size(), m = (int)p.size();
    const unsigned char *T = (const unsigned char *)t.data(), *P = (const unsigned char *)p.data();
    vector<int> pi(m);
    pi[0] = 0;
    for (int i = 1, k = 0; i < m; ++i) {
      while (k > 0 && P[i] != P[k]) k = pi[k - 1];
      k += P[i] == P[k];
      pi[i] = k;
    }
    pos.clear();
    pos.reserve(n - m + 1);
    for (int i = 0, k = 0; i < n; ++i) {
      while (k > 0 && T[i] != P[k]) k = pi[k - 1];
      k += T[i] == P[k];
      if (k == m) pos.push_back(i - m + 1), k = pi[m - 1];
    }
  }
  const vector<int> &answer() const { return pos; }
};
