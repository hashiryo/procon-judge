#pragma once
// 愚直解 (pj testdata crosscheck 用)。頂点が 12 個ほどまでの入力で、頂点 1 を含み頂点 N を含まない S を全部試し、S から
// 出る辺の本数が最小の S をすべて集めて、問いごとに同じ側になる S と違う側になる S があるかを調べる。
#include <cstdlib>
#include <vector>

struct Solver {
  int n;
  const vector<array<int, 2>> &roads, &qs;
  Solver(int n, const vector<array<int, 2>> &roads, const vector<array<int, 2>> &qs) : n(n), roads(roads), qs(qs) {}
  vector<int> run() {
    if (n > 14) abort();
    vector<int> best;
    int mn = 1 << 30;
    for (int S = 0; S < 1 << n; ++S) {
      if (!(S & 1) || (S >> (n - 1) & 1)) continue;
      int c = 0;
      for (auto &[a, b] : roads) c += (S >> a & 1) && !(S >> b & 1);
      if (c < mn) mn = c, best.clear();
      if (c == mn) best.push_back(S);
    }
    vector<int> ans;
    for (auto &[c, d] : qs) {
      int x = 0;
      for (int S : best) x |= ((S >> c & 1) == (S >> d & 1)) ? 1 : 2;
      ans.push_back(x);
    }
    return ans;
  }
};
