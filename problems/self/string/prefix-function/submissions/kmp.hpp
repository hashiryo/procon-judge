#pragma once
// KMP の失敗関数の素直な形 (参照実装と同じ手順)。文字は unsigned char の配列で読み、たどる位置 k をループの外に持つ。
#include "pj.hpp"

struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void run() {
    const int n = (int)s.size();
    pi.resize(n);
    const unsigned char *t = (const unsigned char *)s.data();
    int *p = pi.data();
    p[0] = 0;
    for (int i = 1, k = 0; i < n; ++i) {
      while (k > 0 && t[i] != t[k]) k = p[k - 1];
      k += t[i] == t[k];
      p[i] = k;
    }
  }
  const vector<int> &answer() const { return pi; }
};
