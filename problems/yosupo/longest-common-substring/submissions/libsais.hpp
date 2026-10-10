#pragma once
// libsais 2.10.4 (https://github.com/IlyaGrebnov/libsais、Ilya Grebnov、Apache License 2.0) で、s と t を '$' でつないだ文字列の接尾辞
// 配列 (libsais)、PLCP 配列 (libsais_plcp)、LCP 配列 (libsais_lcp) を作り、隣り合う接尾辞が別の文字列から始まる所の LCP の最大を取る。
// どの段も 64 個先を先読みし、2 個か 4 個ずつ展開してある。隣り合う組の側を見るのに接尾辞配列が要るので、LCP は別の配列に書く。
// problems/_shared/libsais/ の写しには手を入れず、OpenMP は使わない (1 スレッド)。LCP[i] は sa[i - 1] と sa[i] の接尾辞の LCP。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_shared/libsais/libsais.c"

struct Solver {
  string s, t;
  array<int, 4> ans{};

  Solver(const string &s, const string &t) : s(s), t(t) {}

  void run() {
    const int n = int(s.size());
    const string u = s + "$" + t;
    const int N = int(u.size());
    const uint8_t *p = reinterpret_cast<const uint8_t *>(u.data());
    vector<int> sa(N), plcp(N), lcp(N);
    libsais(p, sa.data(), N, 0, nullptr);
    libsais_plcp(p, sa.data(), plcp.data(), N);
    libsais_lcp(plcp.data(), sa.data(), lcp.data(), N);
    int a = 0, c = 0, len = 0;
    for (int i = 1; i < N; ++i) {
      int x = sa[i - 1], y = sa[i];
      if (x > y) swap(x, y);
      if (x < n && n < y && len < lcp[i]) len = lcp[i], a = x, c = y - n - 1;
    }
    ans = {a, a + len, c, c + len};
  }

  array<int, 4> answer() const { return ans; }
};
