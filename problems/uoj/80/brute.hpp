#pragma once
// 愚直解 (pj testdata crosscheck 用)。男女が 7 人ほどまでの入力で、男ごとに組まないか組める女を選ぶのを全部試し、重みの和の
// 最大と、そのときの組を出す。
#include <cstdlib>
#include <vector>

struct Solver {
  int nl, nr;
  const vector<array<i64, 3>> &e;
  i64 best = -1;
  vector<int> mt, cur;
  Solver(int nl, int nr, const vector<array<i64, 3>> &e) : nl(nl), nr(nr), e(e) {}
  void run() {
    if (nl > 8 || nr > 8) abort();
    vector<vector<i64>> w(nl, vector<i64>(nr, -1));
    for (auto &[v, u, c] : e) w[v][u] = c;
    vector<char> used(nr, 0);
    cur.assign(nl, -1);
    auto rec = [&](auto &&self, int v, i64 s) -> void {
      if (v == nl) {
        if (s > best) best = s, mt = cur;
        return;
      }
      cur[v] = -1, self(self, v + 1, s);
      for (int u = 0; u < nr; ++u)
        if (!used[u] && w[v][u] >= 0) used[u] = 1, cur[v] = u, self(self, v + 1, s + w[v][u]), used[u] = 0, cur[v] = -1;
    };
    rec(rec, 0, 0);
  }
  i64 sum() const { return best; }
  const vector<int> &mate() const { return mt; }
};
