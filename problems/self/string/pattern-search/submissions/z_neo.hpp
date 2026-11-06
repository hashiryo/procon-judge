#pragma once
// NeoLibrary の z_algorithm (neo/string/z_algorithm.hpp) で、P と T を区切り文字で繋いだ文字列の Z 配列を作る。
#include "pj.hpp"
#include "neo/string/z_algorithm.hpp"

struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() {
    const int n = (int)t.size(), m = (int)p.size();
    string s;
    s.reserve(n + m + 1);
    s += p, s += '\x01', s += t;
    const vector<int> z = z_algorithm(s);
    pos.clear();
    pos.reserve(n - m + 1);
    for (int i = 0; i + m <= n; ++i)
      if (z[m + 1 + i] == m) pos.push_back(i);
  }
  const vector<int> &answer() const { return pos; }
};
