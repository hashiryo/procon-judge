#pragma once
// Runs 定理による方法 (核は _lyndon_runs.hpp) で、根を伸ばす LCE と LCS を文字を直接比べて求める版。NeoLibrary の suffix_array で
// 接尾辞配列を 1 回だけ作り、その順位から 2 つの順序の Lyndon 配列を求める。run にならない根は、伸ばした長さの和が周期 p に届く前に
// 止まり、run の根は同じ run の中の 2 つ目からは飛ばすので、比べる文字の数は run の長さの和と、根ごとの p 以下に収まる。
// 一様な文字列では、たいてい 1 文字目か 2 文字目で止まる。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "_lyndon_runs.hpp"

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    const vector<int> sa = suffix_array(s);
    vector<int> isa(n);
    for (int i = 0; i < n; ++i) isa[sa[i]] = i;
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    ans = lyndon_runs::collect(isa, [&](int i, int j, int p) {
      int e = 0, b = 0;
      while (j + e < n && t[i + e] == t[j + e]) ++e;  // run にならないなら e < p で止まる
      for (const int need = p - e; b < need; ++b)
        if (b == i || t[i - 1 - b] != t[j - 1 - b]) return pair<int, int>(-1, 0);
      while (b < i && t[i - 1 - b] == t[j - 1 - b]) ++b;
      return pair<int, int>(i - b, j + e);
    });
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
