#pragma once
// NeoLibrary の GeneralMatching (neo/algo/optimize/matching/GeneralMatching.hpp)。中身は self-flow-general-matching の
// hybrid2 と同じ。マッチングを run で求め、辺の番号は answer で取り出す。
#include <optional>
#include "neo/algo/optimize/matching/GeneralMatching.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& e;
 optional<GeneralMatching> gm;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), e(edges) {}
 void run() { gm.emplace(n, e); }
 vector<int> answer() const { return gm->matching(); }
};
