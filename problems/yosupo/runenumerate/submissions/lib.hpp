#pragma once
// 今の Library (mylib) の run_enumerate (mylib/string/z_algorithm.hpp)。Main–Lorentz の分割統治で、区間を半分に分けて、真ん中をまたぐ
// run を Z 配列で見つける (O(n log n))。文字は座標圧縮してから扱う。raw のときの lib.cpp と同じ。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "mylib/string/z_algorithm.hpp"

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const auto r = run_enumerate(s);
    ans.resize(r.size());
    for (size_t i = 0; i < r.size(); ++i) ans[i] = {get<0>(r[i]), get<1>(r[i]), get<2>(r[i])};
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
