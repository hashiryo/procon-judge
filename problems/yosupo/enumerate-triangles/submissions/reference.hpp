#pragma once
// 三角形の列挙。各辺を (次数, 番号) の小さい方から大きい方へ向け、
// 辺ごとに両端の行き先の共通部分を、片側を印して数える。O(M sqrt(M))。
// 三角形 a<b<c (向きの順) は辺 {a,b} の共通行き先 c としてちょうど 1 回見つかる。
#include <algorithm>
#include <utility>
#include <vector>

struct Solver {
  static constexpr long long mod = 998244353;

  int n;
  std::vector<int> x, u, v;
  std::vector<std::vector<int>> g;  // 向き付けた先だけを持つ
  long long ans = 0;

  Solver(int n_, std::vector<int> x_, std::vector<int> u_, std::vector<int> v_)
      : n(n_), x(std::move(x_)), u(std::move(u_)), v(std::move(v_)) {
    std::vector<int> deg(n, 0);
    for (int i = 0; i < (int)u.size(); ++i) {
      ++deg[u[i]];
      ++deg[v[i]];
    }
    g.assign(n, {});
    for (int i = 0; i < (int)u.size(); ++i) {
      int a = u[i], b = v[i];
      if (std::make_pair(deg[a], a) > std::make_pair(deg[b], b)) std::swap(a, b);
      g[a].push_back(b);
    }
  }

  void run() {
    ans = 0;
    std::vector<char> mark(n, 0);
    for (int i = 0; i < (int)u.size(); ++i) {
      const int a = u[i], b = v[i];
      // 行き先の少ない方を印して、多い方を走査する。
      if (g[a].size() < g[b].size()) {
        for (int c : g[a]) mark[c] = 1;
        for (int c : g[b])
          if (mark[c]) add(a, b, c);
        for (int c : g[a]) mark[c] = 0;
      } else {
        for (int c : g[b]) mark[c] = 1;
        for (int c : g[a])
          if (mark[c]) add(a, b, c);
        for (int c : g[b]) mark[c] = 0;
      }
    }
  }

  void add(int a, int b, int c) {
    ans = (ans + (long long)x[a] * x[b] % mod * x[c]) % mod;
  }

  long long answer() const { return ans; }
};
