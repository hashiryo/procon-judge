#pragma once
// lyndon_direct で、文字を直接比べる回数に全体で 32n の予算を持たせ、使い切ったら LCE と LCS を LCP 配列と NeoLibrary の RangeMin で
// 引く形に切り替える版。直接比べる形は、a b a^2 b a^3 b ... のような入力で O(n^1.5) 回比べるが、ふだんの入力では n の 4 倍から 24
// 倍で済む (Fibonacci で 24 倍)。予算を使い切るまでは lyndon_direct と同じで、切り替えたあとは根ごとに O(1) なので、全体は最悪でも
// O(n)。LCE の表は切り替えるときに初めて作り、後ろへ伸ばす側は Lyndon 配列に使った接尾辞配列を使い回す。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "neo/data_structure/RangeMin.hpp"
#include "_lyndon_runs.hpp"

namespace lyndon_hybrid {
// 文字列の任意の 2 つの接尾辞の LCP を O(1) で引く。
struct Lce {
  vector<int> isa;
  RangeMin rm;
  Lce(const string &s, const vector<int> &sa) : isa(s.size()) {
    const int n = int(s.size());
    const vector<int> lcp = lcp_array(s, sa);
    for (int i = 0; i < n; ++i) isa[sa[i]] = i;
    rm = RangeMin(vector<unsigned>(lcp.begin(), lcp.end()));
  }
  int operator()(int i, int j) const {  // i != j、どちらも n 未満
    const auto [a, b] = minmax(isa[i], isa[j]);
    return int(rm.min(a, b));
  }
};
}  // namespace lyndon_hybrid

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
    long long budget = 32LL * n;
    unique_ptr<lyndon_hybrid::Lce> fwd, bwd;
    ans = lyndon_runs::collect(isa, [&](int i, int j, int p) {
      if (budget >= 0) {
        int e = 0, b = 0;
        while (j + e < n && t[i + e] == t[j + e]) ++e;
        budget -= e + 1;
        for (const int need = p - e; b < need; ++b)
          if (b == i || t[i - 1 - b] != t[j - 1 - b]) return budget -= b + 1, pair<int, int>(-1, 0);
        while (b < i && t[i - 1 - b] == t[j - 1 - b]) ++b;
        budget -= b + 1;
        return pair<int, int>(i - b, j + e);
      }
      if (!fwd) {
        const string r(s.rbegin(), s.rend());
        fwd = make_unique<lyndon_hybrid::Lce>(s, sa), bwd = make_unique<lyndon_hybrid::Lce>(r, suffix_array(r));
      }
      const int e = j < n ? (*fwd)(i, j) : 0, b = i > 0 ? (*bwd)(n - i, n - j) : 0;  // 反転した文字列で、n - i と n - j から始まる接尾辞
      return e + b >= p ? pair<int, int>(i - b, j + e) : pair<int, int>(-1, 0);
    });
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
