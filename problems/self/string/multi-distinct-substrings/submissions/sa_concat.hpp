#pragma once
// 文字列を区切り文字 '{' でつなぎ、NeoLibrary の接尾辞配列と LCP 配列 (neo/string/suffix_array.hpp) で数える (参照実装と同じ数え方)。
// 接尾辞配列の順に、位置 i から始まる接尾辞は、次の区切り文字までの長さ b(i) のうち 1 つ前の接尾辞との LCP を超える分を足す。
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  const vector<string> &ss;
  u64 ans = 0;
  explicit Solver(const vector<string> &ss) : ss(ss) {}
  void run() {
    size_t tot = 0;
    for (const auto &s : ss) tot += s.size() + 1;
    string t;
    t.reserve(tot);
    for (const auto &s : ss) t += s, t += '{';
    const int n = (int)t.size();
    const vector<int> sa = suffix_array(t), lcp = lcp_array(t, sa);
    vector<int> bound(n);
    for (int i = n, d = 0; i--;) d = t[i] == '{' ? 0 : d + 1, bound[i] = d;
    u64 a = bound[sa[0]];
    for (int r = 1; r < n; ++r) a += (u64)max(0, bound[sa[r]] - lcp[r - 1]);
    ans = a;
  }
  u64 answer() const { return ans; }
};
