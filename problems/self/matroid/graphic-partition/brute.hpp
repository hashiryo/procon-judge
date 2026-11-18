#pragma once
// 愚直解。辺の部分集合を全部試し、閉路が無く色ごとの上限を守るもののうち要素数が最大のものを返す (2^m m)。
// 小さい入力 (gen.py の seed 1000 以上、辺 16 本まで) でだけ使う。
#include <numeric>

struct Solver {
  int n;
  const vector<array<int, 2>> &e;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap)
      : n(n), e(edges), color(color), cap(cap) {}

  void run() {
    const int m = e.size();
    unsigned best = 0;
    int best_size = 0;
    vector<int> par(n), cnt(cap.size());
    for (unsigned s = 0; s < (1u << m); ++s) {
      const int size = __builtin_popcount(s);
      if (size <= best_size) continue;
      iota(par.begin(), par.end(), 0);
      fill(cnt.begin(), cnt.end(), 0);
      auto find = [&](int x) {
        while (par[x] != x) x = par[x];
        return x;
      };
      bool ok = true;
      for (int i = 0; i < m && ok; ++i) {
        if (!(s >> i & 1)) continue;
        const int a = find(e[i][0]), b = find(e[i][1]);
        if (a == b || ++cnt[color[i]] > cap[color[i]]) ok = false;
        else par[a] = b;
      }
      if (ok) best = s, best_size = size;
    }
    for (int i = 0; i < m; ++i)
      if (best >> i & 1) ans.push_back(i);
  }

  vector<int> answer() const { return ans; }
};
