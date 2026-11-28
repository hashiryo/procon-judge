#pragma once
// 最小全域木。Kruskal で、重みの小さい順に辺を見て、両端が別の成分なら木に採る。
#include <algorithm>
#include <numeric>
#include <vector>

struct Solver {
  int n;
  std::vector<int> a, b, ord;
  std::vector<long long> c;
  long long w = 0;
  std::vector<int> chosen;

  Solver(int n_, std::vector<int> a_, std::vector<int> b_, std::vector<long long> c_)
      : n(n_), a(std::move(a_)), b(std::move(b_)), c(std::move(c_)) {}

  int find(std::vector<int> &p, int x) {
    while (p[x] != x) x = p[x] = p[p[x]];
    return x;
  }

  void run() {
    const int m = (int)a.size();
    ord.resize(m);
    std::iota(ord.begin(), ord.end(), 0);
    std::sort(ord.begin(), ord.end(), [&](int x, int y) { return c[x] < c[y]; });
    std::vector<int> p(n);
    std::iota(p.begin(), p.end(), 0);
    chosen.clear();
    w = 0;
    for (int i = 0; i < m && (int)chosen.size() < n - 1; ++i) {
      const int e = ord[i];
      int x = find(p, a[e]), y = find(p, b[e]);
      if (x == y) continue;
      p[x] = y;
      chosen.push_back(e);
      w += c[e];
    }
  }

  long long weight() const { return w; }
  const std::vector<int> &edges() const { return chosen; }
};
