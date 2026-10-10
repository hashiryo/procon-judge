#pragma once
// libsais の提出 (libsais.hpp) から、先読みだけを抜いた版。_shared/libsais/libsais.c を読む間だけ __builtin_prefetch を何もしない形に
// 置き換える (yosupo-suffixarray の libsais_nopf と同じ)。ほかは libsais.hpp と同じ。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <cstdint>
#include <string>
#include <vector>
#include "pj.hpp"
#define __builtin_prefetch(address, ...) ((void)(address))
#include "_shared/libsais/libsais.c"
#undef __builtin_prefetch

struct Solver {
  string s;
  long long ans = 0;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    const uint8_t *t = reinterpret_cast<const uint8_t *>(s.data());
    vector<int> sa(n), plcp(n);
    libsais(t, sa.data(), n, 0, nullptr);
    libsais_plcp(t, sa.data(), plcp.data(), n);
    libsais_lcp(plcp.data(), sa.data(), sa.data(), n);
    ans = (long long)n * (n + 1) / 2;
    for (const int x : sa) ans -= x;
  }

  long long answer() const { return ans; }
};
