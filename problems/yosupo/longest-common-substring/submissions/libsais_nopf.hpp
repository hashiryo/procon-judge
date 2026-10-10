#pragma once
// libsais の提出 (libsais.hpp) から、先読みだけを抜いた版。_shared/libsais/libsais.c を読む間だけ __builtin_prefetch を何もしない形に
// 置き換える (yosupo-suffixarray の libsais_nopf と同じ)。ほかは libsais.hpp と同じ。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "pj.hpp"
#define __builtin_prefetch(address, ...) ((void)(address))
#include "_shared/libsais/libsais.c"
#undef __builtin_prefetch

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
