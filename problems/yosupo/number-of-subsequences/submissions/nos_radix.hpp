#pragma once
// 同じ値の前の位置を、(値, 添字) の組を値の下位から 11 bit ずつの基数ソートで並べて求める版 (値は 2^30 未満なので 3 段)。基数ソートは
// 安定なので、同じ値の添字は前から並び、並びの 1 つ前が前の位置になる。DP は _nos_dp.hpp。
// 異なる部分列の数え上げの実装を比べた記録は algo-notes の notes/number-of-subsequences.md にある。
#include <array>
#include <cstdint>
#include <vector>
#include "pj.hpp"
#include "_nos_dp.hpp"

struct Solver {
  vector<unsigned> a;
  unsigned ans = 0;

  explicit Solver(const vector<unsigned> &a) : a(a) {}

  void run() {
    const int n = int(a.size());
    unsigned mx = 0;
    for (const unsigned x : a) mx = max(mx, x);
    vector<uint64_t> p(n), q(n);  // 上位 32 bit が値、下位 32 bit が添字
    for (int i = 0; i < n; ++i) p[i] = uint64_t(a[i]) << 32 | unsigned(i);
    for (int sh = 0; (mx >> sh) != 0 || sh == 0; sh += 11) {
      array<int, 2049> cnt{};
      for (int i = 0; i < n; ++i) ++cnt[((p[i] >> 32 >> sh) & 2047) + 1];
      for (int d = 0; d < 2048; ++d) cnt[d + 1] += cnt[d];
      for (int i = 0; i < n; ++i) q[cnt[(p[i] >> 32 >> sh) & 2047]++] = p[i];
      p.swap(q);
      if (sh + 11 >= 32) break;
    }
    vector<int> prev(n, -1);
    for (int k = 1; k < n; ++k)
      if ((p[k] >> 32) == (p[k - 1] >> 32)) prev[unsigned(p[k])] = int(unsigned(p[k - 1]));
    ans = nos_dp::count(prev);
  }

  unsigned answer() const { return ans; }
};
