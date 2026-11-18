#pragma once
// 愚直解 (pj testdata crosscheck 用)。頂点が 14 個ほどまでの入力で、空でない真部分集合を全部試してカットの最小を出す。
#include <cstdlib>
#include <vector>

struct Solver {
  int n;
  const vector<array<i64, 3>> &e;
  Solver(int n, const vector<array<i64, 3>> &e) : n(n), e(e) {}
  i64 run() {
    if (n > 16) abort();
    i64 best = -1;
    for (int S = 1; S < (1 << n) - 1; ++S) {
      i64 c = 0;
      for (auto &[u, v, w] : e)
        if ((S >> u & 1) != (S >> v & 1)) c += w;
      if (best < 0 || c < best) best = c;
    }
    return best < 0 ? 0 : best;
  }
};
