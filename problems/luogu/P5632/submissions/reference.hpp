#pragma once
// 期待出力を作る参照実装。_shared/gmincut とは別に書いた、よく知られた Stoer-Wagner の方法 (隣接行列で O(n^3)、2026-11-18)。
// 段ごとに、縮めていない頂点を、選んだ集合への重みの和が最大のものから順に選び、最後の頂点の重みの和で答えを更新して、
// 最後の頂点を 1 つ前の頂点へ縮める (行列の行と列を足し、最後の頂点は今の頂点の列の末尾と入れ替える)。小さい入力では
// brute.hpp (頂点集合を全部試す) と、gen.py の seed 1000 から 2999 で突き合わせてある。
#include <vector>

struct Solver {
  int n;
  const vector<array<i64, 3>> &e;
  Solver(int n, const vector<array<i64, 3>> &e) : n(n), e(e) {}
  i64 run() {
    vector<vector<i64>> w(n, vector<i64>(n, 0));
    for (auto &[u, v, c] : e)
      if (u != v) w[u][v] += c, w[v][u] += c;
    vector<int> vs(n);
    for (int i = 0; i < n; ++i) vs[i] = i;
    i64 best = -1;
    for (int k = n; k > 1; --k) {
      vector<i64> a(k, 0);
      vector<char> used(k, 0);
      int s = -1, t = -1;
      for (int step = 0; step < k; ++step) {
        int sel = -1;
        for (int i = 0; i < k; ++i)
          if (!used[i] && (sel < 0 || a[i] > a[sel])) sel = i;
        used[sel] = 1, s = t, t = sel;
        for (int i = 0; i < k; ++i)
          if (!used[i]) a[i] += w[vs[sel]][vs[i]];
      }
      if (best < 0 || a[t] < best) best = a[t];
      // vs[t] を vs[s] へ縮める。
      for (int i = 0; i < k; ++i) w[vs[s]][vs[i]] += w[vs[t]][vs[i]], w[vs[i]][vs[s]] = w[vs[s]][vs[i]];
      w[vs[s]][vs[s]] = 0;
      vs[t] = vs[k - 1];
    }
    return best < 0 ? 0 : best;
  }
};
