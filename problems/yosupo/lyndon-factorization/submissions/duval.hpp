#pragma once
// Duval の方法 (1983) による Lyndon 分解。i から始まる部分を読み、j を 1 つずつ進めながら、周期の頭 k と比べる。s[k] = s[j] なら k も
// 進め、s[k] < s[j] なら k を i に戻す (s[i, j + 1) は Lyndon 語)。s[k] > s[j] で止まったら、周期 j - k の Lyndon 語 s[i, i + j - k)
// を、i から何回か切り出す。比べる回数は全体で 4n 以下。
// Lyndon 分解の実装を比べた記録は algo-notes の notes/lyndon.md にある。
#include <string>
#include <vector>
#include "pj.hpp"

struct Solver {
  string s;
  vector<int> cut;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    const unsigned char *t = reinterpret_cast<const unsigned char *>(s.data());
    cut.clear();
    cut.push_back(0);
    for (int i = 0; i < n;) {
      int j = i + 1, k = i;
      for (; j < n && t[k] <= t[j]; ++j) k = t[k] < t[j] ? i : k + 1;
      for (const int p = j - k; i <= k; i += p) cut.push_back(i + p);
    }
  }

  const vector<int> &answer() const { return cut; }
};
