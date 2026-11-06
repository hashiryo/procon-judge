#pragma once
// 参照実装。接尾辞配列と LCP 配列の sparse table (_shared/string/_ref_lcp.hpp) で、s[a, n) と s[b, n) の LCP が len 以上かを見る。
// 乱択を使わないので、ハッシュの衝突で答えが変わらない。
#include "_shared/string/_ref_lcp.hpp"

struct Solver {
  string_ref::SuffixLCP t;
  explicit Solver(const string &s) : t(s) {}
  bool equal(int a, int b, int len) { return len == 0 || a == b || t.lcp(a, b) >= len; }
};
