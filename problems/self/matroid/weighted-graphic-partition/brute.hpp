#pragma once
// 愚直解。辺の部分集合を全部試し、閉路が無く色ごとの上限を守るものについて、要素数ごとに重みの和の最大を求める (2^m m)。
// 小さい入力 (gen.py の seed 1000 以上、辺 14 本まで) でだけ使う。
#include <numeric>

struct Solver {
  int n;
  const vector<array<int, 2>> &e;
  const vector<int> &color, &cap;
  const vector<i64> &w;
  vector<i64> best;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap, const vector<i64> &w)
      : n(n), e(edges), color(color), cap(cap), w(w) {}

  void run() {
    const int m = e.size();
    vector<i64> bw(m + 1, -1);
    vector<unsigned> bs(m + 1);
    vector<int> par(n), cnt(cap.size());
    for (unsigned s = 0; s < (1u << m); ++s) {
      iota(par.begin(), par.end(), 0);
      fill(cnt.begin(), cnt.end(), 0);
      auto find = [&](int x) {
        while (par[x] != x) x = par[x];
        return x;
      };
      bool ok = true;
      i64 sum = 0;
      for (int i = 0; i < m && ok; ++i) {
        if (!(s >> i & 1)) continue;
        const int a = find(e[i][0]), b = find(e[i][1]);
        if (a == b || ++cnt[color[i]] > cap[color[i]]) ok = false;
        else par[a] = b, sum += w[i];
      }
      const int k = __builtin_popcount(s);
      if (ok && sum > bw[k]) bw[k] = sum, bs[k] = s;
    }
    int r = 0;
    while (r + 1 <= m && bw[r + 1] >= 0) ++r;
    best.assign(bw.begin(), bw.begin() + r + 1);
    for (int i = 0; i < m; ++i)
      if (bs[r] >> i & 1) ans.push_back(i);
  }

  vector<i64> answer() const { return best; }
  vector<int> chosen() const { return ans; }
};
