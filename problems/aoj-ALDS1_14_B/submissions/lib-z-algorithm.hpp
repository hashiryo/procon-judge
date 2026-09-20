#pragma once
#include "pj.hpp"
#include "mylib/string/z_algorithm.hpp"

// P と T を区切り文字で繋いで Z 配列を作る。P の長さに一致した位置が出現位置。
// 前処理も探索も O(|T| + |P|) で、余分な構造を持たない。
struct Solver {
  string t, p;
  vector<int> pos;

  Solver(const string &t, const string &p) : t(t), p(p) {}

  void run() {
    const int n = (int)p.size(), m = (int)t.size();
    string s = p + '\x01' + t;  // 区切りは入力に出てこない文字にする
    vector<int> z = z_algorithm(s);
    pos.clear();
    for (int i = n + 1; i <= m + n; ++i)
      if (z[i] == n) pos.push_back(i - n - 1);
  }

  const vector<int> &answer() const { return pos; }
};
