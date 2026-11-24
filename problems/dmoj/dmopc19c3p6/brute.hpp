#pragma once
// 愚直解。辺ごとに 0 から 3 を全部試し (4^M)、1、2、3 の辺がそれぞれ全域木になるものを探す。小さい入力 (gen.py の seed 1000
// 以上、M は 9 まで) でだけ使う。
#include <numeric>

struct Solver {
  int n;
  const vector<array<int, 2>> &e;
  bool found = false;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges) : n(n), e(edges) {}

  bool tree(const vector<int> &c, int t) const {
    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    auto find = [&](int x) {
      while (p[x] != x) x = p[x];
      return x;
    };
    int cnt = 0;
    for (size_t i = 0; i < e.size(); ++i) {
      if (c[i] != t) continue;
      const int a = find(e[i][0]), b = find(e[i][1]);
      if (a == b) return false;
      p[a] = b, ++cnt;
    }
    return cnt == n - 1;
  }

  void run() {
    const int m = e.size();
    vector<int> c(m, 0);
    for (long long s = 0; s < (1LL << (2 * m)) && !found; ++s) {
      for (int i = 0; i < m; ++i) c[i] = s >> (2 * i) & 3;
      if (tree(c, 1) && tree(c, 2) && tree(c, 3)) {
        found = true;
        ans.resize(m);
        for (int i = 0; i < m; ++i) ans[i] = c[i] - 1;
      }
    }
  }

  bool ok() const { return found; }
  const vector<int> &group() const { return ans; }
};
