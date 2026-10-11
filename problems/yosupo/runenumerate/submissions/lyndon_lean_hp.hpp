#pragma once
// lyndon_ll_w8 で、2 つの Lyndon 配列を連結リストの節から読み、確保する配列を減らした版 (中身は _lyndon_lean.hpp)。
// 作業領域は 2 MB 境界に寄せて huge page を頼む (Linux のみ)。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_lyndon_lean.hpp"

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() { ans = lyndon_lean::runs<true>(s); }

  const vector<array<int, 3>> &answer() const { return ans; }
};
