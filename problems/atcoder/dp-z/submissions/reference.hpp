#pragma once
#include "pj.hpp"

// 参照実装。_shared/lines を使わずに、傾きが減る順の両端キューの CHT を素直に書く。要らない直線の判定は
// __int128 の積で比べ、問いは先頭から進める。
struct Solver {
  i64 c;
  vector<i64> h;
  i64 ans = 0;

  Solver(i64 c, const vector<i64> &h) : c(c), h(h) {}

  void run() {
    const int n = (int)h.size();
    vector<i64> la, lb;
    size_t p = 0;
    auto add = [&](i64 a, i64 b) {
      while (la.size() - p >= 2) {
        const size_t k = la.size();
        // 直線 k-2, k-1, 新しい直線で、k-1 が下側の包絡に出ないなら除く。
        if ((__int128)(b - lb[k - 2]) * (la[k - 2] - la[k - 1]) <= (__int128)(lb[k - 1] - lb[k - 2]) * (la[k - 2] - a)) {
          la.pop_back(), lb.pop_back();
        } else {
          break;
        }
      }
      la.push_back(a), lb.push_back(b);
    };
    auto get = [&](i64 x) {
      while (p + 1 < la.size() && la[p + 1] * x + lb[p + 1] <= la[p] * x + lb[p]) ++p;
      return la[p] * x + lb[p];
    };
    i64 dp = 0;
    add(-2 * h[0], h[0] * h[0]);
    for (int i = 1; i < n; ++i) {
      dp = get(h[i]) + h[i] * h[i] + c;
      add(-2 * h[i], dp + h[i] * h[i]);
    }
    ans = dp;
  }

  i64 answer() const { return ans; }
};
