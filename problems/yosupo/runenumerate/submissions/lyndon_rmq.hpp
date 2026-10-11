#pragma once
// Runs 定理による方法 (核は _lyndon_runs.hpp) で、根を伸ばす LCE と LCS を、LCP 配列と NeoLibrary の RangeMin で 1 回ずつ引く版。
// 前へ伸ばす LCS は、反転した文字列の接尾辞配列と LCP 配列で引くので、接尾辞配列を 2 回作る。最悪でも O(n) で、文字を直接比べる
// lyndon_direct が長い比べを重ねる入力でも遅くならない。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "neo/data_structure/RangeMin.hpp"
#include "_lyndon_runs.hpp"

namespace lyndon_rmq {
// 文字列の任意の 2 つの接尾辞の LCP を O(1) で引く。
struct Lce {
  vector<int> isa;
  RangeMin rm;
  explicit Lce(const string &s) {
    const int n = int(s.size());
    const vector<int> sa = suffix_array(s), lcp = lcp_array(s, sa);
    isa.resize(n);
    for (int i = 0; i < n; ++i) isa[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
  }
  // 位置 i と j (i != j、どちらも n 未満) の接尾辞の LCP。
  int operator()(int i, int j) const {
    const auto [a, b] = minmax(isa[i], isa[j]);
    return int(rm.min(a, b));
  }
};
}  // namespace lyndon_rmq

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    if (n < 2) {
      ans.clear();
      return;
    }
    const lyndon_rmq::Lce fwd(s), bwd(string(s.rbegin(), s.rend()));
    ans = lyndon_runs::collect(fwd.isa, [&](int i, int j, int p) {
      const int e = j < n ? fwd(i, j) : 0, b = i > 0 ? bwd(n - i, n - j) : 0;  // 反転した文字列で、n - i と n - j から始まる接尾辞
      return e + b >= p ? pair<int, int>(i - b, j + e) : pair<int, int>(-1, 0);
    });
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
