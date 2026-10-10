#pragma once
// libsais 2.10.4 (https://github.com/IlyaGrebnov/libsais、Ilya Grebnov、Apache License 2.0) で、接尾辞配列 (libsais)、PLCP 配列
// (libsais_plcp、Φ 配列を経由する方法)、LCP 配列 (libsais_lcp、接尾辞配列の領域に書く) を作って数える。どの段も 64 個先を先読みし、
// 2 個か 4 個ずつ展開してある。problems/_shared/libsais/ の写しには手を入れず、OpenMP は使わない (1 スレッド)。libsais_lcp の LCP[i] は
// sa[i - 1] と sa[i] の接尾辞の LCP (LCP[0] = 0)。
// LCP 配列の実装を比べた記録は algo-notes の notes/lcp-array.md にある。
#include <cstdint>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_shared/libsais/libsais.c"

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
