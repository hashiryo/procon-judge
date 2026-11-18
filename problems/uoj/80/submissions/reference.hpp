#pragma once
// 期待出力を作る参照実装。ライブラリを使わずに書いた、よく知られた Hungarian 法 (O(N^3)、2026-11-18)。N = max(n_l, n_r) の
// 正方行列で、組める組は -重み、組めない組は 0 にして費用の最小の完全な割り当てを求め、重みが正の組だけを残す (組まない人は
// 0 の組に当たる)。小さい入力では brute.hpp (組を全部試す) と、gen.py の seed 1000 から 2999 で突き合わせてある。
#include <algorithm>
#include <climits>
#include <vector>

struct Solver {
  int nl, nr;
  const vector<array<i64, 3>> &e;
  i64 total = 0;
  vector<int> mt;
  Solver(int nl, int nr, const vector<array<i64, 3>> &e) : nl(nl), nr(nr), e(e) {}
  void run() {
    const int N = max(nl, nr);
    vector<i64> c((size_t)(N + 1) * (N + 1), 0);
    for (auto &[v, u, w] : e) c[(size_t)(v + 1) * (N + 1) + (u + 1)] = -w;
    vector<i64> U(N + 1, 0), V(N + 1, 0);
    vector<int> p(N + 1, 0), way(N + 1, 0);
    for (int i = 1; i <= N; ++i) {
      p[0] = i;
      int j0 = 0;
      vector<i64> minv(N + 1, LLONG_MAX);
      vector<char> used(N + 1, 0);
      do {
        used[j0] = 1;
        const int i0 = p[j0];
        i64 delta = LLONG_MAX;
        int j1 = 0;
        for (int j = 1; j <= N; ++j)
          if (!used[j]) {
            const i64 cur = c[(size_t)i0 * (N + 1) + j] - U[i0] - V[j];
            if (cur < minv[j]) minv[j] = cur, way[j] = j0;
            if (minv[j] < delta) delta = minv[j], j1 = j;
          }
        for (int j = 0; j <= N; ++j)
          if (used[j]) U[p[j]] += delta, V[j] -= delta;
          else minv[j] -= delta;
        j0 = j1;
      } while (p[j0] != 0);
      do {
        const int j1 = way[j0];
        p[j0] = p[j1], j0 = j1;
      } while (j0);
    }
    mt.assign(nl, -1), total = 0;
    for (int j = 1; j <= N; ++j) {
      const int i = p[j];
      if (i >= 1 && i <= nl && j <= nr && c[(size_t)i * (N + 1) + j] < 0) mt[i - 1] = j - 1, total -= c[(size_t)i * (N + 1) + j];
    }
  }
  i64 sum() const { return total; }
  const vector<int> &mate() const { return mt; }
};
