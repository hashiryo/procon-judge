#pragma once
// NeoLibrary の runs (neo/string/runs.hpp) を呼ぶ提出。中身は、この問題で書き比べた lyndon_fs_hp2 を写したもの。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/runs.hpp"

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() { ans = runs(s); }

  const vector<array<int, 3>> &answer() const { return ans; }
};
