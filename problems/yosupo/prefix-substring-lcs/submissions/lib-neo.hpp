#pragma once
// NeoLibrary の PrefixSubstringLCS (psl_ckpt を写したもの) を呼ぶ提出。
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/PrefixSubstringLCS.hpp"

struct Solver {
  string s, t;
  vector<array<int, 3>> qs;
  vector<int> ans;

  Solver(const string &s, const string &t, const vector<array<int, 3>> &qs) : s(s), t(t), qs(qs) {}

  void run() {
    const PrefixSubstringLCS lcs(s, t);
    ans.resize(qs.size());
    for (size_t k = 0; k < qs.size(); ++k) ans[k] = lcs.query(qs[k][0], qs[k][1], qs[k][2]);
  }

  const vector<int> &answer() const { return ans; }
};
