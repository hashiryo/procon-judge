#pragma once
#include "common.hpp"
#include "neo/algo/optimize/line/MonotoneCHT.hpp"

// common.hpp の LineSolver<Lines> の包みを通さず、run() の中で MonotoneCHT を直接使う形。
// dp-z の lib-neo-inline と同じで、包みを通す代金を除いたライブラリの速さを測る。
struct Solver {
  i64 a, b, w;
  vector<i64> d;
  i64 ans = 0;

  Solver(i64 a, i64 b, i64 w, const vector<i64> &d) : a(a), b(b), w(w), d(d) {}

  void run() {
    const i64 n = (i64)d.size();
    MonotoneCHT<i64> lines;
    ans = tail(a, b, n, 0);
    lines.add_line(0, 0);
    for (i64 i = 1; i <= n; ++i) {
      const i64 dp = lines.query_monotone(i) + d[i - 1] + b * i * (i - 1) / 2 - a * i + a;
      ans = std::min(ans, dp + tail(a, b, n, i));
      lines.add_line(-b * i, dp + b * i * (i + 1) / 2 + a * i);
    }
    ans += w;
  }

  i64 answer() const { return ans; }
};
