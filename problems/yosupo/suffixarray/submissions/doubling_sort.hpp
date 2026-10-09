#pragma once
// prefix doubling の基準。先頭 2^k 文字の順位の組 (rk[i], rk[i + 2^k]) で添字を std::sort で並べ直し、組が変わるところで新しい順位を
// 振る。順位がすべて違ったら止める。1 回が O(n log n) の比較なので、全体は O(n log^2 n)。英小文字の一様な文字列なら 5 回ほど、
// 同じ文字が続く文字列では 19 回ほど並べ直す。
#include <algorithm>
#include <numeric>
#include "pj.hpp"

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    sa.resize(n);
    if (!n) return;
    std::iota(sa.begin(), sa.end(), 0);
    vector<int> rk(n), tmp(n);
    for (int i = 0; i < n; ++i) rk[i] = (unsigned char)s[i];
    for (int k = 1;; k <<= 1) {
      const auto key2 = [&](int i) { return i + k < n ? rk[i + k] : -1; };
      const auto less = [&](int x, int y) { return rk[x] != rk[y] ? rk[x] < rk[y] : key2(x) < key2(y); };
      std::sort(sa.begin(), sa.end(), less);
      tmp[sa[0]] = 0;
      for (int i = 1; i < n; ++i) tmp[sa[i]] = tmp[sa[i - 1]] + (less(sa[i - 1], sa[i]) ? 1 : 0);
      std::swap(rk, tmp);
      if (rk[sa[n - 1]] == n - 1) break;  // 順位がすべて違う
    }
  }

  const vector<int> &answer() const { return sa; }
};
