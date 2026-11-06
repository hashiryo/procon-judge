#pragma once
// 今の Library の z_algorithm と z_to_kmp。Z 配列から失敗関数 (長さ n + 1、kmp[i] は長さ i の接頭辞の border) を作り、1 つずらして返す。
#include "pj.hpp"
#include "mylib/string/z_algorithm.hpp"

struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void run() {
    const vector<int> kmp = z_to_kmp(z_algorithm(s));
    pi.assign(kmp.begin() + 1, kmp.end());
  }
  const vector<int> &answer() const { return pi; }
};
