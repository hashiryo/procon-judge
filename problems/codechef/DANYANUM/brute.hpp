#pragma once
// 愚直解 (pj testdata crosscheck 用)。値ごとの個数を持ち、問いでは mask を大きい順に試して、mask を含む要素の数を数える (k <= 6)。
#include "pj.hpp"
struct Solver {
  int k;
  vector<int> a;
  vector<pair<int, int>> ops;
  vector<int> ans;
  Solver(int k_, const vector<int> &a_, const vector<pair<int, int>> &ops_) : k(k_), a(a_), ops(ops_) {}
  void run() {
    const int N = 1 << k;
    vector<int> c(N, 0);
    for (int x : a) ++c[x];
    for (auto [t, v] : ops) {
      if (t == 1) ++c[v];
      else if (t == 2) --c[v];
      else {
        for (int m = N - 1; m >= 0; --m) {
          int s = 0;
          for (int y = 0; y < N; ++y)
            if ((y & m) == m) s += c[y];
          if (s >= v) {
            ans.push_back(m);
            break;
          }
        }
      }
    }
  }
  vector<int> answer() const { return ans; }
};
