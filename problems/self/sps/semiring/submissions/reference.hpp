#pragma once
#include "pj.hpp"

// 参照実装。部分集合を 1 つずつ回す 3^n の DP (Library にも NeoLibrary にも依存しない)。
struct Solver {
  int t;
  vector<i64> f, g, h;
  Solver(int t_, int, const vector<i64> &f_, const vector<i64> &g_) : t(t_), f(f_), g(g_) {}
  void run() {
    const int N = (int)f.size();
    const bool mn = t == 0 || t == 2;
    const i64 none = mn ? -1 : t == 4 ? 0 : -1000000000000000000LL;
    // a が b より良いか (min なら小さい方、max なら大きい方)。
    auto better = [&](i64 a, i64 b) { return mn ? a < b : a > b; };
    h.assign(N, 0);
    if (t == 0 || t == 1 || t == 4) {
      for (int s = 0; s < N; ++s) {
        bool found = false;
        i64 best = none;
        for (int a = s;; a = (a - 1) & s) {
          const i64 x = f[a], y = g[s ^ a];
          i64 v;
          bool ok = true;
          if (t == 4) v = min(x, y);
          else ok = x != none && y != none, v = x + y;
          if (ok && (!found || better(v, best))) best = v, found = true;
          if (!a) break;
        }
        h[s] = found ? best : none;
      }
    } else {
      h[0] = 0;
      for (int s = 1; s < N; ++s) {
        const int lo = s & -s, rest = s ^ lo;
        bool found = false;
        i64 best = none;
        for (int a = rest;; a = (a - 1) & rest) {
          const i64 x = f[lo | a], y = h[rest ^ a];
          if (x != none && y != none && (!found || better(x + y, best))) best = x + y, found = true;
          if (!a) break;
        }
        h[s] = found ? best : none;
      }
    }
  }
  vector<i64> answer() const { return h; }
};
