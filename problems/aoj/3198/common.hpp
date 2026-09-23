#pragma once
// ライブラリを使う提出が共有するもの。比べたいのはマッチングの解き方なので、
// 解法はここに 1 つ置いて、提出はグラフと関数の組を選ぶだけにする。
#include <algorithm>
#include <utility>
#include "pj.hpp"

template <class Policy> struct MatchSolver {
  using G = typename Policy::G;

  int n;
  G g;
  vector<int> partner;

  MatchSolver(int n, const vector<array<int, 2>> &edges)
      : n(n), g(Policy::make(n)), partner(n + n, -1) {
    for (auto &e : edges) g.emplace_back(e[0], e[1] + n);
  }

  bool toggle(int x, int y) {
    y += n;
    auto it = std::find(g.begin(), g.end(), std::make_pair(x, y));
    if (it != g.end()) {
      g.erase(it);
      // 抜いた辺が使われていたら、その組だけ外して残りは引き継ぐ。
      if (partner[x] == y) partner[x] = partner[y] = -1;
    } else g.emplace_back(x, y);
    auto [match, p] = Policy::matching(g, partner);
    partner = p;
    return (int)match.size() == n;
  }
};
