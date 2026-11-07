#pragma once
// 愚直解。すべての文字列のすべての部分文字列を std::set に入れる。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
#include <set>

struct Solver {
  const vector<string> &ss;
  u64 ans = 0;
  explicit Solver(const vector<string> &ss) : ss(ss) {}
  void run() {
    set<string> st;
    for (const auto &s : ss)
      for (size_t i = 0; i < s.size(); ++i)
        for (size_t j = i + 1; j <= s.size(); ++j) st.insert(s.substr(i, j - i));
    ans = st.size();
  }
  u64 answer() const { return ans; }
};
