#pragma once
// 愚直解 (pj testdata crosscheck 用)。装備が 10 個ほどまでの入力で、k = 1, 2, ... の順に、値 1 から k に別々の装備を
// 割り当てられるかを全部試して確かめる。
#include <cstdlib>
#include <vector>

struct Solver {
  int n;
  const vector<array<int, 2>> &items;
  Solver(int n, const vector<array<int, 2>> &items) : n(n), items(items) {}
  int run() {
    if (n > 12) abort();
    vector<char> used(n, 0);
    // 値 v から k までに、残りの装備を割り当てられるか。
    auto rec = [&](auto &&self, int v, int k) -> bool {
      if (v > k) return true;
      for (int i = 0; i < n; ++i)
        if (!used[i] && (items[i][0] == v || items[i][1] == v)) {
          used[i] = 1;
          const bool ok = self(self, v + 1, k);
          used[i] = 0;
          if (ok) return true;
        }
      return false;
    };
    int k = 0;
    while (k < 10000 && rec(rec, 1, k + 1)) ++k;
    return k;
  }
};
