#pragma once
// 今の Library の z_algorithm で、P と T を区切り文字で繋いだ文字列の Z 配列を作る (aoj-ALDS1_14_B の lib-z-algorithm と同じ)。
#include "pj.hpp"
#include "mylib/string/z_algorithm.hpp"

struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() {
    const int n = (int)p.size(), m = (int)t.size();
    string s = p + '\x01' + t;
    vector<int> z = z_algorithm(s);
    pos.clear();
    for (int i = n + 1; i <= m + n; ++i)
      if (z[i] == n) pos.push_back(i - n - 1);
  }
  const vector<int> &answer() const { return pos; }
};
