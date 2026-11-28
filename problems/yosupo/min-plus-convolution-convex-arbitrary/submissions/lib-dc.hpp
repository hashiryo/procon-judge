#pragma once
#include <algorithm>
#include "pj.hpp"

// 分割統治で各行の最小を求める。今の Library の monotone_minima と同じ形ですが、比較の述語で 2 回評価する
// 代わりに、走査の中で最良の値を覚えて候補ごとに 1 回だけ評価します。凸な a と任意の b の min-plus
// convolution は行の最小の列が行について単調なので、この形で O(N+M) log の評価回数になります。
struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size(), h = n + m - 1;
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    vector<int> r(h);
    auto rec = [&](auto &rec, int h1, int h2, int w1, int w2) -> void {
      if (h1 == h2) return;
      const int hm = (h1 + h2) / 2;
      const int lo = std::max(0, hm - (n - 1)), hi = std::min(m - 1, hm);
      const int wl = std::max(w1, lo), wr = std::min(w2, hi + 1);
      int best = wl;
      i64 bestv = INF;
      for (int j = wl; j < wr; ++j) {
        const i64 v = a[hm - j] + b[j];
        if (v < bestv) bestv = v, best = j;
      }
      r[hm] = best;
      rec(rec, h1, hm, w1, best + 1);
      rec(rec, hm + 1, h2, best, w2);
    };
    rec(rec, 0, h, 0, m);
    c.resize(h);
    for (int k = 0; k < h; ++k) c[k] = a[k - r[k]] + b[r[k]];
  }

  const vector<i64> &answer() const { return c; }
};
