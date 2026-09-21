#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは最大流のエンジンなので、
// 最小カットへの帰着はここに 1 つ置いて、提出はエンジンを選ぶだけにする。
#include "pj.hpp"
#include "mylib/optimization/MaxFlow.hpp"
#include "mylib/optimization/monge_mincut.hpp"

// 国 i の選び方を x_i in {0: 行くがツアーは無し, 1: 行かない, 2: ツアーも行く} と
// して、満足度の符号を変えた費用を最小化する。条件 (D, E) は x_D = 2 かつ x_E = 0
// を禁じる。3 値の変数どうしの費用が Monge なので monge_mincut に載る。
template <class Engine> struct TourSolver {
  using MF = MaxFlow<Engine>;

  vector<array<i64, 2>> bc;
  vector<array<int, 2>> de;
  i64 ans = 0;

  TourSolver(const vector<array<i64, 2>> &bc, const vector<array<int, 2>> &de) : bc(bc), de(de) {}

  void run() {
    const int n = (int)bc.size();
    vector<vector<bool>> to(n, vector<bool>(n, false));
    for (auto &e : de) to[e[0]][e[1]] = true;
    const i64 INF = 1LL << 62;
    auto theta = [&](int i, int xi) {
      if (xi == 0) return -bc[i][1];
      if (xi == 1) return 0LL;
      return -bc[i][0];
    };
    auto phi = [&](int i, int j, int xi, int xj) {
      if (to[i][j] && xi == 2 && xj == 0) return INF;
      return 0LL;
    };
    auto res = monge_mincut<MF>(n, 3, theta, phi);
    ans = -res.first;
  }

  i64 answer() const { return ans; }
};
