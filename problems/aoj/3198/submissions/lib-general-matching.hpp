#pragma once
#include "common.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/general_matching.hpp"

// 一般グラフの最大マッチング。二部であることを使わず花を潰すので、同じ答えを
// 出すのに余計な仕事がある。
struct GeneralPolicy {
  using G = Graph;
  static G make(int n) { return G(n + n); }
  static auto matching(const G &g, const vector<int> &partner) {
    return general_matching(g, partner);
  }
};

using Solver = MatchSolver<GeneralPolicy>;
